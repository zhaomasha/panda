#include "panda_subgraph.hpp"
//新建一个子图文件(子图文件存在，则会被清零)，初始化为默认的大小（操作系统的栈大小要调整，否则会段错误）
void Subgraph::init(string name){
	//文件存在则清零，不存在则创建
	filename=name;
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
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
//增加一个顶点，暂时不考虑块中顶点的删除，删除只是添加一个标记
void Subgraph::add_vertex(Vertex &vertex){
	//在顶点块的链表中找到一个有空闲位置的块，只有链表的尾块可能没有满
	BlockHeader<Vertex> *b;
	if(head.vertex_tail==INVALID_BLOCK){
		//如果链表尾是空，则说明还没有顶点块，则申请一个顶点块
		b_type num=require(1);
		b=(BlockHeader<Vertex> *)get_block(num);
		//更新子图的顶点链表索引，双向链表
		head.vertex_head=num;
		head.vertex_tail=num;
		b->pre=INVALID_BLOCK;
		b->next=INVALID_BLOCK;	
		//初始化该块的内部索引
		b->init_block();
		
	}else{	
		//链表尾不是空，说明有顶点块，取出链表尾块
		b=(BlockHeader<Vertex> *)get_block(head.vertex_tail);
		if(b->size==b->capacity){
			//块满了，申请一个块
			b_type num=require(1);
			b->next=num;
			b->clean=1;//块修改后一定要记得把脏位置1
			b=(BlockHeader<Vertex> *)get_block(num);
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
//加入一条边，参数1是边所属顶点的id，参数2是边
void Subgraph::add_edge(v_type id,Edge &e){
	b_type num;
	//获得顶点
	Vertex *v=get_vertex(id,&num);
	//查询该顶点的索引块，得到边要加入的块号
		
} 
//查询顶点的索引块，参数1是顶点的指针，参数2是边的目的顶点的id，返回边要加入的块号
b_type Subgraph::index_edge(Vertex* v,v_type id,b_type num){
	BlockHeader<Index> *b=NULL;
	//1：获取索引块
	if(v->index==INVALID_BLOCK){
		//如果顶点还没有索引块，则创建一个索引块
		b_type new_num=require(3);
		//加入到顶点的索引链表中
		v->index=new_num;
		//修改了顶点的数据，则要给顶点所在的块的脏位置1
		b=(BlockHeader<Index> *)get_block(num);
		b->clean=1;
                //获取新分配的索引块，!!!!!!!!!!!!现在还没有加锁，可能会把顶点所在的块删掉，从而导致错误!!!!!!!!!!
		b=(BlockHeader<Index> *)get_block(new_num);
		b->init();
		b->next=INVALID_BLOCK;	
		b->clean=1;
		
	}else{
		//顶点有索引块，则取出第一块
		b=(BlockHeader<Index> *)get_block(v->index);	
	}
	//2：查询边应该插入的位置
	b_type	
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
//得到一个块，该块号要存在，如果块缓存在cache中，则直接返回，如果没有缓存，则从文件中读取到cache，有必要的时候要移除一个cache，还没考虑到锁
void* Subgraph::get_block(b_type number){
	c_it block=cache.find(number);
	void *p;
	if(block!=cache.end()){
		//如果该块在缓存中，则返回块指针
		return block->second;
	}
	else{
		//如果该块不在缓存中，则读入该块
		if(!(cache.size()<atoi(getenv("CACHESZ")))){
			//如果缓存满了，则移除一个块，随机移除，没有考虑到替换策略，删除前要判断该块是否脏了，脏了就要写入到文件
			srand((unsigned)time(0));
			int ff=rand();
			int del=ff%cache.size();
			for(block=cache.begin();del>0;del--) block++;
			cout<<"删除块"<<block->first<<endl;
			if(((BlockHeader<char>*)(block->second))->clean==1){
				//块脏了，则写入到文件中
				cout<<"写文件"<<block->first<<endl;
				io.seekp(get_offset(block->first));
				io.write((char*)(block->second),head.block_size);
			}
			cache.erase(block);
		}
		//分配内存，把块读进来	
		p=malloc(head.block_size);
		io.seekg(get_offset(number));
		io.read((char*)(p),head.block_size);
		((BlockHeader<char>*)p)->clean=0;//刚进来的块是干净的	
		//只有块在内存的时候，才会把块的data字段指向正确的块内容区域
		((BlockHeader<char>*)p)->data=(Content<char>*)((BlockHeader<char>*)p+1);
		//把块加入缓存中
		cache[number]=p;
		return p;
	}
}


