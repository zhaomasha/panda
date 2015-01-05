#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	Subgraph *s=new Subgraph();
	//Subgraph s;
	s->recover("subgraph.dat","/home/zhaomasha/panda/test");
	
	list<Edge> r;
	//s->read_edges(1205,2,r,0);
	s->read_all_edges(1205,r);
	for(list<Edge>::iterator it=r.begin();it!=r.end();it++){
		cout<<(*it).id<<" ";
	}
	cout<<endl;
	if(s->vertex_is_in(1)){
		Vertex v=s->get_vertex(2499);
		cout<<v.id<<" "<<v.param<<endl; 
	}
	delete s;
	//cout<<"delete_count_write:"<<s.delete_count<<endl;

	//s.delete_count=0;
	//time_pre=getTime();
	//s.output_edge(44,0);
	//time_next=getTime();
	//cout<<"out times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;
	//cout<<"delete_count_read:"<<s.delete_count<<endl;

	//s.output_edge(44);
	//s.delete_count=0;
	//time_pre=getTime();
	//for(i=1;i<count;i++){
		//Edge* ep=s.read_edge(44,10000,1);*/
		/*if(ep==NULL) cout<<"noedge "<<endl;
		else{
			cout<<ep->id<<" "<<endl;
		}*/
        /*}
	time_next=getTime();
	cout<<"times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;
	cout<<"delete_count_read:"<<s.delete_count<<endl;
	
		
	s.delete_count=0;
	time_pre=getTime();
	s.index_output_edge(44,0);
	time_next=getTime();
	cout<<"index out times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;
	cout<<"delete_count_read:"<<s.delete_count<<endl;


	//s.output_edge(44);
	//s.all_vertex();
	
		//cout<<INVALID_VERTEX<<" "<<INVALID_BLOCK<<" "<<INVALID_INDEX;*/
}
