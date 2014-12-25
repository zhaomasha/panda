#include "panda_client.hpp"
using namespace zmq;
int main(){
	/*context_t ctx(16);
	socket_t sock(ctx,ZMQ_REQ);
	sock.connect("tcp://127.0.0.1:5555");
	message_t omsg(sizeof(uint32_t));
	*(uint32_t*)omsg.data()=CMD_CREATE_GRAPH;
	sock.send(omsg,ZMQ_SNDMORE);
	string graph_name="graph_zms";
	message_t omsg1((void*)graph_name.c_str(),graph_name.length()+1,NULL);
	sock.send(omsg1,0);
	omsg.rebuild();
	omsg1.rebuild();
	sock.recv(omsg,0);
	sock.recv(omsg1,0);
	cout<<(char*)omsg1.data()<<endl;

	omsg.rebuild(sizeof(uint32_t));
	*(uint32_t*)omsg.data()=CMD_GET_META;
	sock.send(omsg,ZMQ_SNDMORE);
	proto_vertex pv;
	string name("graph_zms");
	memcpy(pv.graph_name,name.c_str(),name.length()+1);
	pv.vertex_id=3;
	omsg1.rebuild(&pv,sizeof(proto_vertex),NULL);
	sock.send(omsg1,0);
	omsg.rebuild();
	omsg1.rebuild();
	sock.recv(omsg,0);
	sock.recv(omsg1,0);
	cout<<((proto_reply_vertex*)omsg1.data())->ip<<endl;*/
	Client c("127.0.0.1","5555","5556");
	c.create_graph("graph");
}
