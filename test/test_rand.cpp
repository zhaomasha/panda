#include "panda_subgraph.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main()
{
	
	srand((unsigned)time(0));
	long time_pre=getTime();
	for(int i=1;i<100;i++){
		uint32_t ff=rand();
		cout<<ff<<" "<<ff%100000000<<":";
	}
	
}
