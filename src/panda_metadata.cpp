#include "panda_metadata.hpp"
//初始化一个子图的元数据
void metadata::init(string name,string path){
	graph_name=name;
	ifstream fin;
	fin.open(path.c_str());
	string tmp;
	while(getline(fin,tmp)){
		meta.insert(parse_sub_ip(tmp,":"));
	}
}
void metadata::add_meta(uint32_t key,string ip){
	meta[key]=ip;
}
//找顶点所在的节点，不存再则返回空串
string metadata::find_meta(uint32_t key){
	return meta[key];
}
void metadata::print(){
	unordered_map<uint32_t,string>::iterator it=meta.begin();
	while(it!=meta.end()){
		cout<<it->first<<" "<<it->second<<endl;
		it++;
	}
}
//初始化负载
void balance::init(){
	path=string(getenv("BAL_DIR_NAME"))+"/balance.cfg";
	if(access(path.c_str(),0)!=0){
		//如果不存在负载文件，则创建一个负载文件
 		ofstream fout(path.c_str());
		fout.close();
		//初始化内存中的负载,每个节点都为负载都为0
		vector<string> v;
		parse_env("SLAVE_IP",v,":");
		for(int i=0;i<v.size();i++){
			bal.insert(pair<string,uint32_t>(v[0],0));
		}
	}else{
		//否则读取负载文件，初始化内存中的负载
		ifstream fin;
		fin.open(path.c_str());
		string tmp;
		while(getline(fin,tmp)){
			bal.insert(parse_ip_num(tmp,":"));
		}
		
	}
}
//得到最小负载的节点，遍历负载集合，找出最小负载的ip
string balance::get_min(){
	unordered_map<string,uint32_t>::iterator it=bal.begin();
	uint32_t min;
	int n=0;
	string ip;
	while(it!=bal.end()){
		if(n==0) {
			min=it->second;
			ip=it->first;
		}
		else{
			if(it->second<min) {
				min=it->second;
				ip=it->first;
			}
		}
		n++;
		it++;
	}
	return ip;
}
void balance::update(string ip,int num){
	unordered_map<string,uint32_t>::iterator it=bal.find(ip);
	if(it==bal.end()) cout<<"master: no "<<ip<<" node"<<endl;
	else{
		it->second+=num;
	}
}
void balance::print(){
	unordered_map<string,uint32_t>::iterator it=bal.begin();
	while(it!=bal.end()){
		cout<<it->first<<" "<<it->second<<endl;	
		it++;
	}
}



