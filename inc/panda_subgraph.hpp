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
	b_type block_num;//块的总数目
    	uint32_t block_size;//该子图的block大小
}__attribute__((packed));

//子图类
class Subgraph{
        string filename;//子图对应的文件名
	fstream io;
	SubgraphHeader head;
	BlockHeader<Edge> *block;//测试字段
        //子图在内存中缓存的结构，该子图所有的块一起管理
        //unordered_map<b_type,Node*> cache;
        c_type cache;
	Node *node_head,*node_tail;//指向内存中块链表的头部和尾部，块链表是双向链表，表明块被访问的时间顺序
	
public:
	//创建一个子图的文件，初始化大小为16M
        void init(string filename);
	//创建子图的头，初始化索引结构
	void format(uint32_t blocksize=atoi(getenv("BLOCKSZ")));
	f_type get_offset(b_type num);
	void update_index();
	void add_file(uint32_t size=atoi((getenv("INCREASZ"))));
	void test();
	void* requireRaw(uint32_t type);
	void* require(uint32_t type);
};




#endif
