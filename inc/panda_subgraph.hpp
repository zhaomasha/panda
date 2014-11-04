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
    void init(string name);
	//创建子图的头，初始化索引结构
	void format();
};




#endif
