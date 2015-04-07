#include<panda_client.hpp>
int main()
{
   Client c;
   int res=c.graph_is_in("beng");
   if(res){
       cout<<"connect beng"<<endl;
       c.connect("beng");
   }
   else {
       cout<<"create graph 'beng' "<<endl;
       c.create_graph("beng"); 
       c.connect("beng");
   }
   //创建顶点
   /*for(int i=50;i<200;i++){
       Vertex_u v(i,"nick");
       cout<<c.add_vertex(v)<<" ";
   }
   string blog_ids[]={"123244","zmsasa","11111111111111111111111111111111111112222222222222222222"};
   for(int i=1;i<30;i++){
       Edge_u e(52,i,blog_ids[i%3],3);
       c.add_edge(e);
   }*/
   //读取边
   /*list<Edge_u> edges;
   c.read_edges(50,edges);
   cout<<"all edge num is:"<<edges.size()<<endl;
   list<Edge_u>::iterator it;
   for(it=edges.begin();it!=edges.end();it++){
      cout<<(*it).s_id<<" "<<(*it).d_id<<" "<<(*it).blog_id<<" "<<(*it).type<<" "<<ctime(&(*it).timestamp)<<endl;
   } */
   /*list<Edge_u> *edges;
   uint32_t size;
   cout<<"read->"<<c.read_edge_index_pthread(blog_ids[2],&edges,&size)<<endl;
   cout<<"size->"<<size<<endl;
   list<Edge_u>::iterator it;
   for(int i=0;i<size;i++){
      for(it=edges[i].begin();it!=edges[i].end();it++){
            cout<<(*it).s_id<<" "<<(*it).d_id<<" "<<(*it).blog_id<<" "<<(*it).type<<" "<<ctime(&(*it).timestamp)<<endl;
      }
   } */
   /*uint32_t *nums;
   uint32_t size1;
   c.get_vertex_num_pthread(&nums,&size1);
   for(int i=0;i<size1;i++) cout<<"one->"<<nums[i]<<" ";*/
   Vertex_u v;
   uint32_t num;
   c.read_vertex(12,v,&num);
   cout<<v.id<<" "<<v.nick_name<<" "<<num<<endl;
   c.read_vertex(52,v,&num);
   cout<<v.id<<" "<<v.nick_name<<" "<<num<<endl;
   cout<<c.read_vertex(49,v,&num);
}


