#ifndef PANDA_CLIENT
#define PANDA_CLIENT
#include "panda_head.hpp"
#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
#include "panda_type.hpp"
#include "panda_split_method.hpp"

class Client{
public:
	//cache是客户端缓存的元数据，暂时没有考虑到删除操作，否则缓存的内容就会过时。
	//缓存是随着访问历史增加的，刚开始缓存为空，由于没有考虑到删除操作，所以缓存会和真实的元数据同步，元数据是不会变的
	static unordered_map<string,unordered_map<uint32_t,string>*> cache;
	context_t *ctx;
	socket_t * s_con;
	string master_ip;
	string master_port;
	string slave_port;
	string graph_name;
	unordered_map<string,socket_t*>c_cons;//ip和zmq套接字的hash表，对每一个slave最终都会有一个ip和套接字的条目
	//创建一个客户端，初始化连接master的套接字，可以自己指定master
	Client(string ip=string(getenv("MASTER_IP")),string m_port=string(getenv("MASTER_PORT")),string s_port=string(getenv("SLAVE_PORT")));
	//返回slave节点的套接字，如果没有则创建一个到该slave节点的套接字
	socket_t* find_sock(string ip);
	//创建一个图，成功返回0（STATUS_OK），图已经存在，则不成功返回大于0（STATUS_EXIST）
	uint32_t create_graph(string graph_name);
	//判断图是否存在
	bool graph_is_in(string graph_name);
	//连接一个图，只有连接了一个图后，才能开始操作图。只是会查询图存不存在，然后是对Cliet对象中的graph_name字段赋值，如果失败，则代表该图不存在，则字段还是原来的值，连接的还是原来的图
	bool connect(string graph_name);
	//查询目前连接的图，字段值为空，则代表还没有连接图
	string current_graph();
	//查询元数据，根据图名和顶点号，查询ip，首先会在缓存查，没有则向master查寻，最后会更新缓存。总是会对每一个子图分配一个ip的，所以总是会返回一个ip
	string get_meta(string graph_name,v_type id);
	
	//创建一个顶点，成功则返回0（STATUS_OK）,顶点已经存在返回大于0（STATUS_V_EXIST）,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t add_vertex(Vertex_u &v);
	//批量创建顶点，成功则返回0（STATUS_OK），num存储插入成功的顶点的数目,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t add_vertexes(list<Vertex_u> &vertexes,uint32_t *num=NULL);
	//增加一条边，成功则返回0（STATUS_OK）,顶点不存在返回大于0（STATUS_V_NOT_EXIST）,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t add_edge(Edge_u &e);
	//增加多条边，成功则返回0（STATUS_OK），num存储插入成功的边的数目,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t add_edges(list<Edge_u> &edges,uint32_t *num=NULL);
	//返回两个顶点之间所有的边，成功则返回0（STATUS_OK）,顶点不存在返回大于0（STATUS_V_NOT_EXIST）,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t read_edge(v_type s_id,v_type d_id,list<Edge_u> &edges);
	//返回一个顶点所有的边，成功则返回0（STATUS_OK）,顶点不存在返回大于0（STATUS_V_NOT_EXIST）,图不存在或者为空，则返回STATUS_NOT_EXIST
	uint32_t read_edges(v_type id,list<Edge_u> &edges);

	//缓存中查询元数据，不存在，则返回空串
	string cache_get_meta(string graph_name,uint32_t key);
	//缓存中添加一个子图
	void cache_add_subgraph(string graph_name,uint32_t key,string ip);
	//查询缓存中是否有该图存在，存在则返回1，不存在返回0
	bool cache_graph_is_in(string graph_name);
	//缓存中添加一个图
	void cache_add_graph(string graph_name);
	//测试，输出缓存
	void print();
};



#endif
