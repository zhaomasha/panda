#include "panda_subgraph.hpp"

Vertex::Vertex(v_type i):id(i),size(0),head(INVALID_BLOCK),tail(INVALID_BLOCK){}

Vertex::Vertex(Vertex_u v){
	id=v.id;
	param=v.param;
}

Edge::Edge(Edge_u e){
	id=e.d_id;
	param=e.param;
	timestamp=e.timestamp;
}
Edge::Edge(){}
Edge_u Edge::to_edge_u(v_type s_id){
	Edge_u edge_u(s_id,id,param,timestamp);
	return edge_u;
}

//新建一个子图文件(子图文件存在，则会被清零)，初始化为默认的大小（操作系统的栈大小要调整，否则会段错误）
//返回0表示成功，返回1表示失败
//1
int Subgraph::init(string name,string dir){
	//文件存在则清零，不存在则创建
	filename=name;
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
	if(!io){
		cout<<name<<" create failed"<<endl;
		return 1;
	}	
	graph_dir=dir;
	sub_key=get_sub_key(name,dir);
	first=last=NULL;//内存的缓冲区块链表为空
	delete_count=0;
	//初始化默认大小
	add_file(atoi(getenv("INITSZ")));
        init_locks();
        return 0;
}
//子图扩张，默认大小可以配置，也可以指定
//1
void Subgraph::add_file(uint32_t size){
	io.seekp(0,fstream::end);
	char tmp[1024*1024*size];
	io.write(tmp,sizeof(tmp));
} 
//格式化子图，创建子图的头，初始化索引结构等等，参数是该子图的block大小
//1
void Subgraph::format(uint32_t block_size){
	head.free_head=INVALID_BLOCK;
	head.vertex_head=INVALID_BLOCK;		
	head.vertex_tail=INVALID_BLOCK;		
	head.vertex_num=0;
	head.free_num=0;
	head.block_num=0;
	head.block_size=block_size;
	//初始化索引
	update_index();
        //创建顶点的索引文件
	vertex_index.init(graph_dir+"/"+sub_key+"_"+getenv("VERTEX_INDEX_FILENAME"));
	vertex_index.format();
}
//读取子图文件，还原一个子图
//1
void Subgraph::recover(string name,string dir){
	graph_dir=dir;
	filename=name;
	sub_key=get_sub_key(name,dir);
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary);
	io.seekg(0);
	io.read((char*)&head,sizeof(SubgraphHeader));//读取子图文件中的子图头，这部分数据要事先读入内存
	first=last=NULL;//内存缓冲区是0
        init_locks();
	delete_count=0;
	vertex_index.recover(graph_dir+"/"+sub_key+"_"+getenv("VERTEX_INDEX_FILENAME"));//还原顶点索引	
}
//初始化锁
//1
void Subgraph::init_locks(){
        require_lock=Getlock();
        Initlock(require_lock,NULL);
        getblock_lock=Getlock();
        Initlock(getblock_lock,NULL);
        cache_lock=Getlock();
        Initlock(cache_lock,NULL);
        vertex_lock=Getlock();
        Initlock(vertex_lock,NULL);

}
//释放锁
//1
void Subgraph::free_locks(){
        Destroylock(require_lock);
        free(require_lock);
        Destroylock(getblock_lock);
        free(getblock_lock);
        Destroylock(cache_lock);
        free(cache_lock);
        Destroylock(vertex_lock);
        free(vertex_lock);
}
//根据子图目录和子图名字得到子图的key
//0
string Subgraph::get_sub_key(string name,string dir){
	int begin=dir.length()+1;
	int len=0;
	const char *c_name=name.c_str();  
	for(int i=begin;c_name[i]!='.';i++) len++;
	string key(c_name+begin,len);
	return key;
}
//析构函数，把子图头和内存中的缓存存到硬盘里
//1
Subgraph::~Subgraph(){
	cout<<"subgraph:"<<filename<<" xigou"<<endl;
        //把子图头存入文件
	io.seekp(0);	
	//io.seekp(get_offset(block->number));
        io.write((char*)&head,sizeof(SubgraphHeader));
  	//遍历子图的缓存里面的块，脏了就写到文件里面
	c_it it;
        it=cache.begin();
	Node* node;
	while(it!=cache.end()){
		node=it->second;
		if(((BlockHeader<char>*)(node->block))->clean==1){
			//脏块，则写入到文件中
			io.seekp(get_offset(((BlockHeader<char>*)(node->block))->number));
			io.write((char*)(node->block),head.block_size);
		}
		//释放块所占的内存
		if(((BlockHeader<char>*)(node->block))->is_hash==1){
			delete ((BlockHeader<char>*)(node->block))->data_hash;//释放块内的hash表内存
		}
		free(node);
                //？？？？？？？？？？？？？释放锁的空间，以及destroy锁
		it++;
	}
        free_locks();
}
//计算块对应的文件偏移
//0
f_type Subgraph::get_offset(b_type num){
	return sizeof(SubgraphHeader)+num*head.block_size;
}
//对文件新增的部分建立索引，文件新增的部分还没有取到内存中
//1
void Subgraph::update_index(){
	//计算新增部分的块的数目
	io.seekg(0,fstream::end);
	f_type file_size=io.tellg();
	b_type num=head.block_num;
	b_type blocks=((long)file_size-sizeof(SubgraphHeader)-head.block_size*num)/head.block_size;
	//开辟一个块的内存(只需要块头的大小，不需要整个块)，依次为每个块建立索引，再写入到磁盘中对应的位置
	BlockHeader<char> *block=(BlockHeader<char>*)malloc(sizeof(BlockHeader<char>));
	int i;	
	for(i=0;i<blocks;i++){
		//block从0开始编号
		block->number=num+i;
		block->next=head.free_head;
		head.free_head=block->number;
		head.free_num++;
		head.block_num++;
		io.seekp(get_offset(block->number));
		io.write((char*)block,sizeof(BlockHeader<char>));
	}
	free(block);	
}
//分配一个空闲块，无论空闲块是否还有，参数为块的类型，返回块号(要注意的是，require后的这个块已经脱离了控制，如果当时没用到，则会丢失)
//由于删除操作是用标志位的方式，所以暂时没有写返还块的操作
//2 整个操作加锁，保证其是串行的
b_type Subgraph::require(uint32_t type){
        Lock(require_lock); 
	if(head.free_num==0){
		//如果空闲块为0，则扩展子图文件，并建索引
		add_file();//按默认配置的大小分配
		update_index();//对分配的文件建立索引
	}
	if(head.free_num>0)
		return requireRaw(type);
	else{
                Unlock(require_lock); 
		return INVALID_BLOCK;//返回无效块
        }
}
//空闲块还有的时候，分配一个块，返回块的编号，只是分配工作，还需要get_block函数来返回该块
//2 和require操作配合加锁
b_type Subgraph::requireRaw(uint32_t type){
	//如果还有空闲块，则直接剥离出一个块，初始化块头，修改子图的头，并返回块地址
	b_type number=head.free_head;
	//分配一块内存，再把文件中的块读入到这个内存里
	BlockHeader<char> *block=(BlockHeader<char>*)malloc(head.block_size);
	io.seekg(get_offset(number));
	io.read((char*)block,head.block_size);
	//初始化块头公共的部分，比如块的类型，块的大小，修改子图的一些字段，比如剩余空闲块，链表头，块内部的内容（索引等）由块初始化来完成，
	head.free_head=block->next;
	head.free_num--;
	block->type=type;
	//不同的块类型，容量不一样
	if(type==1){
		block->capacity=(head.block_size-sizeof(BlockHeader<Vertex>))/sizeof(Content<Vertex>);
	}

	if(type==2){
		block->capacity=(head.block_size-sizeof(BlockHeader<Edge>))/sizeof(Content<Edge>);
	}
	if(type==3){
		block->capacity=(head.block_size-sizeof(BlockHeader<Index>))/sizeof(Content<Index>);
	}
	//把块头的内容写入到文件里，可以只写块头的大小，也可以写块的大小
	io.seekp(get_offset(number));
	io.write((char*)block,head.block_size);
	free(block);
        Unlock(require_lock);
	return number;	
}
//根据缓存中的块，得到Node类
//1
Node* Subgraph::block2node(void*block){
        b_type num=((BlockHeader<char>*)block)->number;
        Lock(cache_lock);
        Node* tmp=cache.find(num)->second;//map结构线程不安全，所以要防止和get_block中的增删操作并发，加锁 
        Unlock(cache_lock);
        return tmp;
}
//根据缓存中的块号，得到Node类
//1
Node* Subgraph::num2node(b_type num){
        Lock(cache_lock);
        Node* tmp=cache.find(num)->second;//map结构线程不安全，所以要防止和get_block中的增删操作并发，加锁 
        Unlock(cache_lock);
        return tmp;
}
//释放锁
//1 只有获取了该块的线程才会调用这个函数来释放锁
void Subgraph::unlock2block(void*block){
        Node* node=block2node(block);
        Unlock(node->lock);
}
//释放锁
//1 只有获取了该块的线程才会调用这个函数来释放锁
void Subgraph::unlock2num(b_type num){
        Node* node=num2node(num);
        Unlock(node->lock);
}
//得到一个块，该块号要存在，如果块缓存在cache中，则直接返回，如果没有缓存，则从文件中读取到cache，有必要的时候要移除一个cache，还没考虑到锁
//块存在hash表中，并且创建链表，新块放置在链头，旧块放置在链尾
//is_new代表该块是新分配的块还是旧块，1为新块   is_hash代表要不要为该块创建hash，1为创建  is_lock为要不要对返回的块先加锁，防止已经拥有该块的线程又再获取该块，则死锁了
//2 加锁，整个过程保证串行，得到的块在返回之前要先获取锁，不然可能会在下次操作该块时，已经被别的线程释放掉了
void* Subgraph::get_block(b_type number,char is_new,char is_hash,int is_lock){
        //cout<<"coming-----"<<endl;
        if(is_lock==1) Lock(getblock_lock);//获取一个块是一个串行动作，所以开始就要获取锁，获取一个已经占有的块时，就不需要再次获取锁了
	//cout<<"yeah get lock"<<endl;
        //sleep(3);
        //cout<<"sleep over"<<endl;
        c_it node_it=cache.find(number);
	void *block;
	Node *node;
	if(node_it!=cache.end()){
		//如果该块在缓存中，则返回块指针
                Node *node_tmp=node_it->second;
                if(is_lock==1) {
                     ((BlockHeader<char>*)(node_tmp->block))->fix++;//把该块的占据位加1，表示，这个块不能被移除
                     Unlock(getblock_lock);
                     Lock(node_tmp->lock);
                     ((BlockHeader<char>*)(node_tmp->block))->fix--;//获得这个块后，就可把占据位减1了
                }
		return node_tmp->block;
	}
	else{
		//如果该块不在缓存中，则读入该块
		if(!(cache.size()<atoi(getenv("CACHESZ")))){
			//如果缓存满了，则移除链表中的最后一个块，被锁住的块不能移除，删除前要判断该块是否脏了，脏了就要写入到文件
			node=last;
			while(node!=NULL){
				//遍历缓存，遇到没锁住的块并且占据位置0就停止，基本上是常数时间，被盯住的块一般是在链表头，还可以改进成LRU，但效果可能不是很明显
				if((((BlockHeader<char>*)(node->block))->fix==0)&&Trylock(node->lock)==0){
                                    Unlock(node->lock);//释放锁
                                    break;
                                }else 
                                    node=node->pre;
                                if(node==NULL){
                                    //如果遍历后没有可以删除的块，则休眠1秒，再继续从尾遍历
                                    sleep(1000);
                                    node=last;
                                }
			}
			if(((BlockHeader<char>*)(node->block))->clean==1){
				//块脏了，则写入到文件中
				io.seekp(get_offset(((BlockHeader<char>*)(node->block))->number));
				io.write((char*)(node->block),head.block_size);
			}
			//释放块所占的内存，在缓存结构里移除，以及更新链表
			//更新链表要分3种情况，该块在末尾，链表中，头部，！！！！！！！！多线程的时候要重点考虑这里的锁机制，假设缓存不会只有一个块，否则会有问题
			if(node->pre==NULL){
				//头部
				Node *tmp=node->next;
				tmp->pre=NULL;
				first=tmp;
			}else{
				if(node->next==NULL){
					//尾部
					Node *tmp=node->pre;
					tmp->next=NULL;
					last=tmp;
				}else{
					//中间
					Node *tmp_pre=node->pre;
					Node *tmp_next=node->next;
					tmp_pre->next=tmp_next;
					tmp_next->pre=tmp_pre;
				}
			}
			Node *tmp=node;
                        Lock(cache_lock);
			cache.erase(((BlockHeader<char>*)(node->block))->number);//在子图hash表里面移除元素
                        Unlock(cache_lock);
			if(((BlockHeader<char>*)(tmp->block))->is_hash==1)
				delete ((BlockHeader<char>*)(tmp->block))->data_hash;//释放块内的hash表内存
			Destroylock(tmp->lock);//消除锁
                        free(tmp->lock);//释放锁占的内存空间
                        free(tmp);//释放该块所占的内存
			delete_count++;
		}
		//分配内存，把块读进来	
		node=(Node*)malloc(sizeof(Node)+head.block_size);
		node->block=node+1;
                node->lock=Getlock();//创建锁
                Initlock(node->lock,NULL);//初始化锁
                Lock(node->lock);//获取该块之前，要先获取锁
		block=node->block;
		io.seekg(get_offset(number));
		io.read((char*)(block),head.block_size);
		((BlockHeader<char>*)block)->clean=0;//刚进来的块是干净的	
		((BlockHeader<char>*)block)->fix=0;//刚进来的块没有被盯住
		((BlockHeader<char>*)block)->is_hash=0;//还没有创建hash表
		((BlockHeader<char>*)block)->data_hash=NULL;
		if(is_new==1){
			((BlockHeader<char>*)block)->list_head=INVALID_INDEX;//这个在block的init函数会置位的，提前是为了hash初始化的统一
		}
		//只有块在内存的时候，才会把块的data字段指向正确的块内容区域
		((BlockHeader<char>*)block)->data=(Content<char>*)((BlockHeader<char>*)block+1);
		//把块加入缓存中
                Lock(cache_lock);
		cache[number]=node;
                Unlock(cache_lock);
		//更新链表
		if(first==NULL){
			first=last=node;
			node->pre=NULL;
			node->next=NULL;
		}else{
			node->pre=NULL;
			first->pre=node;
			node->next=first;
			first=node;
		}
		if(is_hash==1){
			char o=((BlockHeader<char>*)block)->type;
			if(o==1)
				((BlockHeader<Vertex>*)block)->init_hash();//创建该块的hash表
			if(o==2)
				((BlockHeader<Edge>*)block)->init_hash();//创建该块的hash表
			if(o==3)
				((BlockHeader<Index>*)block)->init_hash();//创建该块的hash表
		}
                Unlock(getblock_lock);
		return block;
	}
}

