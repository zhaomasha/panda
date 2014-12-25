#ifndef PANDA_METADATA
#define PANDA_METADATA
#include "panda_head.hpp"
#include "panda_util.hpp"
//每个图的元数据，子图和节点得对应关系
class metadata{
public:
	string graph_name;
	unordered_map<uint32_t,string> meta;//图的元数据
	void init(string name,string path);
	void add_meta(uint32_t key,string ip);
	string find_meta(uint32_t key);
	void print();//打印元数据
};
class balance{
public:
	string path;//负载文件的路径
	unordered_map<string,uint32_t> bal;
	void init();//初始化负载均衡
	string get_min();//得到负载最小的节点
	void update(string ip,int num);//更新节点的负载，num代表数目
	void print();	
};

#endif
