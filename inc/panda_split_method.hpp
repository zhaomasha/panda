#ifndef PANDA_SPLIT_METHOD
#define PANDA_SPLIT_METHOD
#include "panda_head.hpp"
uint32_t hash_method(v_type vertex_id){
	uint32_t num=atoi(getenv("HASH_NUM"));
	return vertex_id%num;
}
uint32_t get_subgraph_key(v_type vertex_id){
	return hash_method(vertex_id);
}


#endif
