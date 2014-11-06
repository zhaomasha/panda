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
}__attribute__((packed));

//边的类
class Edge{
public:
	char status;//边的状态，有没有被删除
	v_type dst_id;//目标顶点id
	uint32_t param;//边的属性，写一个用来测试
	t_type timestamp;//时间戳	
}__attribute__((packed));

//顶点内部的索引类
class Index{
public:
	b_type target;
	uint32_t next;
	v_type min;
}__attribute__((packed));

//块的头部类
template <typename T>
class BlockHeader{
public:
	char type;//块的类型
	char clean;//块是否干净
	b_type number;//块号
	b_type pre;//前一个块
	b_type next;//后一个块
	uint32_t capacity;//块的容量，边的个数或者顶点的个数
	uint32_t size;//已经存取的数目
	uint32_t list_head;//块内部链表的头
	T *data;//块的数据
}__attribute__((packed));

//这是一个包装类，承载块头的指针，用来构成链表，用于淘汰块的机制，淘汰块机制采用的是最长没被访问的策略
class Node{
public:
	void *p;
	Node*pre;
	Node*next;
}__attribute__((packed));
typedef unordered_map<b_type,Node> c_type;//缓存的类型
typedef unordered_map<b_type,Node>::iterator c_it;//缓存的类型


#endif


