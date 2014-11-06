#include "panda_types.hpp"
#include "panda_subgraph.hpp"
int main()
{
	Subgraph s;
	s.init("sd");
	s.format(131072);
        void *tmp=s.requireRaw(1);
	cout<<((BlockHeader<Vertex>*)tmp)->number<<" "<<((BlockHeader<Vertex>*)tmp)->capacity<<endl;
	tmp=s.requireRaw(2);
	cout<<((BlockHeader<Edge>*)tmp)->number<<" "<<((BlockHeader<Edge>*)tmp)->capacity<<endl;
        tmp=s.requireRaw(3);
	cout<<((BlockHeader<Index>*)tmp)->number<<" "<<((BlockHeader<Index>*)tmp)->capacity<<endl;


	//cout<<INVALID_VERTEX<<" "<<INVALID_BLOCK<<" "<<INVALID_INDEX;
}
