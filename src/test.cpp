#include<panda_client.hpp>

int main()
{
   Client c;
   int res=c.graph_is_in("beng");
   if(res){
       c.connect("beng");
   }
   else {
       c.create_graph("beng"); 
   }
   //创建顶点
   /*for(int i=50;i<150;i++){
       Vertex_u v(i);
       cout<<c.add_vertex(v)<<" ";
   }*/
   Vertex_u v(300);
   cout<<c.add_vertex(v)<<" ";
   cout<<c.add_vertex(v)<<" ";
     
}
