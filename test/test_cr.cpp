#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	Subgraph *s=new Subgraph();
	
	s->init("/home/zhaomasha/panda/test/zms/subgraph.dat","/home/zhaomasha/panda/test/zms");
	s->format();
	//s->recover("/home/zhaomasha/panda/test/zms/subgraph.dat","/home/zhaomasha/panda/test/zms");
	/*Vertex v(2);
	s->add_vertex(v);
	v.id=3;
	s->add_vertex(v);*/
	delete  s;
	
}
