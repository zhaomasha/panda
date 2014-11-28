#ifndef PANDA_SUBGRAPH
#define PANDA_SUBGRAPH
#include "panda_head.hpp"
class Vertex;
class Edge;
//这是一个包装类，为缓冲区中的块添加链表
class Node{
public:
	void *block;
	Node *pre;
	Node *next;
}__attribute__((packed));
typedef unordered_map<b_type,Node*> c_type;//子图内存储块的hash表
typedef unordered_map<b_type,Node*>::iterator c_it;
/*typedef unordered_map<v_type,Edge*> bc_type;//块内存储边的hash表
typedef unordered_map<v_type,Edge*>::iterator bc_it;*/
//子图的头
class SubgraphHeader{
public:
	b_type free_head;//空闲块链表的头指针
	b_type vertex_head;//顶点链表的头指针
	b_type vertex_tail;//顶点链表的尾指针
	v_type vertex_num;//子图中顶点的数目
    	b_type free_num;//空闲块数目
	b_type block_num;//块的总数目
    	uint32_t block_size;//该子图的block大小
}__attribute__((packed));
//子图类
class Subgraph{
public:
        string filename;//子图对应的文件名
	fstream io;
	SubgraphHeader head;
        //子图在内存中缓存的结构，该子图所有的块一起管理
        //unordered_map<b_type,Node*> cache;
        c_type cache;
	Node* first;//内存中块链表的头
	Node* last;//内存中块链表的尾
	int delete_count;
public:
	//创建一个子图的文件，初始化大小为16M
        void init(string filename);
	//创建子图的头，初始化索引结构
	void format(uint32_t blocksize=atoi(getenv("BLOCKSZ")));
	f_type get_offset(b_type num);
	void update_index();
	void add_file(uint32_t size=atoi((getenv("INCREASZ"))));
	b_type requireRaw(uint32_t type);
	b_type require(uint32_t type);
	void* get_block(b_type number,char is_new,char is_hash);
	void add_vertex(Vertex& vertex,char is_hash);
	void all_vertex(char is_hash);
	void output_edge(v_type id,char is_hash);
	Vertex* get_vertex(v_type id,b_type*num,char is_hash);
	void add_edge(v_type id,Edge& e,char is_hash);
	b_type index_edge(Vertex* v,v_type id,b_type num,char is_hash);
	Edge* read_edge(v_type s_id,v_type d_id,char is_hash);
	void index_output_edge(v_type id,char is_hash);
};



