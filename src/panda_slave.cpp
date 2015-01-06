#include "panda_subgraph.hpp"
#include "panda_graph_set.hpp"
#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
using namespace zmq;
Graph_set *graph_set;
pthread_t thread_worker,thread_switcher;
//进程退出函数
void kill_func(int signum){
	delete graph_set;//释放空间，各种析构，把内存中的内容更新到文件中去
	pthread_kill(thread_switcher,SIGQUIT);//消灭两个线程，然后主线程则退出
	pthread_kill(thread_worker,SIGQUIT);
}
//处理添加顶点的函数，只要客户端把顶点的请求发过来了，说明该顶点就属于该slave节点了，这是一个异步的过程
void handler_add_vertex(Replier &rep){
	proto_graph_vertex_u *req_arg=(proto_graph_vertex_u*)rep.get_arg();//获取参数
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->vertex.id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
	Vertex v(req_arg->vertex);
	int res=sub->add_vertex(v);
	if(res==1){
		rep.ans(STATUS_V_EXIST,"vertex exist",strlen("vertex exist")+1);
	}
	if(res==0){
		rep.ans(STATUS_OK,"ok",strlen("ok")+1);
	} 	
}
//处理批量添加边的函数，边都属于一个图
void handler_add_vertexes(Replier &rep){
	string graph_name=rep.get_graph_name();
	list<Vertex_u> &vertexes=rep.get_vertexes();
	list<Vertex_u>::iterator it=vertexes.begin();
	Subgraph *sub;
	uint32_t num=0;
	while(it!=vertexes.end()){
		sub=graph_set->get_subgraph(graph_name,(*it).id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
		Vertex v(*it);
		int res=sub->add_vertex(v);
		if(res==0) num++;//添加成功，记录一笔
		it++;
	}
	ostringstream stream_num;
	stream_num<<num;
	string string_num=stream_num.str();
	rep.ans(STATUS_OK,string_num.c_str(),string_num.size()+1);
}
//处理添加边的函数，如果边的源顶点不存在，则返回错误的状态，如果成功插入，则返回ok状态
void handler_add_edge(Replier &rep){
	proto_edge_u *req_arg=(proto_edge_u*)rep.get_arg();//获取参数
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->edge.s_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
	Edge e(req_arg->edge);
	int res=sub->add_edge(req_arg->edge.s_id,e);
	if(res==1){
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
	}
	if(res==0){
		rep.ans(STATUS_OK,"ok",strlen("ok")+1);
	}
}
//处理批量添加边的函数，边都属于一个图
void handler_add_edges(Replier &rep){
	string graph_name=rep.get_graph_name();
	list<Edge_u> &edges=rep.get_edges();
	list<Edge_u>::iterator it=edges.begin();
	Subgraph *sub;
	uint32_t num=0;
	while(it!=edges.end()){
		sub=graph_set->get_subgraph(graph_name,(*it).s_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
		Edge e(*it);
		int res=sub->add_edge((*it).s_id,e);
		if(res==0) num++;//添加成功，记录一笔
		it++;
	}
	ostringstream stream_num;
	stream_num<<num;
	string string_num=stream_num.str();
	rep.ans(STATUS_OK,string_num.c_str(),string_num.size()+1);
}
//读取两个顶点之间的所有边，如果边的源顶点不存在，则返回错误的状态，如果成功插入，则返回ok状态
void handler_read_edge(Replier &rep){
	proto_two_vertex_u *req_arg=(proto_two_vertex_u*)rep.get_arg();
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->s_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
	list<Edge_u> edges;
	int res=sub->read_edges(req_arg->s_id,req_arg->d_id,edges);
	if(res==1){	
		cout<<"no vertex"<<endl;	
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
		cout<<"no voer"<<endl;
	}
	if(res==0){
		cout<<"ok"<<endl;
		rep.ans(STATUS_OK,edges);	
	}
}
//读取顶点的所有边，如果边的源顶点不存在，则返回错误的状态，如果成功插入，则返回ok状态
void handler_read_edges(Replier &rep){
	proto_graph_vertex *req_arg=(proto_graph_vertex*)rep.get_arg();
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->vertex_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
	list<Edge_u> edges;
	int res=sub->read_all_edges(req_arg->vertex_id,edges);
	if(res==1){		
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
	}
	if(res==0){
		rep.ans(STATUS_OK,edges);	
	}
}
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
			cout<<"waiting for request"<<endl;
			rep.parse_ask();
			cout<<"cmd "<<rep.get_cmd()<<endl;
			switch(rep.get_cmd()){
				case CMD_ADD_VERTEX:{
					handler_add_vertex(rep);			
					break;
				}
				case CMD_ADD_VERTEXES:{
					handler_add_vertexes(rep);			
					break;
				}
				case CMD_ADD_EDGE:{
					handler_add_edge(rep);			
					break;
				}
				case CMD_ADD_EDGES:{
					handler_add_edges(rep);			
					break;
				}
				case CMD_READ_EDGE:{
					handler_read_edge(rep);			
					break;
				}
				case CMD_READ_EDGES:{
					handler_read_edges(rep);			
					break;
				}
			}
		}
	}catch(zmq::error_t& err){
		cout<<err.what();
	}
}
//创建zmq通信模式的线程
void* switcher(void *args)
{
	context_t& ctx=*(context_t*)args;//获得进程的context
	socket_t gather_sock(ctx,ZMQ_ROUTER);
	string slave_ip("127.0.0.1");
	string slave_port(getenv("SLAVE_PORT"));
        string endpoint="tcp://"+slave_ip+":"+slave_port;
	gather_sock.bind(endpoint.c_str());
	socket_t scatter_sock(ctx,ZMQ_DEALER);	
	scatter_sock.bind("inproc://scatter");
	proxy(gather_sock,scatter_sock,NULL);
}
int main(){
	//首先初始化该节点的图数据
	graph_set=new Graph_set();
	graph_set->init();
	//graph_set.get_subgraph("haoba",2);
	//设置信号函数
	signal(SIGTERM,kill_func);
	signal(SIGINT,kill_func);
	//创建zmq的路由分发模式，创建工作线程
	context_t ctx(16);
	pthread_create(&thread_switcher,NULL,switcher,&ctx);
	sleep(1);
	pthread_create(&thread_worker,NULL,worker,&ctx);
	pthread_join(thread_switcher,NULL);
	pthread_join(thread_worker,NULL);
}



