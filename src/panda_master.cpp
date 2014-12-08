#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
using namespace std;
//工作线程，每一个线程一个套接字，接受客户端请求，处理，再返回结果
void* worker(void *args)
{
	try{
		context_t& ctx=*(context_t*)args;
		socket_t sock(ctx,ZMQ_REP);//创建线程的套接字
		sock.connect("inproc://scatter");//inproc方式，一定要先bind
		int flag=1;
		while(flag){
			/*Replier rep(sock);
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
			}*/
		}
	}catch(zmq::error_t& err){
		cout<<err.what();
	}
	
}
int main(){
	context_t ctx(16);
	pthread_t thread_arg;
	socket_t gather_sock(ctx,ZMQ_ROUTER);
	string master_ip(getenv("MASTER_IP"));
	string master_port(getenv("MASTER_PORT"));
        string endpoint="tcp://"+master_ip+":"+master_port;
	gather_sock.bind(endpoint.c_str());
	socket_t scatter_sock(ctx,ZMQ_DEALER);	
	scatter_sock.bind("inproc://scatter");
	pthread_create(&thread_arg,NULL,worker,&ctx);
	proxy(gather_sock,scatter_sock,NULL);
}
