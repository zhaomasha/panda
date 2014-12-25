#include "panda_util.hpp"
#include "panda_graph_set.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	/*Graph g;
	g.init(getenv("DIR_NAME"));
	cout<<get_subgraph_key(194)<<endl;
	g.get_subgraph(194);
	g.subgraph_path(23);
	g.subgraph_path(0);*/
	/*vector<string> v;
	parse_env("SLAVE_IP",v,":");
	for(int i=0;i<v.size();i++){
		cout<<v[i]<<endl;
	}*/
	/*Subgraph *s=g.sgs[1];
	s->all_vertex(0);*/
	Graph_set gs;
	gs.init();
	gs.get_subgraph("graph1",1);
	gs.get_subgraph("graph2",34);
	gs.get_subgraph("graph3",20);
}
