#ifndef PANDA_TYPES
#define PANDA_TYPES
#include "panda_head.hpp"
//顶点类
class Vertex{
private:
	v_type id;//顶点的id
	char status;//顶点的状态，该顶点是否被删除等
	uint32_t param;//顶点的属性，暂时用一个作为测试
    e_type size;//边的数目
    b_type head;//顶点块链表的头指针
	b_type tail;//顶点块链表的为指针
public:
	explicit Vertex(v_type i);

	v_type getId()const;
	char getStatus()const;
	uint32_t getParam()const;
	e_type getSize()const;
	b_type getHead()const;
	b_type getTail()const;
	
	void setId(v_type i);
	void setStatus(char s);
	void setParam(uint32_t p);
	void setSize(e_type s);
	void setHead(b_type h);
	void setTail(b_type t);
};

//边的类
class Edge{
public:
	char status;//边的状态，有没有被删除
	v_type dst_id;//目标顶点id
	uint32_t param;//边的属性，写一个用来测试
	t_type timestamp;//时间戳	
};

//块的头部类
template <typename T>
class BlockHeader{
	char type;//块的类型
	b_type number;//块号
	b_type pre;//前一个块
	b_type next;//后一个块
	uint32_t capacity;//块的容量，边的个数或者顶点的个数
	uint32_t size;//已经存取的数目
	T *data;//块的数据
};



#endif


