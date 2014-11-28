#include "panda_subgraph.hpp"
#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
using namespace zmq;
Subgraph *s;
//工作线程的函数，每一个线程一个套接字
void * worker(void* args)
{
	try{
		context_t& ctx=*(context_t*)args;//获得进程的context
		socket_t sock(ctx,ZMQ_REP);//创建线程的套接字
		sock.connect("inproc://scatter");//inproc方式，一定要先bind
		int flag=1;
		while(flag){
			Replier rep(sock);
			//没有消息，会block在这
			cout<<"come"<<endl;
			rep.parse_ask();
			cout<<"ok"<<endl;
			switch(rep.get_cmd()){
				case CMD_GET_EDGE:{
					Edge *edge=s->read_edge(44,20,0);
					rep.ans(0,edge,sizeof(Edge));
					break;
				}
			}
		}
	}catch(zmq::error_t& err){
		cout<<err.what();
	}
}
int main(){
	s=new Subgraph();
	cout<<"f"<<endl;
	s->init("subgraph.dat");
	cout<<"here"<<endl;
	s->format(4096);
	Vertex v(44);
	s->add_vertex(v,0);
	Edge e;
	e.id=20;
	e.param=98;
	s->add_edge(44,e,0);

	context_t ctx(16);
	pthread_t thread_arg;
	socket_t gather_sock(ctx,ZMQ_ROUTER);
	gather_sock.bind("tcp://127.0.0.1:5555");
	socket_t scatter_sock(ctx,ZMQ_DEALER);	
	scatter_sock.bind("inproc://scatter");
	pthread_create(&thread_arg,NULL,worker,&ctx);
	cout<<"thread over"<<endl;
	proxy(gather_sock,scatter_sock,NULL);
}



