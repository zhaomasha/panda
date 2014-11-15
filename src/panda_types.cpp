#include "panda_types.hpp"
Vertex::Vertex(v_type i):id(i),size(0),head(INVALID_BLOCK),tail(INVALID_BLOCK){}
v_type Vertex::getId() const
{
	return id;
}
char Vertex::getStatus() const
{
	return status;
}
uint32_t Vertex::getParam() const
{
	return param;
}
e_type Vertex::getSize() const
{
	return size;
}
b_type Vertex::getHead() const
{
	return head;
}
b_type Vertex::getTail() const
{
	return tail;
}
void Vertex::setId(v_type i)
{
	id=i;
}
void Vertex::setStatus(char s)
{
	status=s;
}
void Vertex::setParam(uint32_t p)
{
	param=p;
}
void Vertex::setSize(e_type s)
{
	size=s;
}
void Vertex::setHead(b_type h)
{
	head=h;
}
void Vertex::setTail(b_type t)
{
	tail=t;
}
/*template<typename T> void BlockHeader<T>::init_block(){
	//内容链表和空闲链表置为非法块，说明还没有内容，也没有初始化索引
	list_head=INVALID_INDEX;
	list_tail=INVALID_INDEX;
	list_free=INVALID_INDEX;
	uint32_t i;
	//初始化空闲链表，单向链表
	for(i=0;i<capacity;i++){
		data[i].next=list_free;
		list_free=i;	
	}
}*/







