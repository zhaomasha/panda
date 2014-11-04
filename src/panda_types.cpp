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








