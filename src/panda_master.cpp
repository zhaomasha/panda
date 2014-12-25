#include "panda_zmq.hpp"
#include "panda_zmqproc.hpp"
#include "panda_metadata.hpp"
#include "panda_util.hpp"
#include "panda_split_method.hpp"
using namespace std;
unordered_map<string,metadata*> metas;
balance* bal; 
string server_dir_name(getenv("SERVER_DIR_NAME"));
string bal_dir_name(getenv("BAL_DIR_NAME"));

//创建图是一个异步的过程，首先只会在master中创建该图的元数据，在slave中没有动作，等到再次有该图的操作时，slave才会相应的创建图
//也就是说，图存不存在是由master决定，slave接受到某个图的操作，如果该图不存在，就会创建。
void handler_create_graph(Replier &rep){
	proto_create_graph* req_arg=(proto_create_graph*)rep.get_arg();//获取请求的参数
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
//获取顶点所在ip，如果该顶点所在的子图还没有分配，则分配，slave不会有动作。整个过错也是异步式的，slave直到客户端发送请求过来时才会创建子图
void handler_get_meta(Replier &rep){
	proto_vertex* req_arg=(proto_vertex*)rep.get_arg();//获取请求的参数
	unordered_map<string,metadata*>::iterator it=metas.find(req_arg->graph_name);
	if(it==metas.end()){
		//图不存在，返回错误
		rep.ans(STATUS_NOT_EXIST,"graph has not exist",strlen("graph has not exist")+1);
	}else{
		metadata* m=it->second;//获取该图的元数据信息
		uint32_t key=get_subgraph_key(req_arg->vertex_id);
		string ip=m->find_meta(key);
		if(ip==""){
			//如果ip为空串，则为该子图分配节点，不需要知会slave
			cout<<"graph "<<req_arg->graph_name<<" has no "<<key<<" subgraph"<<endl;
			ip=bal->get_min();
			m->add_meta(key,ip);//找最小负载的ip，更新该图的元数据
			bal->update(ip,1);//同时更新负载的信息	
		}
		//返回ip给客户端
		proto_reply_vertex rep_res;
		memcpy(rep_res.ip,ip.c_str(),ip.length()+1);
		rep.ans(STATUS_OK,&rep_res,sizeof(proto_reply_vertex));
	}

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
			switch(rep.get_cmd()){
				case CMD_CREATE_GRAPH:{
					handler_create_graph(rep);	
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


void print_all_meta(){
	unordered_map<string,metadata*>::iterator it=metas.begin();
	while(it!=metas.end()){
		cout<<it->first<<":"<<endl;
		it->second->print();
		it++;
	}
}
//初始化目录，有关目录不存在则创建，存在则无动作
void init_dir(){
	if(access(server_dir_name.c_str(),0)!=0){
		string cmd=string("mkdir -p ")+server_dir_name;
		system(cmd.c_str());
		cout<<"mkdir ok"<<endl;
	}
	if(access(bal_dir_name.c_str(),0)!=0){
		string cmd=string("mkdir -p ")+bal_dir_name;
		system(cmd.c_str());
		cout<<"mkdir ok"<<endl;
	}
}
int main(){
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
	print_all_meta();
	//初始化负载
	bal=new balance();
	bal->init();
	bal->print();	

	//初始化工作环境
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
