#include "panda_client.hpp"
long getTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}
int main(){
	Client c;
	c.create_graph("beng");
	c.connect("beng");
	//添加一亿个顶点，批量添加，每次批量添加百万个
	/*Vertex_u v(3);
	list<Vertex_u> vertexes;
	uint32_t num=0;
	uint32_t num_all=0;
	uint32_t cir_flag=0;
	long time_pre=getTime();
	for(uint32_t i=0;i<10000000;i++){
		v.id=i+1;
		cir_flag++;
		vertexes.push_back(v);
		if(cir_flag==1000000){
			long time_pre_i=getTime();
			c.add_vertexes(vertexes,&num);
			cout<<"every:"<<num<<endl;
			long time_next_i=getTime();
			cout<<"every write time "<<(time_next_i-time_pre_i)/1000.0<<endl;
			num_all+=num;
			vertexes.clear();
			cir_flag=0;
		}
	}
	cout<<"all:"<<num_all<<endl;
	long time_next=getTime();
	cout<<"all write time "<<(time_next-time_pre)/1000.0<<endl;
*/
	//随机读取两个顶点之间的边
	list<Edge_u> res;
	uint32_t flag=0;
	c.read_edge(10000000,513813,res);
	cin>>flag;
	for(uint32_t i=0;i<1000000;i++){
		uint32_t random_s_id=rand()%10000000+1;
		uint32_t random_d_id=rand()%10000000+1;
		c.read_edge(random_s_id,random_d_id,res);
		flag++;
		if(flag==10000) cout<<i<<" ";
	}
	cout<<endl;
	cout<<res.size();
	//add edges
	/*uint32_t num=0;
	uint32_t num_all=0;
	uint32_t cir_flag=0;
	srand((unsigned)time(0));
	list<Edge_u> edges;
	long time_pre=getTime();
	for(uint32_t i=0;i<3000000000;i++){
		uint32_t random_s_id=rand()%100000000+1;
		uint32_t random_d_id=rand()%100000000+1;
		Edge_u e(random_s_id,random_d_id);
		cir_flag++;
		edges.push_back(e);
		if(cir_flag==1000000){
			long time_pre_i=getTime();
			c.add_edges(edges,&num);
			cout<<"every:"<<num<<endl;
			long time_next_i=getTime();
			cout<<"every write time "<<(time_next_i-time_pre_i)/1000.0<<endl;
			num_all+=num;
			edges.clear();
			cir_flag=0;
		}
	}
	for(uint32_t i=0;i<)
	cout<<"all:"<<num_all<<endl;
	long time_next=getTime();
	cout<<"all write time "<<(time_next-time_pre)/1000.0<<endl;*/
	/*uint32_t num=0;
	uint32_t num_all=0;
	uint32_t cir_flag=0;
	srand((unsigned)time(0));
	list<Edge_u> edges;
	long time_pre=getTime();
	for(uint32_t i=1;i<=10000000;i++){
		for(uint32_t j=0;j<30;j++){
			uint32_t random_d_id=rand()%10000000+1;
			Edge_u e(i,random_d_id);
			cir_flag++;
			edges.push_back(e);
			if(cir_flag==1000000){
				long time_pre_i=getTime();
				c.add_edges(edges,&num);
				cout<<"every:"<<num<<endl;
				long time_next_i=getTime();
				cout<<"every write time "<<(time_next_i-time_pre_i)/1000.0<<endl;
				num_all+=num;
				edges.clear();
				cir_flag=0;
			}
		}
	}
	cout<<"all:"<<num_all<<endl;
	long time_next=getTime();
	cout<<"all write time "<<(time_next-time_pre)/1000.0<<endl;*/
	/*cout<<c.graph_is_in("ppf")<<endl;
	cout<<c.graph_is_in("ppf1")<<endl;*/
	//cout<<"connect graph :"<<c.current_graph()<<endl;
	//Vertex_u v(2,2);
	/*Edge_u e(500,3);
	list<Edge_u> edges;
	uint32_t num=0;
	for(int i=0;i<1000;i++)
		//c.add_edge(e);
		edges.push_back(e);
	e.d_id=4;
	for(int i=0;i<1000;i++)
		//c.add_edge(e);
		edges.push_back(e);*/
	/*for(int i=0;i<10;i++){
		v.id=i+3;
		c.add_vertex(v);
	}*/	
	/*long time_pre=getTime();
	c.add_edges(edges,&num);
	//c.add_vertexes(vertexes,&num);
	cout<<"success edge :"<<num<<endl;
	long time_next=getTime();
	cout<<"write time "<<(time_next-time_pre)/1000.0<<endl;
	list<Edge_u> res;
	c.read_edge(500,5,res);
	cout<<res.size()<<endl;
	list<Edge_u> res_all;
	c.read_edges(500,res_all);
	cout<<res_all.size()<<endl;*/
	//cout<<c.add_vertex(v)<<endl;
	//读取顶点所有的边，和两个顶点之间的边
	/*list<Edge_u> res;
	c.read_edges(10000000,res);
	cout<<res.size()<<endl;
	list<Edge_u>::iterator it=res.begin();
	while(it!=res.end()){
		cout<<(*it).s_id<<" "<<(*it).d_id<<" "<<(*it).param<<" "<<ctime((const time_t*)&((*it).timestamp));
		it++;
	}
	res.clear();
	cout<<endl;
	c.read_edge(10000000,513814,res);
	it=res.begin();
	while(it!=res.end()){
		cout<<(*it).s_id<<" "<<(*it).d_id<<" "<<(*it).param<<" "<<ctime((const time_t*)&((*it).timestamp));
		it++;
	}*/
	
	/*v.id=12;
	c.add_vertex(v);
	v.id=13;
	c.add_vertex(v);
	c.connect("ppf");
	v.id=12;
	c.add_vertex(v);
	c.print();*/
	
}
