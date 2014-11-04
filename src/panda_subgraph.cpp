#include "panda_subgraph.hpp"
//创建一个子图的文件，初始化大小为16M
    void Subgraph::init(string name){
		filename=name;
		char tmp[1024*1024*atoi(getenv("INITSZ"))];
		//文件存在则清零，不存在则创建
		io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
		io.write(tmp,sizeof(tmp));
	}
	
	//创建子图的头，初始化索引结构
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
		head.free_num=(file_len-sizeof(SubgraphHeader))/head.block_size;

		cout<<"freenum:"<<head.free_num<<endl;
		io.seekp(0);
		io.write((char*)&head,sizeof(SubgraphHeader));
		io.seekg(0);
		io.read((char*)&head,sizeof(SubgraphHeader));			
		cout<<"new  freenum:"<<head.free_num<<endl;	
	}