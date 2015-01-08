#include "panda_client.hpp"
unordered_map<string,unordered_map<uint32_t,string>*> Client::cache;
//初始化客户端，指定集群master
Client::Client(string ip,string m_port,string s_port){	
	ctx=new context_t(16);
	s_con=new socket_t(*ctx,ZMQ_REQ);
	string endpoint="tcp://"+ip+":"+m_port;
	s_con->connect(endpoint.c_str());
	master_ip=ip;
	master_port=m_port;
	slave_port=s_port;
	graph_name="";//一个客户端同时只能连接一个图，初始化客户端的时候还没有连接图，所以图的名字是空的
}
//
socket_t* Client::find_sock(string ip){
	unordered_map<string,socket_t*>::iterator it=c_cons.find(ip);
	if(it!=c_cons.end()){
		return it->second;
	}else{
		socket_t* s=new socket_t(*ctx,ZMQ_REQ);
		string endpoint="tcp://"+ip+":"+slave_port;
		s->connect(endpoint.c_str());
		c_cons.insert(pair<string,socket_t*>(ip,s));
		return s;
	}
}
//创建一个图，成功返回0，不成功返回非0，不需要和slave通信，只需要和master交互
uint32_t Client::create_graph(string graph_name){
	//首先查看缓存中的元数据是否有该图，如果有就代表该图已经存在，那么直接返回图已经存在
	if(cache_graph_is_in(graph_name)) return STATUS_EXIST;
	//如果缓存中没有该图，再向master查询，缓存中没有，不代表整个系统没有该图，因为缓存在客户端是随着访问历史增加的，刚开始缓存为空	
	Requester req(*s_con);
	//消息的长度要注意，如果是字符串，则要包括最后的空子节
	proto_graph message(graph_name);
	req.ask(CMD_CREATE_GRAPH,&message,sizeof(proto_graph));
	req.parse_ans();
	//如果该图不存在，master则会创建，所以每次达到这一步时，可以确定缓存中是没有该图的数据，而该图是存在的，可以添加该图的缓存
	cache_add_graph(graph_name);
	return req.get_status();
}
//判断图是否已经存在，存在返回true，不存在返回false
bool Client::graph_is_in(string graph_name){
	//首先查看缓存中的元数据是否有该图，如果有就代表该图已经存在，那么直接true
	if(cache_graph_is_in(graph_name)) return true;
	//缓存中不存在，则和master通信
	Requester req(*s_con);
	proto_graph message(graph_name);
	req.ask(CMD_GRAPH_IN,&message,sizeof(proto_graph));
	req.parse_ans();
	if(req.get_status()==STATUS_EXIST){
		//如果图存在，则更新缓存
		cache_add_graph(graph_name);
		return true;
	}
	if(req.get_status()==STATUS_NOT_EXIST){
		return false;
	}
}
//连接一个图，成功返回true，图不存在，则不成功返回false
bool Client::connect(string graph_name){
	if(graph_is_in(graph_name)){
		this->graph_name=graph_name;
		return true;
	}else{
		return false;
	}
}
//查询目前所连接的图
string Client::current_graph(){
	return graph_name;
}
//查询元数据，返回顶点所在的ip
string Client::get_meta(string graph_name,v_type id){
	//先在缓存中找，没找到再去master询问
	uint32_t key=get_subgraph_key(id);//得到顶点所在的子图
	string ip=cache_get_meta(graph_name,key);
	if(ip==""){
		//如果缓存没有所要的元数据，则向master询问
		Requester req(*s_con);
		proto_graph_vertex message(graph_name,id);
		req.ask(CMD_GET_META,&message,sizeof(proto_graph_vertex));		
		req.parse_ans();
		ip=((proto_ip*)req.get_data())->ip;
		//更新缓存
		cache_add_subgraph(graph_name,key,ip);	
		//cout<<graph_name<<":"<<key<<" not exist in cache"<<endl;	
	}
	return ip;
}

