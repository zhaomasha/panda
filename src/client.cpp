#include "panda_zmq.hpp"
#include "panda_head.hpp"
#include "panda_subgraph.hpp"
using namespace zmq;
int main(){
	context_t ctx(16);
	socket_t sock(ctx,ZMQ_REQ);
	sock.connect("tcp://127.0.0.1:5555");
	message_t omsg(sizeof(uint8_t));
	*(uint8_t*)omsg.data()=0;
	sock.send(omsg,ZMQ_SNDMORE);
	message_t omsg1(sizeof(uint8_t));
	*(uint8_t*)omsg1.data()=0;
	sock.send(omsg1,0);
	omsg.rebuild();
	omsg1.rebuild();
	sock.recv(omsg,0);
	sock.recv(omsg1,0);
	Edge *e=(Edge *)omsg1.data();
	cout<<e->param;
	
}
