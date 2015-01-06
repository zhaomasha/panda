#include "panda_client.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main(){
	Client c;
	/*cout<<c.create_graph("beng")<<endl;
	cout<<c.graph_is_in("ppf")<<endl;
	cout<<c.graph_is_in("ppf1")<<endl;*/
	c.connect("beng");
	//cout<<"connect graph :"<<c.current_graph()<<endl;
	//Vertex_u v(2,2);
	Edge_u e(500,3);
	list<Edge_u> edges;
	uint32_t num=0;
	for(int i=0;i<1000;i++)
		//c.add_edge(e);
		edges.push_back(e);
	e.d_id=4;
	for(int i=0;i<1000;i++)
		//c.add_edge(e);
		edges.push_back(e);
	/*Vertex_u v(3);
	list<Vertex_u> vertexes;
	uint32_t num=0;
	for(int i=0;i<10000;i++){
		v.id=i+1;
		vertexes.push_back(v);
	}*/	
	long time_pre=getTime();
	c.add_edges(edges,&num);
	//c.add_vertexes(vertexes,&num);
	cout<<"success edge :"<<num<<endl;
	long time_next=getTime();
	cout<<"write time "<<(time_next-time_pre)/1000.0<<endl;
	list<Edge_u> res;
	c.read_edge(500,5,res);
	cout<<res.size()<<endl;
	list<Edge_u> res_all;
	c.read_edges(500,res_all);
	cout<<res_all.size()<<endl;
	//cout<<c.add_vertex(v)<<endl;
	/*list<Edge_u> res;
	time_pre=getTime();
	c.read_edge(2,3,res);
	cout<<res.size()<<endl;
	time_next=getTime();
	cout<<"read time "<<(time_next-time_pre)/1000.0<<endl;*/
	/*list<Edge_u>::iterator it=res.begin();
	while(it!=res.end()){
		cout<<(*it).s_id<<" "<<(*it).d_id<<" "<<(*it).param<<" "<<ctime((const time_t*)&((*it).timestamp))<<endl;
		it++;
	}*/
	/*v.id=12;
	c.add_vertex(v);
	v.id=13;
	c.add_vertex(v);
	c.connect("ppf");
	v.id=12;
	c.add_vertex(v);
	c.print();*/
	
}
