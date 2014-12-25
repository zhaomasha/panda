#ifndef PANDA_CLIENT
#define PANDA_CLIENT
#include "panda_head.hpp"
#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
class Client{
public:
	context_t *ctx;
	socket_t * s_con;
	string master_ip;
	string master_port;
	string slave_port;
	unordered_map<string,socket_t*>c_cons;
	Client(string ip=string(getenv("MASTER_IP")),string m_port=string(getenv("MASTER_PORT")),string s_port=string(getenv("SLAVE_PORT")));
	socket_t* find_sock(string ip);
	uint32_t create_graph(string graph_name);//创建一个图，成功返回0，不成功返回大于0
	
	
};



#endif
