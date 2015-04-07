#include "panda_graph.hpp"
//析构函数，释放所有的子图，使子图的脏块更新到文件中去
Graph::~Graph(){
	cout<<"graph :"<<base_dir<<" xigou"<<endl;
	unordered_map<uint32_t,Subgraph*>::iterator it;
	for(it=sgs.begin();it!=sgs.end();it++){
		delete it->second;
	} 
        Destroylock(index_lock);
        free(index_lock);
        Destroylock(vertex_num_lock);
        free(vertex_num_lock);
}

//在已有的图路径里面，初始所有的子图
void Graph::init(string dir,uint32_t blocksize){
	block_size=blocksize;
	base_dir=dir;
        vertex_num=0;//顶点数目在初始化子图之前是0
	glob_t g;
	g.gl_offs=0;
        //初始化锁
        index_lock=Getlock();
        Initlock(index_lock,NULL); 
        vertex_num_lock=Getlock();
        Initlock(vertex_num_lock,NULL);
	string sg_pattern=base_dir+"/*.dat";//dat文件是子图数据文件
	int res=glob(sg_pattern.c_str(),0,0,&g);
	if(res!=0&&res!=GLOB_NOMATCH){
		cout<<"failed to invoking glob"<<endl;
	}else{ 
		if(g.gl_pathc==0) cout<<base_dir<<" no match"<<endl;//子图目录下面没有子图文件
		else{
			//遍历子图文件，在内存中创建子图对象
			for(uint32_t i=0;i<g.gl_pathc;i++){
				Subgraph *s=new Subgraph();//创建子图
				s->recover(g.gl_pathv[i],base_dir);
				uint32_t s_key=subgraph_key(g.gl_pathv[i]);//得到子图的key
                                vertex_num+=s->head.vertex_num;
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
	return path;
}
//顶点得到该顶点的子图，由于上层的调用者是串行，所以这个函数不需要加锁
Subgraph * Graph::get_subgraph(v_type vertex_id){
	uint32_t key=get_subgraph_key(vertex_id);
	unordered_map<uint32_t,Subgraph*>::iterator it=sgs.find(key);
	if(it!=sgs.end()){
		//存在子图了直接返回指针
                Subgraph* tmp=it->second;
		return tmp;
	}else{	
		//不存在，创建子图，添加到缓存中，再返回
		Subgraph* s=new Subgraph();
		s->init(subgraph_path(key),base_dir);
		s->format(block_size);
		sgs[key]=s;
		return s;	
	}	
}
//添加边索引的操作，是串行
void Graph::add_edge_index(Key k,Value v){
        Lock(index_lock);
        edge_index.insert_kv(k,v);
        Unlock(index_lock);
}
//获取边索引的操作，是串行
void Graph::get_edge_index(Key k,list<Value>& vs){
        Lock(index_lock);
        edge_index.find_values(k,vs);
        Unlock(index_lock);
}
//增加顶点数目
void Graph::vertex_num_increment(){
        Lock(vertex_num_lock);
        vertex_num++;
        Unlock(vertex_num_lock);
}

