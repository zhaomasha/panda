#ifndef PANDA_ZMQPROC
#define PANDA_ZMQPROC

#include "panda_zmq.hpp"
#include "panda_head.hpp"

using namespace zmq;

enum{ASK_CMD,ASK_ARG,ASK_SIZE};
enum{CMD_CREATE_GRAPH,CMD_GET_META,CMD_ADD_VERTEX};
enum{ANS_STATUS,ANS_DATA,ANS_SIZE};
enum{STATUS_OK,STATUS_EXIST,STATUS_NOT_EXIST};

//通信的消息体，增加顶点/获取顶点元数据的消息体，简单起见，图的名字定长
class proto_vertex{
public:
	char graph_name[20];
	v_type vertex_id;
};
class proto_reply_vertex{
public:
	char ip[20];
};
class proto_create_graph{
public:
	char graph_name[20];
};
//
/*static uint32_t const STATUS_OK=0;
static uint32_t const STATUS_ERR=INVALID_BLOCKNO;*/

class Requester{
public:
	explicit Requester(socket_t& s):sock(s){}
	void ask(uint32_t cmd,void*data,size_t d_size){
		//分段发送消息，请求消息有命令，选项，数据，版本
		zmq::message_t omsg[ASK_SIZE];
		//初始化命令类型消息为四个字节
		omsg[ASK_CMD].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ASK_CMD].data()=cmd;
		//初始化参数消息	
		omsg[ASK_ARG].rebuild(d_size);
		memcpy(omsg[ASK_ARG].data(),data,d_size);
		
		int i=0;
		while(i<ASK_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}
	//接受消息，只有状态和数据两个字段
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
	Requester(Requester const&);
	Requester& operator=(Requester const&);
};

class Replier{
public:
	explicit Replier(socket_t& s):sock(s){}
	bool parse_ask(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ASK_SIZE);//
		if (i!=ASK_SIZE){
			cout<<"ask msg must has "<<ASK_SIZE<<" parts,but actually recv "<<i<<" part(s)"<<endl;
			return false;
		}
		return true;
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
	void ans(uint32_t status,const void* data,size_t size){
		zmq::message_t omsg[ANS_SIZE];//靠靠靠	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		omsg[ANS_DATA].rebuild(size);
		memcpy(omsg[ANS_DATA].data(),data,size);//靠靠靠靠
		int i=0;
		while(i<ANS_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}
private:
	zmq::socket_t& sock;
	zmq::message_t imsg[ASK_SIZE];
	Replier(Replier const&);
	Replier& operator=(Replier const&);
};
#endif
