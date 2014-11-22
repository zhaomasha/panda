#include "panda_subgraph.hpp"
//新建一个子图文件(子图文件存在，则会被清零)，初始化为默认的大小（操作系统的栈大小要调整，否则会段错误）
void Subgraph::init(string name){
	//文件存在则清零，不存在则创建
	filename=name;
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
	first=last=NULL;
	delete_count=0;
	//初始化默认大小
	add_file(atoi(getenv("INITSZ")));
}
//子图扩张，默认大小可以配置，也可以指定
void Subgraph::add_file(uint32_t size){
	io.seekp(0,fstream::end);
	char tmp[1024*1024*size];
	io.write(tmp,sizeof(tmp));
} 
//格式化子图，创建子图的头，初始化索引结构等等，参数是该子图的block大小
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
}
//计算块对应的文件偏移
f_type Subgraph::get_offset(b_type num){
	return sizeof(SubgraphHeader)+num*head.block_size;
}
//对文件新增的部分建立索引，文件新增的部分还没有取到内存中，否则会有一致性的问题
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
b_type Subgraph::require(uint32_t type){
	if(head.free_num==0){
		//如果空闲块为0，则扩展子图文件，并建索引
		add_file();//按默认配置的大小分配
		update_index();//对分配的文件建立索引
	}
	if(head.free_num>0)
		return requireRaw(type);
	else
		return INVALID_BLOCK;//返回无效块
	
}
//空闲块还有的时候，分配一个块，返回块的编号，只是分配工作，还需要get_block函数来返回该块
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
	return number;	
}
//增加一个顶点，暂时不考虑块中顶点的删除，删除只是添加一个标记，同时只操作一个块，不需要盯住
void Subgraph::add_vertex(Vertex &vertex){
	//在顶点块的链表中找到一个有空闲位置的块，只有链表的尾块可能没有满
	BlockHeader<Vertex> *b;
	if(head.vertex_tail==INVALID_BLOCK){
		//如果链表尾是空，则说明还没有顶点块，则申请一个顶点块
		b_type num=require(1);
		b=(BlockHeader<Vertex> *)get_block(num);//不需要盯住该块
		//更新子图的顶点链表索引，双向链表
		head.vertex_head=num;
		head.vertex_tail=num;
		b->pre=INVALID_BLOCK;
		b->next=INVALID_BLOCK;	
		//初始化该块的内部索引
		b->init_block();
		
	}else{	
		//链表尾不是空，说明有顶点块，取出链表尾块
		b=(BlockHeader<Vertex> *)get_block(head.vertex_tail);//不需要盯住该块
		if(b->size==b->capacity){
			//块满了，申请一个块
			b_type num=require(1);
			b->next=num;
			b->clean=1;//块修改后一定要记得把脏位置1
			b=(BlockHeader<Vertex> *)get_block(num);//不需要盯住该块
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
	head.vertex_num++;//子图对顶点的统计信息
}
//返回顶点的指针，没有建索引，参数2用来记录该顶点所属的块号，当该块的内容改变时，要把脏位置1
Vertex* Subgraph::get_vertex(v_type id,b_type *num){
	//遍历的方式寻找顶点，遍历每一个顶点块，每一个块中的所有顶点，找到顶点后就返回该顶点的指针
	b_type p=head.vertex_head;
	while(p!=INVALID_BLOCK){
		BlockHeader<Vertex> *b=(BlockHeader<Vertex> *)get_block(p);
		uint32_t i=b->list_head;
		while(i!=INVALID_INDEX){
			if(b->data[i].content.id==id) {
				*num=b->number;
				return (Vertex *)(b->data+i);//返回指针
			}
			i=b->data[i].next;	
		}			
		p=b->next;
	}	
	return NULL;
}
//加入一条边，参数1是边所属顶点的id，参数2是边，可能同时操作多块，需要盯住块
void Subgraph::add_edge(v_type id,Edge &e){
	b_type num;//顶点所在的块号
	b_type b_num;//边所在的块号
	//获得顶点，以及顶点所在的块
	Vertex *v=get_vertex(id,&num);
	//获取顶点所在的块，把此块盯住
	BlockHeader<Vertex> *b=(BlockHeader<Vertex>*)get_block(num);
	(b->fix)++;
	//查询该顶点的索引块，得到边要加入的块号
	b_num=index_edge(v,e.id,num);
	(b->fix)--;//获得边的块号后，顶点所在的块可以不需要盯住了
	BlockHeader<Edge>* block=(BlockHeader<Edge>*)get_block(b_num);
	e.status=0;//刚插入的边的状态为0，表示存在
	block->add_content(e);//把边加入块中	
	block->clean=1;
} 
//查询顶点的索引块，参数1是顶点的指针，参数2是边的目的顶点的id，参数3是顶点的块号，返回边要加入的块号，而且确保这个块肯定还能容纳边
b_type Subgraph::index_edge(Vertex* v,v_type id,b_type num){
	BlockHeader<Index> *b=NULL;
	//1：获取索引块链表的第一块，然后盯住这个块
	if(v->index==INVALID_BLOCK){
		
		//如果顶点还没有索引块，则创建一个索引块，以后这个块一直是头块，采取分裂的方式扩容
		b_type new_num=require(3);
		//加入到顶点的索引链表中
		v->index=new_num;
		//修改了顶点的数据，则要给顶点所在的块的脏位置1
		BlockHeader<Vertex> *bv=(BlockHeader<Vertex> *)get_block(num);
		bv->clean=1;
                //获取新分配的索引块
		b=(BlockHeader<Index> *)get_block(new_num);
		b->init_block();
		b->next=INVALID_BLOCK;
		b->pre=INVALID_BLOCK;	
		b->clean=1;
		//把索引块盯住
		b->fix++;
		
	}else{
		//顶点有索引块，则取出第一块
		b=(BlockHeader<Index> *)get_block(v->index);
		b->fix++;//一定要盯住，在这里调了好久的bug	
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
				BlockHeader<Edge> *insert_b=(BlockHeader<Edge>*)get_block(in.target);//获取边块，这个块一直是块头，采取分裂方式扩容
				//把块加入到顶点的边链表中，作为块头，同时把顶点所在块脏位置1
				v->head=in.target;
				insert_b->pre=INVALID_BLOCK;
				insert_b->next=INVALID_BLOCK;
				insert_b->min=in.id;
				insert_b->fix++;//要盯住这个块，不然就可能被替换掉
				BlockHeader<Vertex> *bv=(BlockHeader<Vertex> *)get_block(num);
				insert_b->fix--;
				bv->clean=1;
				//初始化该块
				insert_b->init_block();
				insert_b->clean=1;
				(b->fix)--;//找到块后，就可以不用盯住索引块了
				return in.target;	
			}else{
				while(true){
					//遍历该块的索引项，找出边要插入的块
					if((b->data[i].content.id==INVALID_VERTEX)||(id<b->data[i].content.id)){
						//如果下一个索引项的最小值无效或者大于插入边的id，则这个索引项就是要找的
						BlockHeader<Edge> *insert_b=(BlockHeader<Edge>*)get_block(b->data[i].content.target);//根据索引获得边块
						if(insert_b->size<insert_b->capacity){
							//如果该边块还没有满，则返回该块
							(b->fix)--;//找到块后，就可以不用盯住索引块了
							return insert_b->number;
						}else{
							//如果满了，则要进行边块的分裂
							insert_b->fix++;//由于要分裂，所以要把该块盯住
							b_type new_block_num=require(2);//从空闲块中释放一个新边块
							BlockHeader<Edge> *new_block=(BlockHeader<Edge> *)get_block(new_block_num);//这个块不需要初始化	
							new_block->fix++;//新块需要盯住，应为这两个edge块到最后还要用到，不能被中间换出，否则指针就乱指了
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
								BlockHeader<Index> *new_index=(BlockHeader<Index> *)get_block(new_index_num);//这个块不需要初始化
								new_index->fix++;
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
								new_index->fix--;
							}
							b->clean=1;
							(b->fix)--;//索引块也不需要盯住了
							//判断该边要插入哪个块，旧块和新块之间选择
							insert_b->fix--;//分裂完后，不需要盯住了
							new_block->fix--;
							if(id<insert_b->min){
								//插入旧块
								return insert_b->number;
							}else{
								return new_block->number;
							}
						}
					}else{
						i=b->data[i].next;
					}
					
				}
			}	
			
		}else{
			(b->fix)--;//不在这一块，则把盯住位释放，继续找下一块
			b=(BlockHeader<Index> *)get_block(b->next);
			(b->fix)++;//盯住新的块
		}
	}
	
}
//根据源顶点和目的顶点，读一条边，不需要盯块
Edge* Subgraph::read_edge(v_type s_id,v_type d_id){
	b_type num;
	Vertex* v=get_vertex(s_id,&num);//得到顶点
	b_type index=v->index;
	while(index!=INVALID_BLOCK){
		BlockHeader<Index>* in_block=(BlockHeader<Index>*)get_block(index);//获取索引块
		//索引块里面一定有索引项
		if(in_block->min==INVALID_VERTEX||d_id<in_block->min){
			uint32_t item=in_block->list_head;
			while(true){
				if(in_block->data[item].content.id==INVALID_VERTEX||d_id<in_block->data[item].content.id){
					BlockHeader<Edge>* e_block=(BlockHeader<Edge>*)get_block(in_block->data[item].content.target);//获取边块
					return e_block->get_content(d_id);						
				}else{
					item=in_block->data[item].next;
				}
			}
		}else{
			index=in_block->next;
		}
	}
	//出了循环，说明没有这条边
	return NULL;	
}

