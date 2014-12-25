#include "panda_graph_set.hpp"
void Graph_set::init(){
	base_dir=string(getenv("DIR_NAME"));
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
				Graph *graph=new Graph();
				graph->init(g.gl_pathv[i]);
				string graph_name=graph_key(g.gl_pathv[i]);	
				graphs.insert(pair<string,Graph*>(graph_name,graph));
			}
		}
	}
}

Subgraph* Graph_set::get_subgraph(string graph_name,v_type v){
	unordered_map<string,Graph*>::iterator it=graphs.find(graph_name);
	if(it==graphs.end()){
		cout<<"no "<<graph_name<<" graph"<<endl;
		return NULL;
	}else{
		return it->second->get_subgraph(v);
	}
}


