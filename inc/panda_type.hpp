#ifndef PANDA_TYPE
#define PANDA_TYPE
#include "panda_head.hpp"
#include "panda_util.hpp"
class Vertex_u{
public:
	v_type id;
        char nick_name[NICKNAME_LEN+1];
	Vertex_u(v_type id,string nick_name){
		this->id=id;
                if(nick_name.length()>NICKNAME_LEN){
                    //如果blog_id的长度超过规定值了，就截取规定值的长度
                    strcpy(this->nick_name,nick_name.substr(0,NICKNAME_LEN).c_str());
                }else{
                    strcpy(this->nick_name,nick_name.c_str());
                }
	}
	Vertex_u(){}	
};
class Edge_u{
public:
	v_type s_id;//源顶点的id
	v_type d_id;//目标顶点的id
	char blog_id[BLOGID_LEN+1];
        int type;
	t_type timestamp;//时间戳
	Edge_u(v_type s_id,v_type d_id,string blog_id,int type,t_type timestamp=time(NULL)){
		this->s_id=s_id;
		this->d_id=d_id;
		if(blog_id.length()>BLOGID_LEN){
                    //如果blog_id的长度超过规定值了，就截取规定值的长度
                    strcpy(this->blog_id,blog_id.substr(0,BLOGID_LEN).c_str());
                }else{
                    strcpy(this->blog_id,blog_id.c_str());
                }
                this->type=type;
		this->timestamp=timestamp;
	}
	Edge_u(){}
};
class Two_vertex{
public:
	v_type s_id;
	v_type d_id;
	Two_vertex(v_type s_id,v_type d_id):s_id(s_id),d_id(d_id){}
	Two_vertex(){}
};
//边属性索引的key
class Key{
public:
       char key[BLOGID_LEN+1];
       Key(const char *blog_id){
            strcpy(key,blog_id);
       }
       Key(string blog_id){
            strcpy(key,blog_id.c_str());
       }
       int operator<=(Key &k){
            if(strcmp(key,k.key)<=0) return 1;
            else return 0;
       }
       int operator<(Key &k){
            if(strcmp(key,k.key)<0) return 1;
            else return 0;
       }
       int operator==(Key &k){
            if(strcmp(key,k.key)==0) return 1;
            else return 0;
       }
       int operator>(Key &k){
            if(strcmp(key,k.key)>0) return 1;
            else return 0;
       }
       int operator>=(Key &k){
            if(strcmp(key,k.key)>=0) return 1;
            else return 0;
       }
};
//边属性索引的value
class Value{
public:
      v_type s_id;
      v_type d_id;
      Value(v_type s_id,v_type d_id):s_id(s_id),d_id(d_id){}
      Value(){} 
};
#endif
