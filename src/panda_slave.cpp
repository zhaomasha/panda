#include "panda_subgraph.hpp"
#include "panda_graph_set.hpp"
#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
#include "panda_type.hpp"
using namespace zmq;
Graph_set *graph_set;
pthread_t thread_worker,thread_worker1,thread_switcher;
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
                graph_set->get_graph(req_arg->graph_name)->vertex_num_increment();
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
		if(res==0){
                    graph_set->get_graph(graph_name)->vertex_num_increment();
                    num++;//添加成功，记录一笔
                }
		it++;
	}
	ostringstream stream_num;
	stream_num<<num;
	string string_num=stream_num.str();
	rep.ans(STATUS_OK,string_num.c_str(),string_num.size()+1);
}
//处理添加边的函数，如果边的源顶点不存在，则返回错误的状态，如果成功插入，则返回ok状态
//加边的同时，如果成功，要添加边的属性索引，边的属性索引是针对整个图的，而不是子图
void handler_add_edge(Replier &rep){
	proto_edge_u *req_arg=(proto_edge_u*)rep.get_arg();//获取参数
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->edge.s_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
	Edge e(req_arg->edge);
	int res=sub->add_edge(req_arg->edge.s_id,e);
	if(res==1){
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
	}
	if(res==0){
                //添加边属性的索引
                Graph *graph=graph_set->get_graph(req_arg->graph_name);
                Key k(req_arg->edge.blog_id);
                Value v(req_arg->edge.s_id,req_arg->edge.d_id);
                graph->add_edge_index(k,v); 
		rep.ans(STATUS_OK,"ok",strlen("ok")+1);
	}
}
//处理批量添加边的函数，边都属于一个图
//加边的同时，如果成功，要添加边的属性索引，边的属性索引是针对整个图的，而不是子图
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
		if(res==0) {
                      //添加边属性的索引
                      Graph *graph=graph_set->get_graph(graph_name);
                      Key k((*it).blog_id);
                      Value v((*it).s_id,(*it).d_id);
                      graph->add_edge_index(k,v); 
                      num++;//添加成功，记录一笔
                }
		it++;
	}
	ostringstream stream_num;
	stream_num<<num;
	string string_num=stream_num.str();
	rep.ans(STATUS_OK,string_num.c_str(),string_num.size()+1);
}
//处理查找该子图的某个图的顶点数目的函数
void handler_get_vertex_num(Replier &rep){
	proto_graph *req_arg=(proto_graph*)rep.get_arg();
        Graph *graph=graph_set->get_graph(req_arg->graph_name);
        uint32_t num=graph->vertex_num; 
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
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
	}
	if(res==0){
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
//读取顶点，如果顶点不存在，则返回错误的状态，如果存在，则返回ok状态和顶点的信息
void handler_read_vertex(Replier &rep){
	proto_graph_vertex *req_arg=(proto_graph_vertex*)rep.get_arg();
	Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,req_arg->vertex_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
        Vertex_u v;
        uint32_t num;
        int res=sub->read_vertex(req_arg->vertex_id,v,&num);
        proto_vertex_num mes;
        mes.vertex=v;
        mes.num=num;	
	if(res==1){		
		rep.ans(STATUS_V_NOT_EXIST,"vertex not exist",strlen("vertex not exist")+1);
	}
	if(res==0){
		rep.ans(STATUS_OK,&mes,sizeof(proto_vertex_num));	
	}
}
//处理批量read边的函数，边都属于一个图
void handler_read_two_edges(Replier &rep){
	string graph_name=rep.get_graph_name();
	list<Two_vertex> &vertexes=rep.get_two_vertexes();
	list<Two_vertex>::iterator it=vertexes.begin();
	Subgraph *sub;
	uint32_t num=0;
	list<Edge_u> edges;
	while(it!=vertexes.end()){
		sub=graph_set->get_subgraph(graph_name,(*it).s_id);//得到该图该顶点所在的子图，子图不存在，则会创建一个
		//sub->read_edges((*it).s_id,(*it).d_id,edges);
		sub->read_all_edges((*it).s_id,edges);
		it++;
	}
	rep.ans(STATUS_OK,edges);
}
//处理根据属性查找边的函数
void handler_read_edge_index(Replier &rep){
	proto_blog_id *req_arg=(proto_blog_id*)rep.get_arg();
	list<Edge_u> edges;//存储返回的结果
        Key k(req_arg->blog_id);
        Graph *graph=graph_set->get_graph(req_arg->graph_name);
        list<Value> vs;
        unordered_map<int,unordered_map<int,int>*> repeated;//这个map用来查重，重复的两个顶点就不用再找边了
        unordered_map<int,unordered_map<int,int>*>::iterator it_out;
        unordered_map<int,int>::iterator it_in;
        list<Value>::iterator it;
        graph->get_edge_index(k,vs);
        for(it=vs.begin();it!=vs.end();it++){
               v_type s_id=(*it).s_id;
               v_type d_id=(*it).d_id;
               //查看
               it_out=repeated.find(s_id);
               int flag;//0代表没有重复，1代表重复
               if(it_out==repeated.end()){
                    unordered_map<int,int>*tmp=new unordered_map<int,int>();
                    (*tmp)[d_id]=1;
                    repeated[s_id]=tmp; 
                    flag=0;
               }else{
                    unordered_map<int,int>*tmp=it_out->second;
                    if(tmp->find(d_id)==tmp->end()){
                         flag=0;
                         (*tmp)[d_id]=1;
                    }else{
                         flag=1;
                    }
               }
               if(flag==0){
                    //顶点没有重复的时候就去查找边
                    Subgraph *sub=graph_set->get_subgraph(req_arg->graph_name,s_id);
                    sub->read_edges(s_id,d_id,req_arg->blog_id,edges);    
               }
        }
        //释放内存
        for(it_out=repeated.begin();it_out!=repeated.end();it_out++)
              delete it_out->second;
	rep.ans(STATUS_OK,edges);
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
			cout<<pthread_self()<<" cmd "<<rep.get_cmd()<<endl;
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
				case CMD_READ_TWO_EDGES:{
					handler_read_two_edges(rep);			
					break;
				}
                                case CMD_READ_EDGE_INDEX:{
					handler_read_edge_index(rep);			
					break;
				}
                                case CMD_GET_ALL_VERTEX_NUM:{
					handler_get_vertex_num(rep);			
					break;
				}
                                case CMD_READ_VERTEX:{
					handler_read_vertex(rep);			
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
	string slave_ip(getenv("LOCAL_IP"));
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
	pthread_create(&thread_worker1,NULL,worker,&ctx);
	pthread_join(thread_switcher,NULL);
	pthread_join(thread_worker,NULL);
	pthread_join(thread_worker1,NULL);
}



