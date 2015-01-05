#ifndef PANDA_TYPE
#define PANDA_TYPE
#include "panda_head.hpp"
class Vertex_u{
public:
	v_type id;
	uint32_t param;//一个属性，测试用 
	Vertex_u(v_type id,uint32_t param=0){
		this->id=id;
		this->param=param;
	}
	Vertex_u(){}	
};
class Edge_u{
public:
	v_type s_id;//源顶点的id
	v_type d_id;//目标顶点的id
	uint32_t param;//一个属性，测试用
	t_type timestamp;//时间戳
	Edge_u(v_type s_id,v_type d_id,uint32_t param=0,t_type timestamp=time(NULL)){
		this->s_id=s_id;
		this->d_id=d_id;
		this->param=param;
		this->timestamp=timestamp;
	}
	Edge_u(){}
};


#endif
