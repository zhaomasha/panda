#ifndef PANDA_GRAPH_SET
#define PANDA_GRAPH_SET
#include "panda_graph.hpp"
#include "panda_head.hpp"
#include "panda_util.hpp"
class Graph_set{
public:
	string base_dir;//整个图数据库的根目录
	unordered_map<string,Graph *> graphs;
	~Graph_set();//析构函数，释放该节点所有的图对象
	void init();//初始化该节点的所有图，以及子图，如果图数据目录不存在，则创建
	//如果图不存在，则创建图，即创建图的目录，这又是一个异步的过程，和子图是一样的
	Subgraph* get_subgraph(string graph_name,v_type v);
};




#endif
