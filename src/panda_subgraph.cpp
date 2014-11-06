#include "panda_subgraph.hpp"
//子图不存在时，新建一个子图文件，初始化为配置文件中的大小，操作系统的栈大小要调整，否则会段错误
void Subgraph::init(string name){
	//文件存在则清零，不存在则创建
	filename=name;
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
	add_file(atoi(getenv("INITSZ")));
}
//子图扩张，大小默认可以配置，也可以指定
void Subgraph::add_file(uint32_t size){
	io.seekp(0,fstream::end);
	char tmp[1024*1024*size];
	io.write(tmp,sizeof(tmp));
} 
//格式化子图，创建子图的头，初始化索引结构等等
void Subgraph::format(uint32_t block_size){
	head.free_head=INVALID_BLOCK;
	head.vertex_head=INVALID_BLOCK;		
	head.vertex_tail=INVALID_BLOCK;		
	head.vertex_num=0;
	head.free_num=0;
	head.block_num=0;
	head.block_size=block_size;
	node_head=NULL;
	node_tail=NULL;
	//初始化索引
	update_index();

	  	
}
//计算块对应的文件偏移
f_type Subgraph::get_offset(b_type num){
	return sizeof(SubgraphHeader)+num*head.block_size;
}
//对文件新增的部分建立索引，参数1是现在子图中块的数目，参数2是新增文件后的文件大小,文件新增的部分还没有取到内存中，否则会有一致性的问题
void Subgraph::update_index(){
	//计算新增部分的块的数目
	io.seekg(0,fstream::end);
	f_type file_size=io.tellg();
	b_type num=head.block_num;
	b_type blocks=((long)file_size-sizeof(SubgraphHeader)-head.block_size*num)/head.block_size;
	//开辟一个块的内存，依次为每个块建立索引，再写入到磁盘中对应的位置
	BlockHeader<b_type> *block=(BlockHeader<b_type>*)malloc(sizeof(BlockHeader<b_type>));
	int i;	
	for(i=0;i<blocks;i++){
		block->number=num+i;
		block->next=head.free_head;
		head.free_head=block->number;
		head.free_num++;
		head.block_num++;
		io.seekp(get_offset(block->number));
		io.write((char*)block,sizeof(BlockHeader<b_type>));
	}
	free(block);	
}

//分配一个空闲块，无论空闲块是否还有，参数为块的类型，每次分配都会导入一个块到内存中
void* Subgraph::require(uint32_t type){
	if(head.free_num==0){
		//如果空闲块为0，则扩展子图文件，并建索引
		add_file();//按默认配置的大小分配
		update_index();//对分配的文件建立索引
	}
	if(head.free_num>0)
		return requireRaw(type);
	else
		return NULL;
	
}
//空闲块还有的时候，返回一块
void* Subgraph::requireRaw(uint32_t type){
	//如果还有空闲块，则直接剥离出一个块，初始化块头，修改子图的头，并返回块地址
	b_type number=head.free_head;
	BlockHeader<void> *block=(BlockHeader<void>*)malloc(head.block_size);

	//分配一块内存，再把文件中的块读入到这个内存里
	io.seekg(get_offset(number));
	io.read((char*)block,head.block_size);
	//初始化块头公共的部分，比如块的类型，块的大小，块的数据指针，修改子图的一些字段，比如剩余空闲块，链表头
	block->size=0;
	block->list_head=INVALID_INDEX;
	block->data=block+1;
	head.free_head=block->next;
	head.free_num--;
	block->type=type;
	//不同的块类型，容量不一样
	if(type==1){
		block->capacity=(head.block_size-sizeof(BlockHeader<Vertex>))/sizeof(Vertex);
	}

	if(type==2){
		block->capacity=(head.block_size-sizeof(BlockHeader<Edge>))/sizeof(Edge);
	}
	if(type==3){
		block->capacity=(head.block_size-sizeof(BlockHeader<Index>))/sizeof(Index);
	}
	return block;	
}
//增加一个顶点
/*void Subgraph::add_vertex(Vertex v){
	
}*/
//得到一个块，该块号要存在，如果块缓存在cache中，则直接返回，如果没有缓存，则从文件中读取到cache，有必要的时候要移除一个cache
/*void* Subgraph::get_block(b_type number){
	c_it block=cache.find(number);
	Node node;
	if(block!=cache.end()){
		//如果该块在缓存中，直接返回该块的指针，返回之前要调整时间链表
		if(block->second.pre==NULL) return block->second.p;
		if(block->second.next==NULL){
			(block->second.pre)->next=NULL;
			(block)
		}
		(block->second.pre)->next=block->second.next;
		
		return block->second.p;
	}
	else{
		//如果该块不在缓存中，则读入该块
		if(cache.size()<getenv("CACHESZ")){
			//如果缓存没满，则直接把块读进来，更新时间链表
			node.p=malloc(head.block_size);
			((BlockHeader<void>)node.p).clean=0;//刚进来的块是干净的
			io.seekg(get_offset(number));
			io.read((char*)(node.p),head.block_size);
			//把块加入缓存中
			cache[number]=node;
			//更新缓存的时间链表，以及链表的头尾指针
			node.next=node_head;
			node.pre=NULL;
			node_head=node;
			if(node_tail==NULL) node_tail=node;
			return node.p;
		}
		else{
			//如果缓存满了，则移除一个块
			
		}
	}
}*/

void Subgraph::test(){
	//-----------遍历每一块
	b_type p=head.free_head;
	while(p!=INVALID_BLOCK){
		block=(BlockHeader<Edge>*)malloc(head.block_size);
		io.seekg(sizeof(SubgraphHeader)+p*head.block_size);
		io.read((char*)block,head.block_size);
		block->data=(Edge*)(block+1);
		cout<<"block num:"<<block->number<<endl;
		p=block->next;
		free(block);
	}
	//----------------------------

}
