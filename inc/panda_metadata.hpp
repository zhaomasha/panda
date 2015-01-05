#ifndef PANDA_METADATA
#define PANDA_METADATA
#include "panda_head.hpp"
#include "panda_util.hpp"
//一个图的元数据，子图和节点的对应关系
class metadata{
public:
	string graph_name;//该元数据所属的图的名字
	string path;//元数据文件名
	unordered_map<uint32_t,string> meta;//图的元数据，子图id和ip的hash表
	//初始化一个元数据文件，一个元数据文件代表一个图的元数据
	void init(string name,string path);
	//添加子图和ip项
	void add_meta(uint32_t key,string ip);
	//返回子图对应的ip
	string find_meta(uint32_t key);
	//析构函数，把新的元数据重新写入文件中去，覆盖以前旧的
	~metadata();
	void print();//打印元数据，测试用
};
//负载的类，每个节点所对应的子图数目当作负载
class balance{
public:
	string path;//负载文件的路径，在参数里面写好了
	unordered_map<string,uint32_t> bal;//ip和子图数目的hash表
	void init();//初始化负载均衡
	~balance();//把最新的负载写入到文件中，覆盖旧的内容
	string get_min();//得到负载最小的节点
	void update(string ip,int num);//更新节点的负载，num代表数目
	void print();//打印元数据，测试用
};

#endif
