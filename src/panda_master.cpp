#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
#include "panda_metadata.hpp"
#include "panda_util.hpp"
#include "panda_split_method.hpp"
using namespace std;
unordered_map<string,metadata*> metas;//ip和元数据对应表
balance* bal; 
string server_dir_name(getenv("SERVER_DIR_NAME"));
string bal_dir_name(getenv("BAL_DIR_NAME"));
pthread_t thread_worker,thread_switcher;

//进程退出函数
void kill_func(int signum){	
	delete bal;//各种析构，把内存中的内容更新到文件中去
	unordered_map<string,metadata*>::iterator it=metas.begin();
	while(it!=metas.end()){
		delete it->second;
		it++;
	}	
	pthread_kill(thread_switcher,SIGQUIT);//消灭两个线程，然后主线程则退出
	pthread_kill(thread_worker,SIGQUIT);
}

//获取元数据的函数，根据图和顶点获取所在的节点，如果顶点所在的子图还没有分配节点，则根据负载来分配，虽然这时候该子图还没有数据，但是已经分配好了节点，这是个异步过程。客户端会先确保访问的图存在，所以这个函数能调用，就肯定能保证该图已经存在。为了保险起见，如果图不存在，则返回空串
string get_meta(string graph_name,v_type id){
	unordered_map<string,metadata*>::iterator it=metas.find(graph_name);
	if(it==metas.end()) return "";//如果找不到该图的元数据，说明图不存在，则返回空串
	metadata* m=it->second;
	uint32_t key=get_subgraph_key(id);//得到顶点所在的子图
	string res=m->find_meta(key);
	if(res==""){
		//如果图中该子图没有，则分配一个节点给该子图
		string ip=bal->get_min();
		bal->update(ip,1);//该节点的负载加1
		m->add_meta(key,ip);//更新该图的元数据
		return ip;
	}else{
		return res;	
	}
}


//创建图是一个异步的过程，首先只会在master中创建该图的元数据，在slave中没有动作，等到再次有该图的操作时，slave才会相应的创建图
//也就是说，图存不存在是由master决定，slave接受到某个图的操作，如果该图不存在，就会创建。
void handler_create_graph(Replier &rep){
	//元数据里面包含了系统中所有的图
	proto_graph* req_arg=(proto_graph*)rep.get_arg();//获取请求的参数
	unordered_map<string,metadata*>::iterator it=metas.find(req_arg->graph_name);
	if(it!=metas.end()){
		//图已经存在，返回错误
		rep.ans(STATUS_EXIST,"graph has exist",strlen("graph has exist")+1);
	}else{
		//图不存在，创建图文件(meta文件)，同时更新内存中的缓存，返回成功
		string path=server_dir_name+"/"+req_arg->graph_name+".meta";
		ofstream fout(path.c_str());
		fout.close();
		metadata* m=new metadata();
		m->init(req_arg->graph_name,path);
		metas.insert(pair<string,metadata*>(req_arg->graph_name,m));			  
		rep.ans(STATUS_OK,"ok",3);	
	}	
}
//图是否存在，就是查看元数据中有否该图
void handler_graph_is_in(Replier &rep){
	//元数据里面包含了系统中所有的图
	proto_graph* req_arg=(proto_graph*)rep.get_arg();//获取请求的参数
	unordered_map<string,metadata*>::iterator it=metas.find(req_arg->graph_name);
	if(it!=metas.end()){
		//图已经存在
		rep.ans(STATUS_EXIST,"graph has exist",strlen("graph has exist")+1);
	}else{
		//图不存在，不会创建，返回图不存在的标志
		rep.ans(STATUS_NOT_EXIST,"no graph",strlen("no graph")+1);
	}
}
//获取顶点所在ip，如果该顶点所在的子图还没有分配，则分配，slave不会有动作。整个过程也是异步式的，slave直到客户端发送请求过来时才会创建子图
void handler_get_meta(Replier &rep){
	proto_graph_vertex* req_arg=(proto_graph_vertex*)rep.get_arg();//获取请求的参数
	string ip=get_meta(req_arg->graph_name,req_arg->vertex_id);
	//返回ip给客户端
	proto_ip rep_res(ip);
	rep.ans(STATUS_OK,&rep_res,sizeof(proto_ip));
}

