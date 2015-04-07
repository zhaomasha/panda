#include"panda_client.hpp"
using namespace std;
int main(int argc,char* argv[])
{
    ifstream fin,fin1;
    string graph_name(argv[1]);
    Client c;
    int res=c.graph_is_in(graph_name);
    if(res){
        c.connect("beng");
    }
    else {
        c.create_graph("beng"); 
        c.connect("beng");
    }
    string vertex_file(graph_name+".vertex");
    string edge_file(graph_name+".edge");
    //存储顶点
    fin.open(vertex_file.c_str());
    string data;
    while(getline(fin,data)){
         v_type id=atol(data.c_str());
         getline(fin,data);
         string nickname=data;
         Vertex_u v(id,nickname);
         c.add_vertex(v);   
    }
    fin.close();
    fin1.open(edge_file.c_str());
    while(getline(fin1,data)){  
         v_type s_id=atol(data.c_str()); 
         getline(fin1,data);
         v_type d_id=atol(data.c_str());
         getline(fin1,data);
         string blog_id=data; 
         getline(fin1,data);
         int type=atol(data.c_str());
         getline(fin1,data);
         t_type timestamp=atol(data.c_str());
         Edge_u edge(s_id,d_id,blog_id,type,timestamp);
         c.add_edge(edge);    
    }  
    fin1.close(); 
}
