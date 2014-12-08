#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	//Subgraph *s=new Subgraph();
	Subgraph s;
	/*s->init("subgraph.dat");
	s->format(4096);
	int count=10000;
	int i;
	for(i=1;i<25;i++){	
		Vertex v(i);
		s->add_vertex(v,1);
	}
	s->all_vertex(1);
	delete(s);
	s=new Subgraph();*/
	s.recover("subgraph.dat");
	Vertex v(44);
	s.add_vertex(v,0);
	s.all_vertex(1);
/*
	Vertex v(44);
	s.add_vertex(v,0);
	Edge e;*/
	/*if(ep==NULL) cout<<"no edge"<<endl;
	else{
		cout<<ep->id;
	}*/
/*	srand((unsigned)time(0));
	long time_pre=getTime();
	for(i=1;i<count;i++){
		int ff=rand();
		int del=ff%1000000+1;
		e.id=i;
		s.add_edge(44,e,1);
		//cout<<i<<" ";
	}*/
	/*for(i=1000;i>1;i--){
		int ff=rand();
		int del=ff%10000+1;
		e.id=i+2;
		s.add_edge(44,e);
	}*/
/*	long time_next=getTime();
	cout<<"cache size:"<<s.cache.size()<<endl;
	cout<<"times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;

	//s.index_output_edge(44);

	//s.output_edge(44);
	cout<<"delete_count_write:"<<s.delete_count<<endl;

	s.delete_count=0;
	time_pre=getTime();
	s.output_edge(44,0);
	time_next=getTime();
	cout<<"out times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;
	cout<<"delete_count_read:"<<s.delete_count<<endl;

	//s.output_edge(44);
	s.delete_count=0;
	time_pre=getTime();
	for(i=1;i<count;i++){
		Edge* ep=s.read_edge(44,10000,1);*/
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