//顶点类
class Vertex{
public:
	v_type id;//顶点的id
	char status;//顶点的状态，该顶点是否被删除等，0代表存在，1代表已经删除
	uint32_t param;//顶点的属性，暂时用一个作为测试
   	e_type size;//边的数目
    	b_type head;//顶点块链表的头指针
	b_type tail;//顶点块链表的尾指针，暂时没用到
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
	b_type target;//索引的边块
	v_type id;//下一个索引块的最小值，为了适应模板类，取名叫id，保持和Edge，Vertex的字段名相同，用来做排序的字段
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
	typedef unordered_map<v_type,T*> bc_type;//块内存储边的hash表
	typedef typename unordered_map<v_type,T*>::iterator bc_it;
	char type;//块的类型，1代表顶点，2代表边，3代表索引
	char clean;//块是否干净，0表示赶紧，1表示脏
	uint32_t fix;//块的盯住位，零的时候表示没有盯住，可以被移除出去，大于0表示被盯住了
	b_type number;//块号
	b_type pre;//前一个块
	b_type next;//后一个块
	uint32_t capacity;//块的容量，边的个数或者顶点的个数
	uint32_t size;//已经存取的数目
	uint32_t list_head;//块内部内容链表的头
	uint32_t list_tail;//块内部内容链表的尾
	uint32_t list_free;//块内部空闲链表的头
	v_type max;//内容的最大值，暂时没用到
	v_type min;//内容的最小值，主要在内部索引块中使用，记录下一个索引块的最小值
	bc_type *data_hash;	
	char is_hash;//hash位，指明该块是否建立了hash
	Content<T> *data;//块的数据
	//模板类的成员函数定义在外部，会导致链接错误
	//------测试函数，读取该块的内容，只读取内容的标识字段
	void output(){
		//cout<<"num:"<<number<<" min:"<<min<<" capacity:"<<capacity<<" size:"<<size<<endl;
		uint32_t p=list_head;
		while(p!=INVALID_INDEX){
			//cout<<data[p].content.id<<" ";
			p=data[p].next;
		}
		//cout<<endl;
	}
	void output_index(){
		uint32_t p=list_free;
		while(p!=INVALID_INDEX){
			cout<<data[p].content.id<<" ";
			p=data[p].next;
		}
		cout<<endl;
	}
	//创建块内边的hash表
	void init_hash(){
		data_hash=new bc_type();
		is_hash=1;//hash位置1，说明该块在内存中建立了hash	
		uint32_t num=list_head;
		while(num!=INVALID_INDEX){
			//遍历块内的边，构建hash，重复的num则不会添加
			if(data_hash->find(num)==data_hash->end())
				(*data_hash)[data[num].content.id]=(T*)(data+num);
			num=data[num].next;
		}
	}
	//初始化block的内部索引
	void init_block(){
		//内容链表和空闲链表置为非法块，说明还没有内容，也没有初始化索引
		list_head=INVALID_INDEX;
		list_tail=INVALID_INDEX;
		list_free=INVALID_INDEX;
		size=0;
		max=INVALID_VERTEX;
		min=INVALID_VERTEX;
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
	//块内没有空间了，什么都不会做
	void add_content(T& content){
		uint32_t free=requireRaw();
		if(free==INVALID_INDEX){
			//块内没有空间了，直接返回
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
				if(is_hash==1){
					//如果该块有hash表，则要更新hash表
					if(data_hash->find(content.id)==data_hash->end()){
						(*data_hash)[content.id]=(T*)(data+free);
					}
				}
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
		if(is_hash==1){
			//如果该块有hash表，则要更新hash表
			if(data_hash->find(content.id)==data_hash->end()){
				(*data_hash)[content.id]=(T*)(data+free);
			}
		}
		
	}
	//块分裂的函数，前提是块满了，把一个块里面的内容分成两份，一份转移到另外的块，并且同时把新块加入链表中
	//分裂的两个块，暂时把块内hash表去掉
	void split(BlockHeader<T>* block,Subgraph* subgraph){
		/*if(type==3){
			cout<<"laile"<<number<<"  "<<data[list_tail].content.id<<endl;
			output();
		}*/
		//获取最大值和最小值
		v_type h=data[list_head].content.id;
		v_type t=data[list_tail].content.id;
		
		memcpy(block->data,data,sizeof(Content<T>)*capacity);//把要分裂的块中的数据部分复制到新块中
		//memcpy(block->data,data,subgraph->head.block_size-sizeof(BlockHeader<T>));//把要分裂的块中的数据部分复制到新块中
		uint32_t p=list_head;//p是要分割的临界，首先置为链表头
		uint32_t s_size;//分割后，前面部分的大小
		if(h==t||type==3){
			//如果元素都是相等的或者分裂的是索引块，那么就分一半
			int i;
			for(i=0;i<size/2;i++){
				p=data[p].next;	
			}
			s_size=size/2;
			
		}else{
			//如果元素不都是相等，则按中间值来划分
			v_type mid=(h+t)/2;//由于有INVALID_VERTEX，运算会溢出
			s_size=0;
			while(true){
				if(data[p].content.id<=mid) {
					p=data[p].next;
					s_size++;
					
				}else{
					break;
				}
			}
		}
		//新块，把后面一部分当做内容链表
		/*if(type==3) cout<<"p:"<<data[p].content.id<<endl;*/
		block->list_head=p;
		block->list_tail=list_tail;
		(block->data[(block->data[p]).pre]).next=INVALID_INDEX;
		(block->data[p]).pre=INVALID_INDEX;
		block->list_free=list_head;
		block->size=capacity-s_size;
		block->min=min;//更新新块的下一个块的最小值	
		//旧块，把后面一部分移到空闲链表去
		data[data[p].pre].next=INVALID_INDEX;
		list_tail=data[p].pre;
		list_free=p;
		size=s_size;
		if(type==2)
			min=data[p].content.id;//更新该块的下一个块的最小值	
		if(type==3){
			min=data[data[p].pre].content.id;//如果分裂索引块，该块的下一个块的最小值是p索引的块的最小值，这个值记录在p的前一个索引项里
			//data[list_tail].content.id=INVALID_VERTEX;//该块的最后一个索引项的id要置为无效，这个bug调了十个小时
		}
		//把新块加入到链表中
		block->next=next;
		if(next!=INVALID_BLOCK){
			//如果原链表中的下一个块不是无效的，则要导入下一个块，更新其指针
			BlockHeader<T> *next_block=(BlockHeader<T>*)((subgraph->get_block)(next,0,0));
			next_block->pre=block->number;
			next_block->clean=1;
		}
		block->pre=number;
		next=block->number;
		clean=1;
		block->clean=1;
		if(is_hash==1){
			//如果原来的块有hash，则分裂之后的两个块也有hash
			delete data_hash;
			init_hash();
			delete block->data_hash;
			block->init_hash();
		}else{
			//原来的块没有hash，则还是要清楚新块，因为新块有可能是初始化过的
			delete block->data_hash;
			block->is_hash=0;
		}	
		/*if(type==3) {
			cout<<"nima:"<<number<<"  "<<data[list_tail].content.id<<"  "<<block->number<<"  "<<block->data[block->list_tail].content.id<<endl;
			output();
			output_index();
			block->output();
			block->output_index();
		}*/
	}
	//根据id返回块中包含该id的一个T指针
	T* get_content(v_type id){
		if(is_hash==1){
			//如果有hash表，则通过hash查找
			//cout<<"hash"<<endl;
			bc_it it=data_hash->find(id);
			if(it!=data_hash->end()){
				return it->second;
			}
			else{ 
				return NULL;
			}
		}
		//cout<<"no hash"<<endl;
		uint32_t num=list_head;
		while(num!=INVALID_INDEX){
			if(data[num].content.id==id) 
				return &(data[num].content);
			else{
				num=data[num].next;
			}
		}
		return NULL;
	}
}__attribute__((packed));




#endif