//增加一个顶点
uint32_t Client::add_vertex(Vertex_u &v){
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//先得到元数据信息，也就是顶点所在ip
	string ip=get_meta(graph_name,v.id);	
	//向slave添加顶点
	Requester req_slave(*find_sock(ip));
	proto_graph_vertex_u mes_slave(graph_name,v);
	req_slave.ask(CMD_ADD_VERTEX,&mes_slave,sizeof(proto_graph_vertex_u));
	req_slave.parse_ans();
	return req_slave.get_status();//返回结果
}
//批量增加顶点，属于一个图
uint32_t Client::add_vertexes(list<Vertex_u> &vertexes,uint32_t *num){
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//找所有边的元数据，由于有缓存，这个操作不会成为瓶颈。边在获取元数据的时候按照ip分类
	unordered_map<string,list<Vertex_u>*> classify;
	list<Vertex_u>::iterator it=vertexes.begin();
	unordered_map<string,list<Vertex_u>*>::iterator it_cl;
	string ip;
	while(it!=vertexes.end()){
		ip=get_meta(graph_name,(*it).id);
		it_cl=classify.find(ip);
		if(it_cl==classify.end()){
			//如果没有这个ip类，则创建，然后把边加入到该ip类
			classify.insert(pair<string,list<Vertex_u>*>(ip,new list<Vertex_u>()));
			it_cl=classify.find(ip);
			it_cl->second->push_back(*it);	
		}else{
			it_cl->second->push_back(*it);	
		}
		it++;
	}
	//元数据查完后，就开始分别把每个ip类的边发送出去
	if(num!=NULL) *num=0;
	it_cl=classify.begin();
	while(it_cl!=classify.end()){	
		Requester req_slave(*find_sock(it_cl->first));
		req_slave.ask(CMD_ADD_VERTEXES,*(it_cl->second),graph_name);
		req_slave.parse_ans();
		if(num!=NULL){
			//统计插入的边的数目
			*num+=atoi((char*)req_slave.get_data());
		}
		delete it_cl->second;//这个ip类的边插完了，则释放空间
		it_cl++;
	}
	return STATUS_OK;
}
//增加一条边，顶点不存在的时候不会自动创建顶点，添加边就会失败
uint32_t Client::add_edge(Edge_u &e){
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//如果连接图了，则首先找图和源顶点的元数据，先在缓存中找，没找到再去master询问
	string ip=get_meta(graph_name,e.s_id);
	Requester req_slave(*find_sock(ip));
	proto_edge_u mes_slave(graph_name,e);
	req_slave.ask(CMD_ADD_EDGE,&mes_slave,sizeof(proto_edge_u));
	req_slave.parse_ans();
	return req_slave.get_status();//返回结果
}
void* thread_add_edges(void *args){
	Ip_Edges* ip_edges=(Ip_Edges*)args;
	Requester req_slave(*ip_edges->sock);
	req_slave.ask(CMD_ADD_EDGES,*(ip_edges->edges),ip_edges->graph_name);
	req_slave.parse_ans();
	//统计插入的边的数目
	ip_edges->num=atoi((char*)req_slave.get_data());
}
//多线程版，批量增加边，如果num不为空，则存储实际添加的边的数目，因为有些顶点可能不存在，添加就会失败
uint32_t Client::add_edges_pthread(list<Edge_u> &edges,uint32_t *num){	
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//找所有边的元数据，由于有缓存，这个操作不会成为瓶颈。边在获取元数据的时候按照ip分类
	unordered_map<string,list<Edge_u>*> classify;
	list<Edge_u>::iterator it=edges.begin();
	unordered_map<string,list<Edge_u>*>::iterator it_cl;
	string ip;
	while(it!=edges.end()){
		ip=get_meta(graph_name,(*it).s_id);
		it_cl=classify.find(ip);
		if(it_cl==classify.end()){
			//如果没有这个ip类，则创建，然后把边加入到该ip类
			classify.insert(pair<string,list<Edge_u>*>(ip,new list<Edge_u>()));
			it_cl=classify.find(ip);
			it_cl->second->push_back(*it);	
		}else{
			it_cl->second->push_back(*it);	
		}
		it++;
	}
	//元数据查完后，就开始分别把每个ip类的边发送出去
	uint32_t size=classify.size();
	Ip_Edges* datas=new Ip_Edges[size];
	uint32_t index=0;
	pthread_t *threads=new pthread_t[size];
	it_cl=classify.begin();
	while(it_cl!=classify.end()){
		datas[index].graph_name=graph_name;
		datas[index].sock=find_sock(it_cl->first);
		datas[index].edges=it_cl->second;
		datas[index].num=0;
		pthread_create(&threads[index],NULL,thread_add_edges,&datas[index]);	
		index++;	
		it_cl++;
	}
	//等待线程的运行完成
	for(index=0;index<size;index++){
		pthread_join(threads[index],NULL);
	}
	//计算成功添加的边数
	if(num!=NULL){
		*num=0;
		for(index=0;index<size;index++){
			*num+=datas[index].num;
		}
	}
	//清理内存
	for(index=0;index<size;index++){
		delete datas[index].edges;
	}
	delete[] datas;
	delete[] threads;
	return STATUS_OK;
}
//批量增加边，如果num不为空，则存储实际添加的边的数目，因为有些顶点可能不存在，添加就会失败
uint32_t Client::add_edges(list<Edge_u> &edges,uint32_t *num){	
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//找所有边的元数据，由于有缓存，这个操作不会成为瓶颈。边在获取元数据的时候按照ip分类
	unordered_map<string,list<Edge_u>*> classify;
	list<Edge_u>::iterator it=edges.begin();
	unordered_map<string,list<Edge_u>*>::iterator it_cl;
	string ip;
	while(it!=edges.end()){
		ip=get_meta(graph_name,(*it).s_id);
		it_cl=classify.find(ip);
		if(it_cl==classify.end()){
			//如果没有这个ip类，则创建，然后把边加入到该ip类
			classify.insert(pair<string,list<Edge_u>*>(ip,new list<Edge_u>()));
			it_cl=classify.find(ip);
			it_cl->second->push_back(*it);	
		}else{
			it_cl->second->push_back(*it);	
		}
		it++;
	}
	//元数据查完后，就开始分别把每个ip类的边发送出去
	if(num!=NULL) *num=0;
	it_cl=classify.begin();
	while(it_cl!=classify.end()){	
		Requester req_slave(*find_sock(it_cl->first));
		req_slave.ask(CMD_ADD_EDGES,*(it_cl->second),graph_name);
		req_slave.parse_ans();
		if(num!=NULL){
			//统计插入的边的数目
			*num+=atoi((char*)req_slave.get_data());
		}
		delete it_cl->second;//这个ip类的边插完了，则释放空间
		it_cl++;
	}
	return STATUS_OK;
}