//工作线程，每一个线程一个套接字，接受客户端请求，处理，再返回结果
void* worker(void *args)
{
	try{
		context_t& ctx=*(context_t*)args;
		socket_t sock(ctx,ZMQ_REP);//创建线程的套接字
		sock.connect("inproc://scatter");//inproc方式，一定要先bind
		int flag=1;
		while(flag){
			Replier rep(sock);
			//没有消息，会block在这
			rep.parse_ask();
			cout<<rep.get_cmd()<<endl;
			switch(rep.get_cmd()){
				case CMD_CREATE_GRAPH:{
					handler_create_graph(rep);	
					break;
				}
				case CMD_GRAPH_IN:{
					handler_graph_is_in(rep);	
					break;
				}
				case CMD_GET_META:{
					handler_get_meta(rep);
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
	context_t& ctx=*(context_t*)args;
	socket_t gather_sock(ctx,ZMQ_ROUTER);
	string master_ip(getenv("MASTER_IP"));
	string master_port(getenv("MASTER_PORT"));
        string endpoint="tcp://"+master_ip+":"+master_port;
	gather_sock.bind(endpoint.c_str());
	socket_t scatter_sock(ctx,ZMQ_DEALER);	
	scatter_sock.bind("inproc://scatter");
	proxy(gather_sock,scatter_sock,NULL);
}
//测试函数，输出整个系统的元数据信息
void print_all_meta(){
	unordered_map<string,metadata*>::iterator it=metas.begin();
	while(it!=metas.end()){
		cout<<it->first<<":"<<endl;
		it->second->print();
		it++;
	}
}
//初始化目录，有关目录不存在则创建，存在则无动作，
void init_dir(){
	if(access(server_dir_name.c_str(),0)!=0){
		string cmd=string("mkdir -p ")+server_dir_name;
		system(cmd.c_str());
	}
	if(access(bal_dir_name.c_str(),0)!=0){
		string cmd=string("mkdir -p ")+bal_dir_name;
		system(cmd.c_str());
	}
}
int main(){
	//初始化master节点的几个目录，不存在则创建
	init_dir();
	//初始化元数据
        glob_t g;
	g.gl_offs=0;
	string meta_pattern=server_dir_name+"/*.meta";
	int res=glob(meta_pattern.c_str(),0,0,&g);
	if(res!=0&&res!=GLOB_NOMATCH){
		cout<<"master meta failed to invoking glob"<<endl;
	}else{
		if(g.gl_pathc==0) cout<<"master meta no match"<<endl;
		else{
			for(uint32_t i=0;i<g.gl_pathc;i++){
				metadata* m=new metadata();
				string graph_name=metapath_key(g.gl_pathv[i]);
				string path(g.gl_pathv[i]);
				m->init(graph_name,path);
				metas.insert(pair<string,metadata*>(graph_name,m));	
			}
		}
	}
	//print_all_meta();
	//初始化负载，如果负载文件不存在，则会创建，然后初始化内存负载为各个节点是0，负载文件存在，则根据文件初始化内存中的负载
	bal=new balance();
	bal->init();
	//bal->print();	
	//设置信号函数
	signal(SIGTERM,kill_func);
	signal(SIGINT,kill_func);
	//初始化工作环境
	context_t ctx(16);
	pthread_create(&thread_switcher,NULL,switcher,&ctx);
	sleep(1);//首先要bind，所以sleep一会，让switcher线程把bind执行玩，再开worker线程中的connect操作
	pthread_create(&thread_worker,NULL,worker,&ctx);
	pthread_join(thread_switcher,NULL);//主线程等待子线程结束
	pthread_join(thread_worker,NULL);
}
