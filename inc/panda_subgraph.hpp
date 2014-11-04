#ifndef PANDA_SUBGRAPH
#define PANDA_SUBGRAPH
#include "panda_head.hpp"
#include "panda_types.hpp"

//子图的头
class SubgraphHeader{
public:
	b_type free_head;//空闲块链表的头指针
	b_type vertex_head;//顶点链表的头指针
	b_type vertex_tail;//顶点链表的尾指针
    v_type vertex_num;//子图中顶点的数目
    b_type free_num;//空闲块数目
    uint32_t block_size;//该子图的block大小
}__attribute__((packed));

//子图类
class Subgraph{
    string filename;//子图对应的文件名
	fstream io;
	SubgraphHeader head;
	BlockHeader block;//只包含一个块做测试
public:
	//创建一个子图的文件，初始化大小为16M
    void init(string name){
		filename=name;
		char tmp[1024*1024*atoi(getenv("INITSZ"))];
		//文件存在则清零，不存在则创建
		io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
		io.write(tmp,sizeof(tmp));
	}
	//创建子图的头，初始化索引结构
	void format(){
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
};




#endif
