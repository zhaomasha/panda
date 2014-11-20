#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	Subgraph s;
	s.init("sd");
	s.format(4096);
	//cout<<"begin: "<<s.cache.size()<<endl;
	int i;
	for(i=1;i<25;i++){	
		Vertex v(i);
		s.add_vertex(v);
	}
	//s.all_vertex();
	Vertex v(44);
	s.add_vertex(v);
	Edge e;
	/*if(ep==NULL) cout<<"no edge"<<endl;
	else{
		cout<<ep->id;
	}*/
	srand((unsigned)time(0));
	long time_pre=getTime();
	for(i=1;i<1000000;i++){
		int ff=rand();
		int del=ff%1000000+1;
		e.id=del;
		s.add_edge(44,e);
		//cout<<i<<" ";
	}
	/*for(i=1000;i>1;i--){
		int ff=rand();
		int del=ff%10000+1;
		e.id=i+2;
		s.add_edge(44,e);
	}*/
	long time_next=getTime();
	cout<<"cache size:"<<s.cache.size()<<endl;
	cout<<"times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;

	//s.index_output_edge(44);

	//s.output_edge(44);
	cout<<"delete_count_write:"<<s.delete_count<<endl;
	
	//s.output_edge(44);
	s.delete_count=0;
	time_pre=getTime();
	for(i=1;i<1000000;i++){
		Edge* ep=s.read_edge(44,i);
		/*if(ep==NULL) cout<<"noedge "<<endl;
		else{
			cout<<ep->id<<" "<<endl;
		}*/
        }
	time_next=getTime();
	cout<<"times:"<<(time_next-time_pre)/1000.0<<"  "<<"rate:"<<sizeof(Edge)*1000000/((time_next-time_pre)/1000.0)/1024/1024<<endl;
	cout<<"delete_count_read:"<<s.delete_count<<endl;
	//s.output_edge(44);
	//s.all_vertex();
	
	/*cout<<"add: "<<s.cache.size()<<endl;	
	void *tmp=s.get_block(s.require(1));
	cout<<((BlockHeader<Vertex>*)tmp)->number<<" "<<((BlockHeader<Vertex>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(2));
	cout<<((BlockHeader<Edge>*)tmp)->number<<" "<<((BlockHeader<Edge>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
        tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(2));
	cout<<((BlockHeader<Edge>*)tmp)->number<<" "<<((BlockHeader<Edge>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
        tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
        void *tmp=s.get_block(s.require(1));
	cout<<((BlockHeader<Vertex>*)tmp)->number<<" "<<((BlockHeader<Vertex>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(2));
	cout<<((BlockHeader<Edge>*)tmp)->number<<" "<<((BlockHeader<Edge>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
        tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(1));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(2));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(2));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;
	tmp=s.get_block(s.require(3));
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<" "<<s.cache.size()<<endl;*/

	//cout<<INVALID_VERTEX<<" "<<INVALID_BLOCK<<" "<<INVALID_INDEX;
}