//------测试函数，根据索引来遍历顶点的边
void Subgraph::index_output_edge(v_type id){
	b_type tmp;
	Vertex *v=get_vertex(id,&tmp);
	b_type num=v->index;
	while(num!=INVALID_BLOCK){
		BlockHeader<Index>* index_block=(BlockHeader<Index>*)get_block(num);
		index_block->fix++;
		b_type index_num=index_block->list_head;
		while(index_num!=INVALID_INDEX){
			BlockHeader<Edge>* edge_block=(BlockHeader<Edge>*)get_block(index_block->data[index_num].content.target);
			edge_block->output();
			index_num=index_block->data[index_num].next;
		}
		index_block->fix--;
		num=index_block->next;
	}
}
//-----------测试函数，输出所有的顶点
void Subgraph::all_vertex(){
	b_type p=head.vertex_head;
	cout<<"all vertex  total:"<<head.vertex_num<<endl;
	while(p!=INVALID_BLOCK){
		BlockHeader<Vertex> *b=(BlockHeader<Vertex> *)get_block(p);
		b->output();	
		p=b->next;	
	}	
}
//-------------
//------------测试函数，输出顶点的所有边
void Subgraph::output_edge(v_type id){
	b_type tmp;
	Vertex *v=get_vertex(id,&tmp);
	b_type num=v->head;
	while(num!=INVALID_BLOCK){
		BlockHeader<Edge>* edge_block=(BlockHeader<Edge>*)get_block(num);
		edge_block->output();
		num=edge_block->next;
	}
}

