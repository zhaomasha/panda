#include "panda_types.hpp"
#include "panda_subgraph.hpp"
int main()
{
	Subgraph s;
	s.init("sd");
	s.format();
	//cout<<"begin: "<<s.cache.size()<<endl;
	int i;
	for(i=1;i<25;i++){	
		Vertex v(i);
		s.add_vertex(v);
	}
	s.all_vertex();
	Vertex v(44);
	s.add_vertex(v);
	s.all_vertex();
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
