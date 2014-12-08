#include "panda_graph.hpp"

//在已有的图路径里面，初始所有的子图
void Graph::init(string dir){
	base_dir=dir;
	glob_t g;
	g.gl_offs=0;
	string sg_pattern=base_dir+"/*";//dat文件是子图数据文件
	int res=glob(sg_pattern.c_str(),0,0,&g);
	if(res!=0&&res!=GLOB_NOMATCH){
		cout<<"failed to invoking glob"<<endl;
	}else{ 
		if(g.gl_pathc==0) cout<<"no match"<<endl;//子图目录下面没有子图文件
		else{
			//遍历子图文件，在内存中创建子图对象
			for(uint32_t i=0;i<g.gl_pathc;i++){
				Subgraph *s=new Subgraph();//创建子图
				s->recover(g.gl_pathv[i]);
				uint32_t s_key=subgraph_key(g.gl_pathv[i]);//得到子图的key
				sgs[s_key]=s;//把子图和key加入到图的缓存中
			}
		}
	}
}
//在子图文件名中得出该子图在内存中的key
uint32_t Graph::subgraph_key(char *path){
	int begin=base_dir.length()+1;
	int len=0;
	for(int i=begin;path[i]!='.';i++)  len++;
	string key(path+begin,len);
	return atoi(key.c_str());
}
//通过key构造子图文件名
string Graph::subgraph_path(uint32_t key){
	char key_string[40];
	sprintf(key_string,"%d",key);
	string path=base_dir+"/"+key_string+".dat";
	cout<<path<<endl;
	return path;
}
//顶点得到该顶点的子图
Subgraph * Graph::get_subgraph(v_type vertex_id){
	uint32_t key=get_subgraph_key(vertex_id);
	unordered_map<uint32_t,Subgraph*>::iterator it=sgs.find(key);
	if(it!=sgs.end()){
		//存在子图了直接返回指针
		return it->second;
	}else{
		//不存在，创建子图，添加到缓存中，再返回
	}	
}


