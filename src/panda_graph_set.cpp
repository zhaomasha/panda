#include "panda_graph_set.hpp"
//析构函数，释放节点所有的图对象
Graph_set::~Graph_set(){
	cout<<"all graph :"<<base_dir<<" xigou"<<endl;
	unordered_map<string,Graph*>::iterator it;
	for(it=graphs.begin();it!=graphs.end();it++){
		delete it->second;
	}
        Destroylock(gs_lock);
        free(gs_lock);  
}

//初始化节点的图数据目录，初始化每一张图，每张图的子图
void Graph_set::init(){
	base_dir=string(getenv("DIR_NAME"));
	if(access(base_dir.c_str(),0)!=0){
		//如果数据目录不存在，则新建
		string cmd=string("mkdir -p ")+base_dir;
		system(cmd.c_str());
	}
        //初始化锁
        gs_lock=Getlock(); 
        Initlock(gs_lock,NULL); 
	glob_t g;
	g.gl_offs=0;
	string graph_pattern=base_dir+"/*";
	int res=glob(graph_pattern.c_str(),0,0,&g);
	if(res!=0&&res!=GLOB_NOMATCH){
		cout<<"failed to invoking glob"<<endl;
	}else{
		if(g.gl_pathc==0) cout<<"slave graphs no match"<<endl;
		else{
			for(uint32_t i=0;i<g.gl_pathc;i++){
				//名为panda_server和panda_bal的目录，是服务器的目录，不是图的目录，所以图的名字也不可以叫这两个名
				if((strcmp(graph_key(g.gl_pathv[i]).c_str(),"panda_server")!=0)&&(strcmp(graph_key(g.gl_pathv[i]).c_str(),"panda_bal")!=0)){
					Graph *graph=new Graph();
					graph->init(g.gl_pathv[i]);
                                        string tmp(g.gl_pathv[i]);
                                        //初始化边的索引
                                        graph->edge_index.recover(tmp+"/"+getenv("EDGE_INDEX_FILENAME"));
					string graph_name=graph_key(g.gl_pathv[i]);	
					graphs.insert(pair<string,Graph*>(graph_name,graph));
				}
			}
		}
	}
}
//如果不存在图，则创建，明显，图的创建也是一个异步的过程，slave接受到某个图的操作，就会创建该图，master的元数据决定是否有该图，slave不会判断
Subgraph* Graph_set::get_subgraph(string graph_name,v_type v){
        Lock(gs_lock);
	unordered_map<string,Graph*>::iterator it=graphs.find(graph_name);
	if(it==graphs.end()){
		//如果不存在这个图，则创建，即创建该图的目录
		string graph_dir=base_dir+"/"+graph_name;
		string cmd=string("mkdir -p "+graph_dir);
		system(cmd.c_str());
		Graph *graph=new Graph();
		graph->init(graph_dir);
                //创建图的时候，创建索引
                graph->edge_index.init(graph_dir+"/"+getenv("EDGE_INDEX_FILENAME"));
                graph->edge_index.format();
		//更新内存
		graphs.insert(pair<string,Graph*>(graph_name,graph));
		//返回该图的子图
                Subgraph*tmp=graph->get_subgraph(v);
                Unlock(gs_lock);
		return tmp;
	}else{
                Subgraph*tmp=it->second->get_subgraph(v);
                Unlock(gs_lock);
		return tmp;
	}
}

//返回图指针，图不存在，则返回空，
Graph* Graph_set::get_graph(string graph_name){
        Lock(gs_lock);
	unordered_map<string,Graph*>::iterator it=graphs.find(graph_name);
        if(it==graphs.end()) {
             Unlock(gs_lock);
             return NULL;
        }
        else{
             Graph *graph=it->second;
             Unlock(gs_lock); 
             return graph;
        }	
}