//增加一个顶点，检查顶点是否已经存在，已经存在则返回1，不存在就插入，返回0。直接在顶点块链表的末尾的空闲块插入一个新顶点，然后更新顶点索引
int Subgraph::add_vertex(Vertex &vertex,char is_hash){
        Lock(vertex_lock);
	if(vertex_is_in(vertex.id)) {
                Unlock(vertex_lock);
                return 1;
        }
	//在顶点块的链表中找到一个有空闲位置的块，只有链表的尾块可能没有满
	BlockHeader<Vertex> *b;
	b_type num;
	if(head.vertex_tail==INVALID_BLOCK){
		//如果链表尾是空，则说明还没有顶点块，则申请一个顶点块
		num=require(1);
		b=(BlockHeader<Vertex> *)get_block(num,1,is_hash);//不需要盯住该块
		//更新子图的顶点链表索引，双向链表
		head.vertex_head=num;
		head.vertex_tail=num;
		b->pre=INVALID_BLOCK;
		b->next=INVALID_BLOCK;	
		//初始化该块的内部索引
		b->init_block();
		
	}else{	
		//链表尾不是空，说明有顶点块，取出链表尾块
		b=(BlockHeader<Vertex> *)get_block(head.vertex_tail,0,is_hash);//不需要盯住该块
		num=head.vertex_tail;
		if(b->size==b->capacity){
			//块满了，申请一个块
			num=require(1);
			b->next=num;
			b->clean=1;//块修改后一定要记得把脏位置1
                        unlock2block(b);//释放上一块的锁，已经用不着了                 
			b=(BlockHeader<Vertex> *)get_block(num,1,is_hash);//不需要盯住该块
			b->next=INVALID_BLOCK;
			b->pre=head.vertex_tail;
			head.vertex_tail=num;
			b->init_block();
		}
	}
	//初始化顶点的一些信息，刚刚插入的顶点是没有边块和索引块的
	vertex.status=0;
	vertex.size=0;
	vertex.head=INVALID_BLOCK;
	vertex.tail=INVALID_BLOCK;
	vertex.index=INVALID_BLOCK;
	b->add_content(vertex);
	b->clean=1;
        unlock2block(b);//释放该块的锁
	head.vertex_num++;//子图对顶点的统计信息
	vertex_index.insert_kv(vertex.id,num);//把顶点id和顶点所在的块号存入索引中
        Unlock(vertex_lock);//该操作操作完后，释放顶点的锁
	return 0;
}
//顶点是否存在，查看索引
//1
bool Subgraph::vertex_is_in(v_type id){
	list<b_type> r;
	vertex_index.find_values(id,r);	
	if(r.size()>0) return true;
	else return false;
}
//返回顶点，如果顶点不存在，则返回无效顶点，顶点id为INVALID_VERTEX
//1
Vertex Subgraph::get_vertex(v_type id,char is_hash){
	b_type num;
	Vertex *v=get_vertex_raw(id,&num,is_hash);
	if(v==NULL){
		return Vertex(INVALID_VERTEX);
	}
        Vertex res=*v;//为了释放顶点块，要把缓存中的顶点内容拷贝出来，再释放顶点所在块的锁，否则先释放锁后，可能这个块会被移除掉，再来引用这个顶点就会失效了
        unlock2num(num);
	return res;	
}
//返回顶点的指针，不存在，则返回空指针，这个函数不能释放顶点块，由调用它的函数来释放
//2 涉及到顶点操作的都是串行，因为顶点的缓冲区还没有加锁机制来实现并发
Vertex* Subgraph::get_vertex_raw(v_type id,b_type *num,char is_hash){
	//通过索引，寻找顶点所在的块
        Lock(vertex_lock); 
	list<b_type> r;
	vertex_index.find_values(id,r);
	if(r.size()!=1) {
                Unlock(vertex_lock);
                return NULL;//如果没有该顶点存在或者顶点有多个，则返回空指针
        }
	b_type p=*(r.begin());
	BlockHeader<Vertex> *b=(BlockHeader<Vertex> *)get_block(p,0,is_hash);
	uint32_t i=b->list_head;
	while(i!=INVALID_INDEX){
		if(b->data[i].content.id==id) {
			*num=b->number;
                        Unlock(vertex_lock);//获得了顶点的块后，才释放锁
			return (Vertex *)(b->data+i);//返回指针
		}
		i=b->data[i].next;	
	}
        unlock2block(b);//如果块里面没有顶点，则释放该块，这种情况目前不会发生，因为顶点暂时不能删除		
        Unlock(vertex_lock);	
	return NULL;//如果遍历块还是没有，则返回空指针
}
//加入一条边，参数1是边所属顶点的id，参数2是边，可能同时操作多块，需要盯住块，成功了返回0，顶点不存在，失败了，返回1
//1
int Subgraph::add_edge(v_type id,Edge &e,char is_hash){
	b_type num;//顶点所在的块号
	b_type b_num;//边所在的块号
	//获得顶点，以及顶点所在的块
	Vertex *v=get_vertex_raw(id,&num,is_hash);
        cout<<"add start "<<id<<" "<<e.id<<endl;
	if(v==NULL) {
             return 1;
        }
	//查询该顶点的索引块，得到边要加入的块号
	b_num=index_edge(v,e.id,num,is_hash);
	BlockHeader<Edge>* block=(BlockHeader<Edge>*)get_block(b_num,0,is_hash,0);//获取的顶点块还被占有，所以无锁方式获取
	e.status=0;//刚插入的边的状态为0，表示存在
	block->add_content(e);//把边加入块中	
	block->clean=1;
        unlock2num(b_num);//写操作完后，才释放顶点块和边块,??????不足之处，该块别的顶点没法操作，但可以并行操作别的块的顶点
        unlock2num(num);
        cout<<"add over "<<id<<" "<<e.id<<endl;
	return 0;
} 
//查询顶点的索引块，参数1是顶点的指针，参数2是边的目的顶点的id，参数3是顶点的块号，返回边要加入的块号，而且确保这个块肯定还能容纳边
//2 
b_type Subgraph::index_edge(Vertex* v,v_type id,b_type num,char is_hash){
	BlockHeader<Index> *b=NULL;
	//1：获取索引块链表的第一块，然后盯住这个块
	if(v->index==INVALID_BLOCK){
		//如果顶点还没有索引块，则创建一个索引块，以后这个块一直是头块，采取分裂的方式扩容
		b_type new_num=require(3);
		//加入到顶点的索引链表中
		v->index=new_num;
		//修改了顶点的数据，则要给顶点所在的块的脏位置1
		BlockHeader<Vertex> *bv=(BlockHeader<Vertex> *)get_block(num,0,is_hash,0);//对已经占有的块，再次获取时，一定要用不加锁的方式
		bv->clean=1;
                //获取新分配的索引块
		b=(BlockHeader<Index> *)get_block(new_num,1,is_hash);
		b->init_block();
		b->next=INVALID_BLOCK;
		b->pre=INVALID_BLOCK;	
		b->clean=1;
	}else{
		//顶点有索引块，则取出第一块
		b=(BlockHeader<Index> *)get_block(v->index,0,is_hash);	
	}
	//2：查询边应该插入的索引块
	while(true){
		if((b->min==INVALID_VERTEX)||(id<b->min)){
			//如果下一块最小值为无效，或者插入的边的id小于下一块最小的id，说明该块就是要找的索引块
			uint32_t i=b->list_head;
			if(i==INVALID_INDEX){
				//只有该顶点插入第一条边的时候，list_head才会为无效值，才会主动创建边的块
				Index in;//创建一个索引项
				in.id=INVALID_VERTEX;//下一个索引项不存在，最小值就是无效的
				in.target=require(2);//获取一个边块号
				b->add_content(in);//把索引项插入索引块中
				b->clean=1;//索引块脏位置1
				BlockHeader<Edge> *insert_b=(BlockHeader<Edge>*)get_block(in.target,1,is_hash);//获取边块，这个块一直是块头，采取分裂方式扩容
				//把块加入到顶点的边链表中，作为块头，同时把顶点所在块脏位置1
				v->head=in.target;
				insert_b->pre=INVALID_BLOCK;
				insert_b->next=INVALID_BLOCK;
				insert_b->min=in.id;
				BlockHeader<Vertex> *bv=(BlockHeader<Vertex> *)get_block(num,0,is_hash,0);//记得用无锁方式
				bv->clean=1;
				//初始化该块
				insert_b->init_block();
				insert_b->clean=1;
				unlock2block(b);//释放索引块
				return in.target;	
			}else{
				while(true){
					//遍历该块的索引项，找出边要插入的块
					if((b->data[i].content.id==INVALID_VERTEX)||(id<b->data[i].content.id)){
						//如果下一个索引项的最小值无效或者大于插入边的id，则这个索引项就是要找的
						BlockHeader<Edge> *insert_b=(BlockHeader<Edge>*)get_block(b->data[i].content.target,0,is_hash);//根据索引获得边块
						if(insert_b->size<insert_b->capacity){
							//如果该边块还没有满，则返回该块
							unlock2block(b);//释放索引块
							return insert_b->number;
						}else{
							//如果满了，则要进行边块的分裂
							b_type new_block_num=require(2);//从空闲块中释放一个新边块
							BlockHeader<Edge> *new_block=(BlockHeader<Edge> *)get_block(new_block_num,1,is_hash);//这个块不需要初始化	
							insert_b->split(new_block,this);//分裂块
							//添加新块的索引
							Index in;
							in.target=new_block_num;
							if(b->data[i].content.id==INVALID_VERTEX){
							}
							in.id=new_block->min;//新索引项的值
							b->data[i].content.id=insert_b->min;//修改旧索引项的值
							if(b->size<b->capacity){
								//如果索引块内容没满，添加新的索引项
								b->add_content(in);
							}else{
								//索引块满了，分裂索引块
								b_type new_index_num=require(3);
								BlockHeader<Index> *new_index=(BlockHeader<Index> *)get_block(new_index_num,1,is_hash);//这个块不需要初始化
								b->split(new_index,this);
								if(in.id<new_index->data[new_index->list_head].content.id){
									//如果索引项的值小于新索引块的第一项的值，则把索引插入旧块
									b->add_content(in);
									//一定要更新旧块的min
									b->min=b->data[b->list_tail].content.id;
								}else{
									//否则插入新索引块
									new_index->add_content(in);
									new_index->clean=1;
									new_index->min=new_index->data[new_index->list_tail].content.id;//这句也可以不要
								}
								unlock2block(new_index);
							}
							b->clean=1;
							unlock2block(b);
							//判断该边要插入哪个块，旧块和新块之间选择
							if(id<insert_b->min){
								//插入旧块，则释放新块的锁
                                                                unlock2block(new_block);
								return insert_b->number;
							}else{
                                                                unlock2block(insert_b);
								return new_block->number;
							}
						}
					}else{
						i=b->data[i].next;
					}
					
				}
			}	
			
		}else{
			b_type next_num=b->next;//在释放该块之前得到下一个索引块，以免在中间把该块移除出去了
                        unlock2block(b);//释放索引块
			b=(BlockHeader<Index> *)get_block(next_num,0,is_hash);
		}
	}
	
}
//根据源顶点和目的顶点，读取所有的边，结果存入到list结构中，源顶点不存在返回1，存在返回0
//2 读操作的原则是，在获取到了下一个块的锁之后，才能释放上一个块的锁，这种有序性，使得读一个顶点的过程中，还可以并发写一个顶点
int Subgraph::read_edges(v_type s_id,v_type d_id,list<Edge_u>& edges,char is_hash){
	b_type num;
	Vertex* v=get_vertex_raw(s_id,&num,is_hash);//得到顶点
        cout<<"read tart "<<s_id<<" "<<d_id<<endl; 
	if(v==NULL) return 1;
	b_type index=v->index;
        int free_num=num;
	while(index!=INVALID_BLOCK){
		BlockHeader<Index>* in_block=(BlockHeader<Index>*)get_block(index,0,is_hash);//获取索引块
                unlock2num(free_num);//获取了索引块后，就可以释放顶点块的锁，或者前一个索引块的锁
                free_num=in_block->number;
		//索引块里面一定有索引项
		if(in_block->min==INVALID_VERTEX||d_id<in_block->min){
			uint32_t item=in_block->list_head;
			BlockHeader<Edge>* e_block=NULL;
			while(item!=INVALID_INDEX){
				if(in_block->data[item].content.id==INVALID_VERTEX||d_id<in_block->data[item].content.id){
					e_block=(BlockHeader<Edge>*)get_block(in_block->data[item].content.target,0,is_hash);//获取边块
                                        unlock2num(free_num);
                                        free_num=e_block->number;
					while(true){
						//遍历一些块，因为可能有很多条边
						if(e_block->get_contents(s_id,d_id,edges)){
							b_type pre_num=e_block->pre;
							if(pre_num!=INVALID_BLOCK){ 
								e_block=(BlockHeader<Edge>*)get_block(pre_num,0,is_hash);
                                                                unlock2num(free_num);
                                                                free_num=e_block->number;    
                                                        }
							else{
                                                                unlock2num(free_num);    
                                                                cout<<"read over "<<s_id<<" "<<d_id<<endl;
								return 0;
                                                        }
						}else{
                                                        unlock2num(free_num);
                                                        cout<<"read over "<<s_id<<" "<<d_id<<endl;
							return 0;
                                                }
					}				
				}else{
					item=in_block->data[item].next;
				}
			}	
		}else{
			index=in_block->next;
		}
	}
	//出了循环，说明没有这条边
        unlock2num(free_num);
        cout<<"read over "<<s_id<<" "<<d_id<<endl;
	return 0;	
}
//读取顶点所有的边，顶点不存在返回1，存在返回0
//2
int Subgraph::read_all_edges(v_type id,list<Edge_u>& edges,char is_hash){
	b_type tmp;
	Vertex *v=get_vertex_raw(id,&tmp,is_hash);
	if(v==NULL) return 1;
        b_type free_num=tmp;
	b_type num=v->head;
	while(num!=INVALID_BLOCK){
		BlockHeader<Edge>* edge_block=(BlockHeader<Edge>*)get_block(num,0,is_hash);
                unlock2num(free_num);
                free_num=edge_block->number;
		edge_block->get_all_contents(id,edges);
		num=edge_block->next;
	}
        unlock2num(free_num);
	return 0;
}
//根据源顶点和目的顶点，读一条边，不需要盯块，这个函数没用到暂时，所以没有写并发操作
Edge* Subgraph::read_edge(v_type s_id,v_type d_id,char is_hash){
	b_type num;
	Vertex* v=get_vertex_raw(s_id,&num,is_hash);//得到顶点
        if(v==NULL) return NULL;//如果顶点不存在，则返回空指针
	b_type index=v->index;
	while(index!=INVALID_BLOCK){
		BlockHeader<Index>* in_block=(BlockHeader<Index>*)get_block(index,0,is_hash);//获取索引块
		//索引块里面一定有索引项
		if(in_block->min==INVALID_VERTEX||d_id<in_block->min){
			uint32_t item=in_block->list_head;
			BlockHeader<Edge>* e_block=NULL;
			while(item!=INVALID_INDEX){
				if(in_block->data[item].content.id==INVALID_VERTEX||d_id<in_block->data[item].content.id){
					 e_block=(BlockHeader<Edge>*)get_block(in_block->data[item].content.target,0,is_hash);//获取边块
                                         
					 return e_block->get_content(d_id);						
				}else{
					item=in_block->data[item].next;
				}
			}
			if(e_block==NULL) {
				return NULL;
			}
			else
				return e_block->get_content(d_id);	
		}else{
			index=in_block->next;
		}
	}
	//出了循环，说明没有这条边
	return NULL;	
}
//------测试函数，根据索引来遍历顶点的边
void Subgraph::index_output_edge(v_type id,char is_hash){
	b_type tmp;
	Vertex *v=get_vertex_raw(id,&tmp,is_hash);
	b_type num=v->index;
	while(num!=INVALID_BLOCK){
		BlockHeader<Index>* index_block=(BlockHeader<Index>*)get_block(num,0,is_hash);
		index_block->fix++;
		b_type index_num=index_block->list_head;
		while(index_num!=INVALID_INDEX){
			BlockHeader<Edge>* edge_block=(BlockHeader<Edge>*)get_block(index_block->data[index_num].content.target,0,is_hash);

			//edge_block->output();
			uint32_t i_num=edge_block->list_head;
			while(i_num!=INVALID_INDEX){
				edge_block->get_content(edge_block->data[i_num].content.id);
				i_num=edge_block->data[i_num].next;
			}


			index_num=index_block->data[index_num].next;
		}
		index_block->fix--;
		num=index_block->next;
	}
}
//-----------测试函数，输出所有的顶点
void Subgraph::all_vertex(char is_hash){
	b_type p=head.vertex_head;
	cout<<"all vertex  total:"<<head.vertex_num<<endl;
	while(p!=INVALID_BLOCK){
		BlockHeader<Vertex> *b=(BlockHeader<Vertex> *)get_block(p,0,is_hash);
		b->output();	
		p=b->next;	
	}	
}
//-------------
//------------测试函数，输出顶点的所有边
void Subgraph::output_edge(v_type id,char is_hash){
	b_type tmp;
	Vertex *v=get_vertex_raw(id,&tmp,is_hash);
	b_type num=v->head;
	while(num!=INVALID_BLOCK){
		BlockHeader<Edge>* edge_block=(BlockHeader<Edge>*)get_block(num,0,is_hash);
		edge_block->output();
		num=edge_block->next;
	}
}

//------------

