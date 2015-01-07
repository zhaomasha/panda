#ifndef PANDA_ZMQPROC
#define PANDA_ZMQPROC
#include "panda_zmq.hpp"
#include "panda_head.hpp"
#include "panda_type.hpp"

using namespace zmq;

enum{ASK_CMD,ASK_ARG,ASK_SIZE};//请求包括两个字段，一个是命令，一个是参数，具体参数是什么，根据命令的类型决定
enum{CMD_CREATE_GRAPH,CMD_GRAPH_IN,CMD_GET_META,CMD_ADD_VERTEX,CMD_ADD_EDGE,CMD_READ_EDGE,CMD_ADD_EDGES,CMD_ADD_VERTEXES,CMD_READ_EDGES,CMD_READ_TWO_EDGES};//这是命令的类型
enum{ANS_STATUS,ANS_DATA,ANS_SIZE};//响应包括两个字段，一个是状态，一个是数据，具体数据是什么，根据请求的类型决定
enum{STATUS_OK,STATUS_EXIST,STATUS_NOT_EXIST,STATUS_V_EXIST,STATUS_V_NOT_EXIST};//这是相应的状态的类型
//下面都是通信的消息体
//图的名字和顶点的id，向客户端请求元数据的消息体
class proto_graph_vertex{
public:
	char graph_name[20];
	v_type vertex_id;
	proto_graph_vertex(string name,v_type id){
		memcpy(graph_name,name.c_str(),name.size()+1);
		vertex_id=id;
	}
};
//图的名字和顶点，向slave发送的消息体
class proto_graph_vertex_u{
public:
	char graph_name[20];
	Vertex_u vertex;
	proto_graph_vertex_u(string name,Vertex_u v){
		memcpy(graph_name,name.c_str(),name.size()+1);
		vertex=v;	
	}
	proto_graph_vertex_u(){}
	proto_graph_vertex_u& operator=(const proto_graph_vertex_u &vertex_u){
		memcpy(graph_name,vertex_u.graph_name,strlen(vertex_u.graph_name)+1);
		vertex=vertex_u.vertex;
	}
};
//图的一条边，向slave发送的消息体
class proto_edge_u{
public:
	char graph_name[20];
	Edge_u edge;
	proto_edge_u(string name,Edge_u e){
		memcpy(graph_name,name.c_str(),name.size()+1);
		edge=e;	
	}
	proto_edge_u(){}
	//if not const,can not use temp to copy
	proto_edge_u& operator=(const proto_edge_u &edge_u){
		memcpy(graph_name,edge_u.graph_name,strlen(edge_u.graph_name)+1);
		edge=edge_u.edge;
	}
};
//源顶点和目标顶点
class proto_two_vertex_u{
public:
	char graph_name[20];
	v_type s_id;
	v_type d_id;
	proto_two_vertex_u(string name,v_type s_id,v_type d_id){
		memcpy(graph_name,name.c_str(),name.size()+1);
		this->s_id=s_id;
		this->d_id=d_id;
	}
	proto_two_vertex_u(){}
};
//slave的ip，client和master暂时设计成只涉及到元数据，也就是client给master一个图和顶点，master返回该图的顶点所在的节点
class proto_ip{
public:
	char ip[20];
	proto_ip(string ip){
		memcpy(this->ip,ip.c_str(),ip.size()+1);
	}
};

//图的名字
class proto_graph{
public:
	char graph_name[20];
	proto_graph(string name){
		memcpy(graph_name,name.c_str(),name.size()+1);
	}
};

