#include"panda_client.hpp"
using namespace std;
int i=0;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *lock1;
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
void* thread_call(void* args)
{
   Client c;
   int res=c.graph_is_in("beng");
   if(res){
       c.connect("beng");
   }
   else {
       c.create_graph("beng"); 
   }
   /*for(int i=558;i<570;i++){
       Vertex_u v(i);
       cout<<"pthread"<<*(int*)args<<"  "<<i<<" "<<c.add_vertex(v)<<endl;
   }*/
   /*for(int i=2;i<10;i++){
      Edge_u e(4,i);
      cout<<"pthread"<<*(int*)args<<"  "<<i<<" "<<c.add_edge(e)<<endl;
   }*/
   
   for(int i=2;i<10;i++){
      list<Edge_u> edges; 
      c.read_edge(5,i,edges);
      cout<<"pthread"<<*(int*)args<<" 5  "<<i<<" "<<edges.size()<<endl;
   }

}
void* thread_call1(void* args)
{
   Client c;
   int res=c.graph_is_in("beng");
   if(res){
       c.connect("beng");
   }
   else {
       c.create_graph("beng"); 
   }
   /*for(int i=558;i<570;i++){
       Vertex_u v(i);
       cout<<"pthread"<<*(int*)args<<"  "<<i<<" "<<c.add_vertex(v)<<endl;
   }*/
   for(int i=2;i<10;i++){
      Edge_u e(14,i);
      cout<<"pthread"<<*(int*)args<<"add 14 "<<i<<" "<<c.add_edge(e)<<endl;
   }
   for(int i=2;i<10;i++){
      list<Edge_u> edges; 
      c.read_edge(14,i,edges);
      cout<<"pthread"<<*(int*)args<<" 14  "<<i<<" "<<edges.size()<<endl;
   }
}

int main()
{
    pthread_t id1,id2;
    int a1=1,a2=2;
    lock1=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock1,NULL);
    long time_pre=getTime();
    pthread_create(&id1,NULL,thread_call,&a1);
    pthread_create(&id2,NULL,thread_call1,&a2);
    pthread_join(id1,NULL);
    pthread_join(id2,NULL); 
    long time_next=getTime();
    cout<<"time is "<<time_next-time_pre<<endl;
}
