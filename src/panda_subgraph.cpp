#include "panda_subgraph.hpp"
//子图不存在时，新建一个子图文件，初始化为一定大小
void Subgraph::init(string name){
	filename=name;
	char tmp[1024*1024*atoi(getenv("INITSZ"))];
	//文件存在则清零，不存在则创建
	io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
	io.write(tmp,sizeof(tmp));
}

//格式化子图，创建子图的头，初始化索引结构等等
void Subgraph::format(){
	head.free_head=INVALID_BLOCK;
	head.vertex_head=INVALID_BLOCK;		
	head.vertex_tail=INVALID_BLOCK;		
	head.vertex_num=0;
	head.block_size=atoi(getenv("BLOCKSZ"));
	//算出文件大小，计算空闲块的大小
	io.seekg(0,fstream::end);
	uint32_t file_len=io.tellg();
	cout<<"filelen:"<<file_len<<endl;
	head.free_num=0;
	uint32_t blocks=(file_len-sizeof(SubgraphHeader))/head.block_size;
	cout<<"block size:"<<blocks<<endl;
/*
	cout<<"freenum:"<<head.free_num<<endl;
	io.seekp(0);
	io.write((char*)&head,sizeof(SubgraphHeader));
	io.seekg(0);
	io.read((char*)&head,sizeof(SubgraphHeader));			
	cout<<"new  freenum:"<<head.free_num<<endl;*/

	//把空闲的文件分成空闲块，然后串成一个空闲块链表，先用简单的链表形式做测试
	int i;
	for(i=0;i<blocks;i++){
		//内存块首地址
		block=(BlockHeader<Edge>*)malloc(head.block_size);
		block->number=i;
		block->data=(Edge*)(block+1);
		block->capacity=(head.block_size-sizeof(BlockHeader<Edge>))/sizeof(Edge);
		block->size=0;
		block->data[0].param=i;
		block->size=1;
		block->next=head.free_head;
		head.free_head=block->number;
		io.seekp(sizeof(SubgraphHeader)+i*head.block_size);
		io.write((char*)block,head.block_size);
	}
	//遍历每一块
	b_type p=head.free_head;
	while(p!=INVALID_BLOCK){
		block=(BlockHeader<Edge>*)malloc(head.block_size);
		io.seekg(sizeof(SubgraphHeader)+p*head.block_size);
		io.read((char*)block,head.block_size);
		block->data=(Edge*)(block+1);
		cout<<"block num:"<<block->number<<"block value"<<block->data[0].param<<endl;
		p=block->next;
		free(block);
	}
	  	
}
