#include "panda_metadata.hpp"
metadata::init(){
	
}
void metadata::add_meta(uint32_t key,string ip){
	meta[key]=ip;
}
//不存再则返回空串
string metadata::find_meta(uint32_t key){
	return meta[key];
}
//初始化负载
void balance::init(){
	path=string(getenv("DIR_NAME"))+"/balance.cfg";
	if(access(path.c_str())!=0){
		//如果不存在负载文件，则创建一个，并初始化里面的内容
 		ofstream fout(path.c_str());
		
	}
}