//请求的类，包括两个消息体，用来接受响应的数据
class Requester{
public:
	explicit Requester(socket_t& s,uint32_t size=200):sock(s),size(size){}
	//发送请求，包括命令和参数
	void ask(uint32_t cmd,void*data,size_t d_size){
		//分段发送消息，请求消息有命令和参数，所以有两个消息体
		zmq::message_t omsg[ASK_SIZE];
		//初始化命令消息体为四个字节
		omsg[ASK_CMD].rebuild(sizeof(uint32_t));
		//填充命令数据
		*(uint32_t*)omsg[ASK_CMD].data()=cmd;
		//初始化参数消息	
		omsg[ASK_ARG].rebuild(d_size);
		memcpy(omsg[ASK_ARG].data(),data,d_size);
		//发送消息体
		int i=0;
		while(i<ASK_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}
	//向同一ip批量请求添加同一张图的多条边
	void ask(uint32_t cmd,list<Edge_u> &edges,string graph_name){
		zmq::message_t omsg[ASK_SIZE];//两个消息体，一个传输命令，一个传输参数
		omsg[ASK_CMD].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ASK_CMD].data()=cmd;
		if(edges.size()==0){
			//如果集合没有数据，可以直接返回而不和系统通信，但这个可以由用户来优化，不需要在这里做
			sock.send(omsg[ASK_CMD],0);//如果集合没有数据，则send的参数是0，直接返回
			return;
		}
		sock.send(omsg[ASK_CMD],ZMQ_SNDMORE);//如果集合有数据，则send参数是ZMQ_SNDMORE,接着发送边
		uint32_t num=edges.size()/size;//消息的段数减1
		uint32_t num_mod=edges.size()%size;//最后一个消息段的边的数目
		if(num_mod==0) {
			num-=1;//如果模是0，说明之前的num是消息的段数，而不是段数减1
			num_mod=size;//最后一段，模为size
		}
		proto_edge_u coms[size];//要发送的内容	
		list<Edge_u>::iterator it=edges.begin();
		uint32_t i=0,j;
		while(i<num){
			//发送完整的消息段
			for(j=0;j<size;j++){
				coms[j]=proto_edge_u(graph_name,*it);//把集合中的边复制到数组的边中
				it++;	
			}
			//复制完size个后，就发送
			omsg[ASK_ARG].rebuild(size*sizeof(proto_edge_u));//发送前先重构消息
			memcpy(omsg[ASK_ARG].data(),coms,size*sizeof(proto_edge_u));//把要发送的内容复制给消息体
			sock.send(omsg[ASK_ARG],ZMQ_SNDMORE);
			i++;
		}
		//发送最后一个消息段
		for(j=0;j<num_mod;j++){
			coms[j]=proto_edge_u(graph_name,*it);//把集合中的边复制到数组的边中
			it++;	
		}
		//发送
		omsg[ASK_ARG].rebuild(num_mod*sizeof(proto_edge_u));//发送前先重构消息
		memcpy(omsg[ASK_ARG].data(),coms,num_mod*sizeof(proto_edge_u));//把要发送的内容复制给消息体
		sock.send(omsg[ASK_ARG],0);
	}
	//向同一ip批量请求添加同一张图的多个顶点
	void ask(uint32_t cmd,list<Vertex_u> &vertexes,string graph_name){
		zmq::message_t omsg[ASK_SIZE];//两个消息体，一个传输命令，一个传输参数
		omsg[ASK_CMD].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ASK_CMD].data()=cmd;
		if(vertexes.size()==0){
			//如果集合没有数据，可以直接返回而不和系统通信，但这个可以由用户来优化，不需要在这里做
			sock.send(omsg[ASK_CMD],0);//如果集合没有数据，则send的参数是0，直接返回
			return;
		}
		sock.send(omsg[ASK_CMD],ZMQ_SNDMORE);//如果集合有数据，则send参数是ZMQ_SNDMORE,接着发送边
		uint32_t num=vertexes.size()/size;//消息的段数减1
		uint32_t num_mod=vertexes.size()%size;//最后一个消息段的边的数目
		if(num_mod==0) {
			num-=1;//如果模是0，说明之前的num是消息的段数，而不是段数减1
			num_mod=size;//最后一段，模为size
		}
		proto_graph_vertex_u coms[size];//要发送的内容	
		list<Vertex_u>::iterator it=vertexes.begin();
		uint32_t i=0,j;
		while(i<num){
			//发送完整的消息段
			for(j=0;j<size;j++){
				coms[j]=proto_graph_vertex_u(graph_name,*it);//把集合中的边复制到数组的边中
				it++;	
			}
			//复制完size个后，就发送
			omsg[ASK_ARG].rebuild(size*sizeof(proto_graph_vertex_u));//发送前先重构消息
			memcpy(omsg[ASK_ARG].data(),coms,size*sizeof(proto_graph_vertex_u));//把要发送的内容复制给消息体
			sock.send(omsg[ASK_ARG],ZMQ_SNDMORE);
			i++;
		}
		//发送最后一个消息段
		for(j=0;j<num_mod;j++){
			coms[j]=proto_graph_vertex_u(graph_name,*it);//把集合中的边复制到数组的边中
			it++;	
		}
		//发送
		omsg[ASK_ARG].rebuild(num_mod*sizeof(proto_graph_vertex_u));//发送前先重构消息
		memcpy(omsg[ASK_ARG].data(),coms,num_mod*sizeof(proto_graph_vertex_u));//把要发送的内容复制给消息体
		sock.send(omsg[ASK_ARG],0);
	}
	//向同一ip批量请求读取同一张图的多条边
	void ask(uint32_t cmd,list<Two_vertex> &vertexes,string graph_name){
		zmq::message_t omsg[ASK_SIZE];//两个消息体，一个传输命令，一个传输参数
		omsg[ASK_CMD].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ASK_CMD].data()=cmd;
		if(vertexes.size()==0){
			//如果集合没有数据，可以直接返回而不和系统通信，但这个可以由用户来优化，不需要在这里做
			sock.send(omsg[ASK_CMD],0);//如果集合没有数据，则send的参数是0，直接返回
			return;
		}
		sock.send(omsg[ASK_CMD],ZMQ_SNDMORE);//如果集合有数据，则send参数是ZMQ_SNDMORE,接着发送边
		uint32_t num=vertexes.size()/size;//消息的段数减1
		uint32_t num_mod=vertexes.size()%size;//最后一个消息段的边的数目
		if(num_mod==0) {
			num-=1;//如果模是0，说明之前的num是消息的段数，而不是段数减1
			num_mod=size;//最后一段，模为size
		}
		//内容的转换，客户端的数据转换为通信中间层的数据
		proto_two_vertex_u coms[size];//要发送的内容	
		list<Two_vertex>::iterator it=vertexes.begin();
		uint32_t i=0,j;
		while(i<num){
			//发送完整的消息段
			for(j=0;j<size;j++){
				coms[j]=proto_two_vertex_u(graph_name,(*it).s_id,(*it).d_id);//把集合中的边复制到数组的边中
				it++;	
			}
			//复制完size个后，就发送
			omsg[ASK_ARG].rebuild(size*sizeof(proto_two_vertex_u));//发送前先重构消息
			memcpy(omsg[ASK_ARG].data(),coms,size*sizeof(proto_two_vertex_u));//把要发送的内容复制给消息体
			sock.send(omsg[ASK_ARG],ZMQ_SNDMORE);
			i++;
		}
		//发送最后一个消息段
		for(j=0;j<num_mod;j++){
			coms[j]=proto_two_vertex_u(graph_name,(*it).s_id,(*it).d_id);//把集合中的边复制到数组的边中
			it++;	
		}
		//发送
		omsg[ASK_ARG].rebuild(num_mod*sizeof(proto_two_vertex_u));//发送前先重构消息
		memcpy(omsg[ASK_ARG].data(),coms,num_mod*sizeof(proto_two_vertex_u));//把要发送的内容复制给消息体
		sock.send(omsg[ASK_ARG],0);
	}
	//接收消息，填充状态和数据两个字段
	bool parse_ans(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ANS_SIZE);
		if (i!=ANS_SIZE){
			cout<<"ans msg must has"<<ANS_SIZE<<" parts,but actually recv "<<i<<" part(s)"<<endl;
			return false;
		}
		return true;
	}
	//接收边的消息，不知道有几个消息段，结果存放在集合中。解析完后，要判断状态，如果状态是ok，而且返回有数据时，集合里面才有数据
	void parse_ans(list<Edge_u> &edges){
		//首先接收状态
		sock.recv(imsg[ANS_STATUS],0);
		if(get_status()!=STATUS_OK){	
			//如果状态不是ok，则只要接收说明字段
			sock.recv(imsg[ANS_DATA],0);
			return;
		}
		//如果状态ok，则继续接收包含边的段
		if(imsg[ANS_STATUS].more()==0) return;//如果后续没有消息段了，直接返回，说明没有数据了
		int i=0;
		do{
			imsg[ANS_DATA].rebuild();//用之前先把消息体重建
			sock.recv(imsg[ANS_DATA],0);//接收消息段
			//解析消息段，把消息里面的边存入到集合中
			int num=get_data_size()/sizeof(Edge_u);//计算消息段中边的数目
			int j;
			Edge_u *edge_p=(Edge_u*)get_data();
			for(j=0;j<num;j++){
				edges.push_back(edge_p[j]);
			}
			
		}while(imsg[ANS_DATA].more());
		
	}
	uint32_t get_status(){
		return *(uint32_t*)imsg[ANS_STATUS].data();
	}
	void* get_data(){
		return imsg[ANS_DATA].data();
	}
	size_t get_data_size(){
		return imsg[ANS_DATA].size();
	}

