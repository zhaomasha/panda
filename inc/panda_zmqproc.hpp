#ifndef NYNN_ZMQPROT_HPP_BY_SATANSON
#define NYNN_ZMQPROT_HPP_BY_SATANSON

#include<panda_zmq.hpp>
#include<panda_head.hpp>
using namespace zmq;

enum{ASK_CMD,ASK_ARG,ASK_SIZE};
enum{CMD_GET_EDGE};
enum{ANS_STATUS,ANS_DATA,ANS_SIZE};

/*static uint32_t const STATUS_OK=0;
static uint32_t const STATUS_ERR=INVALID_BLOCKNO;*/

/*class Requester{
public:
	explicit Requester(socket_t& s):sock(s){}//縮ocket縍equester靠
	void ask(uint8_t cmd,const void*options,size_t osize,const void*data,size_t dsize){
		//分段发送消息，请求消息有命令，选项，数据，版本
		zmq::message_t omsg[ASK_SIZE];
		//初始化消息为四个字节
		omsg[ASK_VERSION].rebuild(sizeof(uint32_t));
		//消息赋值
		*(uint32_t*)omsg[ASK_VERSION].data()=VERSION_NO;
		
		omsg[ASK_CMD].rebuild(sizeof(uint8_t));
		*(uint8_t*)omsg[ASK_CMD].data()=cmd;
		
		omsg[ASK_OPTIONS].rebuild(osize);
		memcpy(omsg[ASK_OPTIONS].data(),options,osize);
		
		omsg[ASK_DATA].rebuild(dsize);
		memcpy(omsg[ASK_DATA].data(),data,dsize);
		
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
			log_w("ans msg must has %d parts,but actually recv %d part(s)",ANS_SIZE,i);
			return false;
		}
		return true;
	}
	uint32_t get_status(){
		return *(uint32_t*)imsg[ANS_STATUS].data();
	}
	void* get_data(){
		if(likely(get_data_size()))return imsg[ANS_DATA].data();
		else return NULL;
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
};*/

//Replier靠靠靠靠靠�
class Replier{
public:
	explicit Replier(socket_t& s):sock(s){}
        //靠靠靠靠靠靠靠
	bool parse_ask(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ASK_SIZE);//靠靠靠靠靠靠靠
		if (i!=ASK_SIZE){
			cout<<"ask msg must has "<<ASK_SIZE<<" parts,but actually recv "<<i<<" part(s)"<<endl;
			return false;
		}
		return true;
	}

	uint8_t get_cmd(){
		return *(uint8_t*)imsg[ASK_CMD].data();
	}
        //靠靠
	void* get_arg(){
		return imsg[ASK_ARG].data();
	}
        //靠靠靠�
	size_t get_arg_size(){
		return imsg[ASK_ARG].size();
	}
	//靠靠靠靠靠靠靠靠靠靠靠靠
	void ans(uint32_t status,void* data,size_t size){
		zmq::message_t omsg[ANS_SIZE];//靠靠靠	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		omsg[ANS_DATA].rebuild(size);
		memcpy(omsg[ANS_DATA].data(),data,size);//靠靠靠靠�
                //靠靠
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
