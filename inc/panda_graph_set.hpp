#ifndef PANDA_GRAPH_SET
#define PANDA_GRAPH_SET
#include "panda_graph.hpp"
#include "panda_head.hpp"
#include "panda_util.hpp"
class Graph_set{
public:
	string base_dir;
	unordered_map<string,Graph *> graphs;
	void init();//初始化该节点的所有图
	Subgraph* get_subgraph(string graph_name,v_type v);
};




#endif