private:
	zmq::socket_t& sock;
	//返回的消息
	zmq::message_t imsg[ANS_SIZE];
	uint32_t size;//批处理时，每个消息段传送的边或者顶点的数目
	Requester(Requester const&);
	Requester& operator=(Requester const&);
};

//响应的类，有两个消息体，用来接受请求的数据
class Replier{
public:
	explicit Replier(socket_t& s,uint32_t size=200):sock(s),size(size){
	}
	//
	void parse_ask(){
		sock.recv(imsg[ASK_CMD],0);//接收命令，根据命令来接收参数
		if(get_cmd()==CMD_ADD_EDGES){
			//如果是批量添加边的命令，则要多次接受消息段
			if(imsg[ASK_CMD].more()==0) return;//如果后续没有消息段了，直接返回，说明没有数据了
			int i=0;
			do{
				imsg[ASK_ARG].rebuild();//用之前先把消息体重建
				sock.recv(imsg[ASK_ARG],0);//接收消息段
				//解析消息段，把消息里面的边存入到集合中
				int num=get_arg_size()/sizeof(proto_edge_u);//计算消息段中消息体的数目
				int j;
				proto_edge_u *protos=(proto_edge_u*)get_arg();
				for(j=0;j<num;j++){
					graph_name=protos[j].graph_name;
					edges.push_back(protos[j].edge);
				}
			}while(imsg[ASK_ARG].more());
			return;
		}
		if(get_cmd()==CMD_ADD_VERTEXES){
			//如果是批量添加顶点的命令，则要多次接受消息段
			if(imsg[ASK_CMD].more()==0) return;//如果后续没有消息段了，直接返回，说明没有数据了
			int i=0;
			do{
				imsg[ASK_ARG].rebuild();//用之前先把消息体重建
				sock.recv(imsg[ASK_ARG],0);//接收消息段
				//解析消息段，把消息里面的边存入到集合中
				int num=get_arg_size()/sizeof(proto_graph_vertex_u);//计算消息段中消息体的数目
				int j;
				proto_graph_vertex_u *protos=(proto_graph_vertex_u*)get_arg();
				for(j=0;j<num;j++){
					graph_name=protos[j].graph_name;
					vertexes.push_back(protos[j].vertex);
				}
			}while(imsg[ASK_ARG].more());
			return;
		}
		if(get_cmd()==CMD_READ_TWO_EDGES){
			//如果是批量读取边的命令，则要多次接受消息段
			if(imsg[ASK_CMD].more()==0) return;//如果后续没有消息段了，直接返回，说明没有数据了
			int i=0;
			do{
				imsg[ASK_ARG].rebuild();//用之前先把消息体重建
				sock.recv(imsg[ASK_ARG],0);//接收消息段
				//解析消息段，把消息里面的边存入到集合中
				int num=get_arg_size()/sizeof(proto_graph_vertex_u);//计算消息段中消息体的数目
				int j;
				proto_two_vertex_u *protos=(proto_two_vertex_u*)get_arg();
				for(j=0;j<num;j++){
					graph_name=protos[j].graph_name;
					Two_vertex t(protos[j].s_id,protos[j].d_id);
					two_vertexes.push_back(t);
				}
			}while(imsg[ASK_ARG].more());
			return;
		}	
		//如果不是批量处理的命令，则只需要接收一次消息段
		sock.recv(imsg[ASK_ARG],0);
		
		
	}
	uint32_t get_cmd(){
		return *(uint32_t*)imsg[ASK_CMD].data();
	}
	void* get_arg(){
		return imsg[ASK_ARG].data();
	}
	size_t get_arg_size(){
		return imsg[ASK_ARG].size();
	}
	string get_graph_name(){
		return graph_name;
	}
	//返回的是引用，一定要注意该集合对象的生命周期
	list<Edge_u>& get_edges(){
		return edges;
	}
	list<Vertex_u>& get_vertexes(){
		return vertexes;
	}
	list<Two_vertex>& get_two_vertexes(){
		return two_vertexes;
	}
	//回复客户端两个数据段，一个状态，一个说明数据
	void ans(uint32_t status,const void* data,size_t size){
		zmq::message_t omsg[ANS_SIZE];	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		omsg[ANS_DATA].rebuild(size);
		memcpy(omsg[ANS_DATA].data(),data,size);
		int i=0;
		while(i<ANS_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}
	//回复客户端边的集合，状态必然是ok
	void ans(uint32_t status,list<Edge_u> &edges){
		zmq::message_t omsg[ANS_SIZE];	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		if(edges.size()==0){
			sock.send(omsg[ANS_STATUS],0);//如果集合没有数据，则send的参数是0，直接返回
			return;
		}
		sock.send(omsg[ANS_STATUS],ZMQ_SNDMORE);//如果集合有数据，则send参数是ZMQ_SNDMORE,接着发送边
		uint32_t num=edges.size()/size;//消息的段数减1
		uint32_t num_mod=edges.size()%size;//最后一个消息段的边的数目
		if(num_mod==0) {
			num-=1;//如果模是0，说明之前的num是消息的段数，而不是段数减1
			num_mod=size;//最后一段，模为size
		}
		Edge_u coms[size];//要发送的内容	
		list<Edge_u>::iterator it=edges.begin();
		uint32_t i=0,j;
		while(i<num){
			//发送完整的消息段
			for(j=0;j<size;j++){
				coms[j]=*it;//把集合中的边复制到数组的边中
				it++;	
			}
			//复制完size个后，就发送
			omsg[ANS_DATA].rebuild(size*sizeof(Edge_u));//发送前先重构消息
			memcpy(omsg[ANS_DATA].data(),coms,size*sizeof(Edge_u));//把要发送的内容复制给消息体
			sock.send(omsg[ANS_DATA],ZMQ_SNDMORE);
			i++;
		}
		//发送最后一个消息段
		for(j=0;j<num_mod;j++){
			coms[j]=*it;//把集合中的边复制到数组的边中
			it++;	
		}
		//发送
		omsg[ANS_DATA].rebuild(num_mod*sizeof(Edge_u));//发送前先重构消息
		memcpy(omsg[ANS_DATA].data(),coms,num_mod*sizeof(Edge_u));//把要发送的内容复制给消息体
		sock.send(omsg[ANS_DATA],0);	
	}
private:
	zmq::socket_t& sock;
	zmq::message_t imsg[ASK_SIZE];
	list<Edge_u> edges;//批处理操作时，存放边的集合
	list<Vertex_u> vertexes;//批处理操作时，存放顶点的集合
	list<Two_vertex> two_vertexes;//批处理操作时，存放源顶点和目的顶点的类集合
	string graph_name;//批处理操作时，记录要操作的图
	const uint32_t size;//发送数据时，每个消息段最多容纳的边或者顶点的数目
	Replier(Replier const&);
	Replier& operator=(Replier const&);
};
#endif
