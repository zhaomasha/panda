#ifndef PANDA_GRAPH
#define PANDA_GRAPH
#include "panda_head.hpp"
#include "panda_subgraph.hpp"
#include "panda_split_method.hpp"
class Graph{
public:
	string base_dir;//图的目录
	unordered_map<uint32_t,Subgraph*> sgs;//图的子图集
	uint32_t block_size;
      	void init(string dir,uint32_t blocksize=atoi(getenv("BLOCKSZ")));
	uint32_t subgraph_key(char* path);
	Subgraph* get_subgraph(v_type vertex_id);
	string subgraph_path(uint32_t key);	
};


#endif
