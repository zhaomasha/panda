#ifndef PANDA_ZMQPROC
#define PANDA_ZMQPROC

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
	explicit Requester(socket_t& s):sock(s){}//¿socket¿Requester¿¿
	void ask(uint8_t cmd,const void*options,size_t osize,const void*data,size_t dsize){
		//·Ö¶Î·¢ËÍÏûÏ¢£¬ÇëÇóÏûÏ¢ÓÐÃüÁî£¬Ñ¡Ïî£¬Êý¾Ý£¬°æ±¾
		zmq::message_t omsg[ASK_SIZE];
		//³õÊ¼»¯ÏûÏ¢ÎªËÄ¸ö×Ö½Ú
		omsg[ASK_VERSION].rebuild(sizeof(uint32_t));
		//ÏûÏ¢¸³Öµ
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
	//½ÓÊÜÏûÏ¢£¬Ö»ÓÐ×´Ì¬ºÍÊý¾ÝÁ½¸ö×Ö¶Î
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
	//·µ»ØµÄÏûÏ¢
	zmq::message_t imsg[ANS_SIZE];
	Requester(Requester const&);
	Requester& operator=(Requester const&);
};*/

class Replier{
public:
	explicit Replier(socket_t& s):sock(s){}
	bool parse_ask(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ASK_SIZE);//¿¿¿¿¿¿¿¿¿¿¿¿¿¿
		if (i!=ASK_SIZE){
			cout<<"ask msg must has "<<ASK_SIZE<<" parts,but actually recv "<<i<<" part(s)"<<endl;
			return false;
		}
		return true;
	}

	uint8_t get_cmd(){
		return *(uint8_t*)imsg[ASK_CMD].data();
	}
	void* get_arg(){
		return imsg[ASK_ARG].data();
	}
	size_t get_arg_size(){
		return imsg[ASK_ARG].size();
	}
	void ans(uint32_t status,void* data,size_t size){
		zmq::message_t omsg[ANS_SIZE];//¿¿¿¿¿¿	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		omsg[ANS_DATA].rebuild(size);
		memcpy(omsg[ANS_DATA].data(),data,size);//¿¿¿¿¿¿¿¿¿
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
