#include "panda_graph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	Graph g;
	g.init(getenv("DIR_NAME"));
	cout<<get_subgraph_key(194)<<endl;
	g.subgraph_path(23);
	g.subgraph_path(0);
	/*Subgraph *s=g.sgs[1];
	s->all_vertex(0);*/
}
