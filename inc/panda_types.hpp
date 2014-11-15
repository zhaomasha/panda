#ifndef PANDA_TYPES
#define PANDA_TYPES
#include "panda_head.hpp"
//顶点类
class Vertex{
public:
	v_type id;//顶点的id
	char status;//顶点的状态，该顶点是否被删除等，0代表存在，1代表已经删除
	uint32_t param;//顶点的属性，暂时用一个作为测试
   	e_type size;//边的数目
    	b_type head;//顶点块链表的头指针
	b_type tail;//顶点块链表的尾指针
	b_type index;//索引块链表的头指针
	explicit Vertex(v_type i);

	v_type getId()const;
	char getStatus()const;
	uint32_t getParam()const;
	e_type getSize()const;
	b_type getHead()const;
	b_type getTail()const;
	
	void setId(v_type i);
	void setStatus(char s);
	void setParam(uint32_t p);
	void setSize(e_type s);
	void setHead(b_type h);
	void setTail(b_type t);
}__attribute__((packed));

//边的类
class Edge{
public:
	char status;//边的状态，有没有被删除
	v_type id;//目标顶点id
	uint32_t param;//边的属性，写一个用来测试
	t_type timestamp;//时间戳	
}__attribute__((packed));

//顶点内部的索引类
class Index{
public:
	b_type target;
	v_type id;//这个是索引块的最小值，为了适应模板类，取名叫id，保持和Edge，Vertex的字段名相同，用来做排序的字段
}__attribute__((packed));

//一个包装类，用来给基本类型(顶点，边，索引)加上链表指针
template <typename T>
class Content{
public:
	T content;
	uint32_t next;
	uint32_t pre;
};

//块的头部类
template <typename T>
class BlockHeader{
public:
	char type;//块的类型，1代表顶点，2代表边，3代表索引
	char clean;//块是否干净，0表示赶紧，1表示脏
	b_type number;//块号
	b_type pre;//前一个块
	b_type next;//后一个块
	uint32_t capacity;//块的容量，边的个数或者顶点的个数
	uint32_t size;//已经存取的数目
	uint32_t list_head;//块内部内容链表的头
	uint32_t list_tail;//块内部内容链表的尾
	uint32_t list_free;//块内部空闲链表的头
	uint32_t max;//内容的最大值，主要用在内部索引
	uint32_t min;//内容的最小值
	Content<T> *data;//块的数据
	//模板类的成员函数定义在外部，会导致链接错误
	//------测试函数，读取该块的内容，只读取内容的标识字段
	void output(){
		uint32_t p=list_head;
		cout<<"block num:"<<number<<endl;
		while(p!=INVALID_INDEX){
			cout<<data[p].content.id<<" ";
			p=data[p].next;
		}
		cout<<endl;
	}
	//初始化block的内部索引
	void init_block(){
		//内容链表和空闲链表置为非法块，说明还没有内容，也没有初始化索引
		list_head=INVALID_INDEX;
		list_tail=INVALID_INDEX;
		list_free=INVALID_INDEX;
		size=0;
		max=INVALID_INDEX;
		min=INVALID_INDEX;
		uint32_t i;
		//初始化空闲链表，单向链表
		for(i=0;i<capacity;i++){
			data[i].next=list_free;
			list_free=i;	
		}
	}
	//块内还有空闲槽的时候，获取一个空闲槽,没空闲的时候返回无效值
	uint32_t requireRaw(){
		uint32_t res=list_free;
		if(list_free==INVALID_INDEX) return res;
		list_free=data[list_free].next;
		return res;
	} 
	//块内还有空间的时候，增加一条内容，把内容顺序地插入到内容双向链表中，都按照id字段排序，每个T类型都有一个id字段
	void add_content(T& content){
		uint32_t free=requireRaw();
		if(free==INVALID_INDEX){
			//------测试输出
			cout<<"块满了啦，傻x"<<endl;
			return;
		}
		uint32_t p=list_head;
		while(p!=INVALID_INDEX){
			int flag=0;
			if(content.id<=data[p].content.id) flag=1;
			if(flag==1){
				//如果flag等于1，则在这个块前面插入
				if(data[p].pre==INVALID_INDEX){
					//该块是第一个块，则需要改头指针
					list_head=free;
					data[free].pre=INVALID_INDEX;
					data[free].next=p;
					data[p].pre=free;	
				}else{
					//该块不是第一个块
					data[free].next=p;
					data[free].pre=data[p].pre;
					data[data[p].pre].next=free;
					data[p].pre=free;
				}
				data[free].content=content;
				size++;
				return;
			}else{
				//如果flag等于0，则继续往下遍历
				p=data[p].next;
			}
		}
		//p为无效索引，说明待插入的值是最大的，可以根据尾指针来插入
		if(list_tail==INVALID_INDEX){
			//如果尾指针是无效值
			list_head=free;
			list_tail=free;
			data[free].next=INVALID_INDEX;
			data[free].pre=INVALID_INDEX;
		}else{
			//尾指针有值
			data[free].next=INVALID_INDEX;
			data[free].pre=list_tail;
			data[list_tail].next=free;
			list_tail=free;
		}
		data[free].content=content;
		size++;
	}
}__attribute__((packed));

//这是一个包装类，承载块头的指针，以后还要里面加一个锁
class Node{
public:
	void *p;
}__attribute__((packed));
//typedef unordered_map<b_type,Node> c_type;//缓存的类型
//typedef unordered_map<b_type,Node>::iterator c_it;
typedef unordered_map<b_type,void*> c_type;//缓存的类型
typedef unordered_map<b_type,void*>::iterator c_it;



#endif


