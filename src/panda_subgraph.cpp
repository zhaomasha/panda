#include "panda_subgraph.hpp"
//子图不存在时，新建一个子图文件，初始化为配置文件中的大小，操作系统的栈大小要调整，否则会段错误
void Subgraph::init(string filename){
	//文件存在则清零，不存在则创建
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
	//初始化索引
	update_index();

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
	b_type blocks=(file_size-sizeof(SubgraphHeader)-head.block_size*num)/head.block_size;
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