//返回两个顶点之间的所有边
uint32_t Client::read_edge(v_type s_id,v_type d_id,list<Edge_u>& edges){
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//如果连接图了，则首先找图和源顶点的元数据，先在缓存中找，没找到再去master询问
	string ip=get_meta(graph_name,s_id);
	Requester req_slave(*find_sock(ip));
	proto_two_vertex_u mes_slave(graph_name,s_id,d_id);
	req_slave.ask(CMD_READ_EDGE,&mes_slave,sizeof(proto_two_vertex_u));
	req_slave.parse_ans(edges);
	return req_slave.get_status();
}
//返回一个顶点的所有边
uint32_t Client::read_edges(v_type id,list<Edge_u>& edges){
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//如果连接图了，则首先找图和源顶点的元数据，先在缓存中找，没找到再去master询问
	string ip=get_meta(graph_name,id);
	Requester req_slave(*find_sock(ip));
	proto_graph_vertex mes_slave(graph_name,id);
	req_slave.ask(CMD_READ_EDGES,&mes_slave,sizeof(proto_graph_vertex));
	req_slave.parse_ans(edges);
	return req_slave.get_status();
}
//查询边的线程入口
void* thread_read_two_edges(void *args){
	Ip_Two_Vertex* ip_two_vertex=(Ip_Two_Vertex*)args;
	Requester req_slave(*ip_two_vertex->sock);
	req_slave.ask(CMD_READ_TWO_EDGES,*(ip_two_vertex->vertexes),ip_two_vertex->graph_name);
	req_slave.parse_ans(*(ip_two_vertex->edges));
}
//多线程批量读取边
uint32_t Client::read_two_edges_pthread(list<Two_vertex>& vertexes,list<Edge_u> **edges,uint32_t *size){	
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//找所有边的元数据，由于有缓存，这个操作不会成为瓶颈。边在获取元数据的时候按照ip分类
	unordered_map<string,list<Two_vertex>*> classify;
	list<Two_vertex>::iterator it=vertexes.begin();
	unordered_map<string,list<Two_vertex>*>::iterator it_cl;
	string ip;
	while(it!=vertexes.end()){
		ip=get_meta(graph_name,(*it).s_id);
		it_cl=classify.find(ip);
		if(it_cl==classify.end()){
			//如果没有这个ip类，则创建，然后把边加入到该ip类
			classify.insert(pair<string,list<Two_vertex>*>(ip,new list<Two_vertex>()));
			it_cl=classify.find(ip);
			it_cl->second->push_back(*it);	
		}else{
			it_cl->second->push_back(*it);	
		}
		it++;
	}
	//元数据查完后，就开始分别把每个ip类的顶点对发送出去，多线程的发送，所以要给每个线程数据
	*size=classify.size();
	*edges=new list<Edge_u>[*size];
	Ip_Two_Vertex* datas=new Ip_Two_Vertex[*size];
	uint32_t index=0;
	pthread_t *threads=new pthread_t[*size];
	it_cl=classify.begin();
	while(it_cl!=classify.end()){
		datas[index].graph_name=graph_name;
		datas[index].sock=find_sock(it_cl->first);
		datas[index].vertexes=it_cl->second;
		datas[index].edges=&(*edges)[index];
		pthread_create(&threads[index],NULL,thread_read_two_edges,&datas[index]);	
		index++;	
		it_cl++;
	}
	//等待线程的运行完成
	for(index=0;index<*size;index++){
		pthread_join(threads[index],NULL);
	}
	//清理内存
	for(index=0;index<*size;index++){
		delete datas[index].vertexes;
	}
	delete[] datas;
	delete[] threads;
	return STATUS_OK;
}
//批量读取边，返回所有的边，源顶点和目的顶点就没有规律了，还要在上面封装接口来读取某个源顶点和目的顶点的边，也可以由用户完成
//如果vertexes是空，则不会发送请求
uint32_t Client::read_two_edges(list<Two_vertex>& vertexes,list<Edge_u> &edges){	
	if(current_graph()=="") return STATUS_NOT_EXIST;//如果还没有连接图，则返回状态STATUS_NOT_EXIST
	//找所有边的元数据，由于有缓存，这个操作不会成为瓶颈。边在获取元数据的时候按照ip分类
	unordered_map<string,list<Two_vertex>*> classify;
	list<Two_vertex>::iterator it=vertexes.begin();
	unordered_map<string,list<Two_vertex>*>::iterator it_cl;
	string ip;
	while(it!=vertexes.end()){
		ip=get_meta(graph_name,(*it).s_id);
		it_cl=classify.find(ip);
		if(it_cl==classify.end()){
			//如果没有这个ip类，则创建，然后把边加入到该ip类
			classify.insert(pair<string,list<Two_vertex>*>(ip,new list<Two_vertex>()));
			it_cl=classify.find(ip);
			it_cl->second->push_back(*it);	
		}else{
			it_cl->second->push_back(*it);	
		}
		it++;
	}
	//元数据查完后，就开始分别把每个ip类的顶点对发送出去
	it_cl=classify.begin();
	while(it_cl!=classify.end()){	
		Requester req_slave(*find_sock(it_cl->first));
		req_slave.ask(CMD_READ_TWO_EDGES,*(it_cl->second),graph_name);
		req_slave.parse_ans(edges);
		delete it_cl->second;//这个ip类的边插完了，则释放空间
		it_cl++;
	}
	return STATUS_OK;
}
//查询缓存，返回元数据，如果不存在，则返回空串
string Client::cache_get_meta(string graph_name,uint32_t key){
	unordered_map<string,unordered_map<uint32_t,string>*>::iterator it=cache.find(graph_name);
	if(it==cache.end()) return "";//如果图不存在，则返回空串
	unordered_map<uint32_t,string>::iterator it_in=it->second->find(key);
	if(it_in==it->second->end()) return "";//如果子图不存在，则返回空串
	return it_in->second;//图和子图都存在，则返回ip
}
//更新缓存，添加一个子图
void Client::cache_add_subgraph(string graph_name,uint32_t key,string ip){
	unordered_map<string,unordered_map<uint32_t,string>*>::iterator it=cache.find(graph_name);
	if(it==cache.end()){
		//如果图不存在，则创建图
		cache_add_graph(graph_name);
		it=cache.find(graph_name);
	}
	it->second->insert(pair<uint32_t,string>(key,ip));//如果key已经在缓存了，insert操作也不会改变以前的value	
}

//查询缓存中是否有该图，如果有，则代表该图存在，因为目前暂时没有考虑删除图的操作。存在返回true，不存在返回false
bool Client::cache_graph_is_in(string graph_name){
	if(cache.find(graph_name)==cache.end()) return false;
	else return true;
}
//缓存中添加一个图
void Client::cache_add_graph(string graph_name){
	//首先保险起见，查看该图在缓存中有没有，有就直接返回
	if(cache_graph_is_in(graph_name)) return;
	unordered_map<uint32_t,string>* tmp=new unordered_map<uint32_t,string>;
	cache.insert(pair<string,unordered_map<uint32_t,string>*>(graph_name,tmp));
}
//测试函数，输出缓存
void Client::print(){
	unordered_map<string,unordered_map<uint32_t,string>*>::iterator it=cache.begin();
	while(it!=cache.end()){
		cout<<it->first<<"->";
		unordered_map<uint32_t,string>::iterator it_in=it->second->begin();
		while(it_in!=it->second->end()){
			cout<<it_in->first<<":"<<it_in->second<<"  ";
			it_in++;
		}
		cout<<endl;
		it++;
	}
}





