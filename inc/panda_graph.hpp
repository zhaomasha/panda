#ifndef PANDA_GRAPH
#define PANDA_GRAPH
#include "panda_head.hpp"
#include "panda_subgraph.hpp"
#include "panda_split_method.hpp"
class Graph{
public:
	string base_dir;//图的目录
	unordered_map<uint32_t,Subgraph*> sgs;//图的子图集，key是子图号，用hash的方式分图，key就是模
	uint32_t block_size;//该图的块大小
	~Graph();//析构函数，释放所有的子图
      	void init(string dir,uint32_t blocksize=atoi(getenv("BLOCKSZ")));//初始化子图，即初始化子图目录里面所有的子图文件
	//子图文件名中得到子图在内存中的key
	uint32_t subgraph_key(char* path);
	//由key（由顶点得到key）得到子图文件名
	string subgraph_path(uint32_t key);
	//由顶点号得到内存中的子图，如果该顶点所在的子图不存在，则创建，然后再返回，
	//很明显，子图的创建是一个异步的过程，首先由master分配子图在哪个节点，这时候节点并没有创建子图，而是下一次客户端向master查询元数据，拿到节点ip后，再向该节点发出请求，该节点再通过该函数创建子图	
	Subgraph* get_subgraph(v_type vertex_id); 
};


#endif
