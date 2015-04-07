#ifndef PANDA_GRAPH
#define PANDA_GRAPH
#include "panda_head.hpp"
#include "panda_subgraph.hpp"
#include "panda_split_method.hpp"
#include "panda_bplus.hpp"
#include "panda_util.hpp"
class Graph{
public:
	string base_dir;//图的目录
	unordered_map<uint32_t,Subgraph*> sgs;//图的子图集，key是子图号，用hash的方式分图，key就是模
	uint32_t block_size;//该图的块大小
        uint32_t vertex_num;//该图的顶点数目，每次系统启动的时候向子图收集而来，每次添加顶点的时候更新这个变量，不需要存入文件中
        Btree<Key,Value> edge_index;//边属性的索引，一个节点所有子图共享一个索引
        lock_t *index_lock;//边索引的锁，查询图的索引是串行
        lock_t *vertex_num_lock;//顶点数目的锁
	~Graph();//析构函数，释放所有的子图
      	void init(string dir,uint32_t blocksize=atoi(getenv("BLOCKSZ")));//初始化子图，即初始化子图目录里面所有的子图文件
	//子图文件名中得到子图在内存中的key
	uint32_t subgraph_key(char* path);
	//由key（由顶点得到key）得到子图文件名
	string subgraph_path(uint32_t key);
	//由顶点号得到内存中的子图，如果该顶点所在的子图不存在，则创建，然后再返回，
	//很明显，子图的创建是一个异步的过程，首先由master分配子图在哪个节点，这时候节点并没有创建子图，而是下一次客户端向master查询元数据，拿到节点ip后，再向该节点发出请求，该节点再通过该函数创建子图	
	Subgraph* get_subgraph(v_type vertex_id);
        //添加边的索引
        void add_edge_index(Key k,Value v);
        //读取边的索引
        void get_edge_index(Key k,list<Value>&vs);
        //增加顶点数目
        void vertex_num_increment();
};


#endif
