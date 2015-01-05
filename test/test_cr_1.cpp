#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	Subgraph *s=new Subgraph();
	
	//s->init("/home/zhaomasha/panda/test/zms/subgraph.dat","/home/zhaomasha/panda/test/zms");
	//s->format();
	s->recover("/home/zhaomasha/panda/test/zms/subgraph.dat","/home/zhaomasha/panda/test/zms");
	//cout<<s->head.free_head<<" "<<s->head.free_num<<" "<<s->head.block_num<<endl;
	Vertex v=s->get_vertex(4);
	cout<<v.id<<endl;
	Vertex v1(4);
	cout<<s->add_vertex(v1)<<endl;
	delete s;
	
}
