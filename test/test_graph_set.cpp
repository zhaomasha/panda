#include "panda_graph_set.hpp"
#include "panda_subgraph.hpp"
int main(){
	Graph_set gs;
	gs.init();
	Subgraph *s=gs.get_subgraph("beng",2);	
	if(s->vertex_is_in(2)){
		cout<<"2 is in"<<endl;
	}else{
		cout<<"2 is not in,create"<<endl;
		Vertex v(2);
		s->add_vertex(v);
	}
	/*for(int i=99990;i<100010;i++){
		Vertex v=s->get_vertex(i);
		cout<<v.id<<endl;
	}*/
	Edge e;
	/*for(int i=1;i<100000;i++){
		e.id=i;
		s->add_edge(2,e);
	}*/
	list<Edge> r;
	s->read_all_edges(2,r);
	cout<<r.size()<<endl;
	list<Edge>::iterator it=r.begin();
	while(it!=r.end()){
		cout<<(*it).id<<" ";
		it++;
	}
	cout<<endl;
	s->vertex_index.scan();
	cout<<endl;
}
