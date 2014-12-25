#include "panda_client.hpp"
//初始化客户端，指定集群master
Client::Client(string ip,string m_port,string s_port){
	ctx=new context_t(16);
	s_con=new socket_t(*ctx,ZMQ_REQ);
	string endpoint="tcp://"+ip+":"+m_port;
	s_con->connect(endpoint.c_str());
	master_ip=ip;
	master_port=m_port;
	slave_port=s_port;
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
//创建一个图，不需要和slave通信，只需要和master交互
uint32_t Client::create_graph(string graph_name){
	Requester req(*s_con);
	req.ask(CMD_CREATE_GRAPH,(void*)graph_name.c_str(),graph_name.length()+1);
	req.parse_ans();
	cout<<(char*)req.get_data()<<endl;//测试用的
	return req.get_status();
}