//------------
//得到一个块，该块号要存在，如果块缓存在cache中，则直接返回，如果没有缓存，则从文件中读取到cache，有必要的时候要移除一个cache，还没考虑到锁
//块存在hash表中，并且创建链表，新块放置在链头，旧块放置在链尾
void* Subgraph::get_block(b_type number){
	c_it node_it=cache.find(number);
	void *block;
	Node *node;
	if(node_it!=cache.end()){
		//如果该块在缓存中，则返回块指针
		return node_it->second->block;
	}
	else{
		//如果该块不在缓存中，则读入该块
		if(!(cache.size()<atoi(getenv("CACHESZ")))){
			//如果缓存满了，则移除链表中的最后一个块，被盯住的块不能移除，删除前要判断该块是否脏了，脏了就要写入到文件
			node=last;
			while(node!=NULL){
				//遍历缓存，遇到没盯住的块就停止，基本上是常数时间，被盯住的块一般是在链表头，还可以改进成LRU，但效果可能不是很明显
				if(((BlockHeader<char>*)(node->block))->fix==0) break;
				else node=node->pre;
			}
			if(node==NULL) {
				return NULL;//如果块都被盯住，则返回空指针
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
			cache.erase(((BlockHeader<char>*)(node->block))->number);
			free(tmp);
			delete_count++;
		}
		//分配内存，把块读进来	
		node=(Node*)malloc(sizeof(Node)+head.block_size);
		node->block=node+1;
		block=node->block;
		io.seekg(get_offset(number));
		io.read((char*)(block),head.block_size);
		((BlockHeader<char>*)block)->clean=0;//刚进来的块是干净的	
		((BlockHeader<char>*)block)->fix=0;//刚进来的块没有被盯住	
		//只有块在内存的时候，才会把块的data字段指向正确的块内容区域
		((BlockHeader<char>*)block)->data=(Content<char>*)((BlockHeader<char>*)block+1);
		//把块加入缓存中
		cache[number]=node;
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
		return block;
	}
}
