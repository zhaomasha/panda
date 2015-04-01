/*
*这个文件是panda系统的底层，包含顶点类，边类，块类(BlockHeader)，子图类
*一个子图是由一个子图头（Subgrapheader）和若干个块组成
*子图实现了内部索引的算法
*/

#ifndef PANDA_SUBGRAPH
#define PANDA_SUBGRAPH
#include "panda_head.hpp"
#include "panda_bplus.hpp"
#include "panda_type.hpp"
#include "panda_util.hpp"

class Vertex;
class Edge;

//子图的头
class SubgraphHeader{
public:
	b_type free_head;//空闲块链表的头指针
	b_type vertex_head;//顶点链表的头指针，顶点块组成双向链表
	b_type vertex_tail;//顶点链表的尾指针
	v_type vertex_num;//子图中顶点的数目，有维护
    	b_type free_num;//空闲块数目
	b_type block_num;//块的总数目
    	uint32_t block_size;//该子图的block大小
}__attribute__((packed));

//子图类
class Subgraph{
public:
	string graph_dir;//子图对应的目录
        string filename;//子图对应的文件名
	string sub_key;
	fstream io;//子图对应的文件io
	SubgraphHeader head;//子图的头 
        c_type cache;//子图在内存中缓存的结构，该子图所有的块一起管理
	Node* first;//内存中块链表的头
	Node* last;//内存中块链表的尾
	int delete_count;//测试用，缓存替换的次数 
	Btree<v_type,b_type> vertex_index;//顶点的索引类
        lock_t *require_lock;//申请一个块操作的锁
        lock_t *getblock_lock;//获取一个块操作的锁
        lock_t *cache_lock;//对cache操作的锁
        lock_t *vertex_lock;//对顶点的操作是串行，加锁
public:
	//创建一个子图的文件，初始化大小可以配置，返回0表示成功，返回1表示失败
        int init(string filename,string dir);
        //子图大小的动态扩张。大小可以配置，也可以指定
	void add_file(uint32_t size=atoi((getenv("INCREASZ"))));
	//创建子图的头，初始化索引结构，紧跟在init之后
	void format(uint32_t blocksize=atoi(getenv("BLOCKSZ")));
        //已有子图文件的情况下，读取一个子图到内存
	void recover(string name,string dir);
        //初始化锁
        void init_locks();
        //释放锁
        void free_locks();
	//通过子图目录和子图名字得到子图key
	string get_sub_key(string name,string dir);
	//析构函数，把内存中的子图内容写到文件中去
	~Subgraph();
      	//计算块号在的文件中的偏移 
	f_type get_offset(b_type num);
        //对新增的文件初始化索引，紧跟在add_file之后
	void update_index();
	//申请一个新块号，没有空闲块，就要透明地扩展文件大小
	b_type require(uint32_t type);
        //有空闲块的时候，申请一个块号
	b_type requireRaw(uint32_t type);
        //根据缓存中的块，得到该块的包装节点node类
        Node *block2node(void*block);
        //给块释放锁
        void unlock2block(void*block); 
        //根据缓存中的块，得到该块的包装节点node类
        Node *num2node(b_type num);
        //给块释放锁
        void unlock2num(b_type num);
        //根据块号得到一个块，块已经读入缓存中 
	void* get_block(b_type number,char is_new,char is_hash,int is_lock=1);

        //增加一个顶点，并更新顶点索引，顶点已经存在返回1，不存在则插入然后返回0
	int add_vertex(Vertex& vertex,char is_hash=0);
	//顶点是否存在，存在返回1，不存在返回0
	bool vertex_is_in(v_type id);
	//返回顶点的指针，不存在，则返回空指针
	Vertex* get_vertex_raw(v_type id,b_type*num,char is_hash=0);
	//返回顶点类，参数是顶点的id，入股顶点不存在，则返回无效顶点，顶点的id为INVALID_VERTEX
	Vertex get_vertex(v_type id,char is_hash=0);
	//插入一条边，成功了返回0，顶点不存在，则会失败，返回1
	int add_edge(v_type id,Edge& e,char is_hash=0);
	//查询顶点的索引，返回边要插入的块
	b_type index_edge(Vertex* v,v_type id,b_type num,char is_hash=0);
	//读取两个顶点之间的所有边，源顶点不存在，则会返回1，源顶点存在则返回0
	int read_edges(v_type s_id,v_type d_id,list<Edge_u>& edges,char is_hash=0);
	//读取顶点所有的边，源顶点不存在，则会返回1，源顶点存在则返回0
	int read_all_edges(v_type id,list<Edge_u>& edges,char is_hash=0);

	void all_vertex(char is_hash);
	void output_edge(v_type id,char is_hash);
	Edge* read_edge(v_type s_id,v_type d_id,char is_hash);
	void index_output_edge(v_type id,char is_hash);
};



//顶点类
class Vertex{
public:
	v_type id;//顶点的id
	char status;//顶点的状态，该顶点是否被删除等，0代表存在，1代表已经删除
	uint32_t param;//顶点的属性，暂时用一个作为测试
   	e_type size;//边的数目，没有维护
    	b_type head;//顶点块链表的头指针
	b_type tail;//顶点块链表的尾指针，暂时没用到
	b_type index;//索引块链表的头指针
	explicit Vertex(v_type i);
	Vertex(Vertex_u v);
}__attribute__((packed));

//边的类
class Edge{
public:
	char status;//边的状态，有没有被删除，0代表存在，1代表已经删除
	v_type id;//目标顶点id
	uint32_t param;//边的属性，写一个用来测试
	t_type timestamp;//时间戳
	Edge(Edge_u e);
	Edge();
	Edge_u to_edge_u(v_type s_id);	
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
	char clean;//块是否干净，0表示干净	，1表示脏
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
		cout<<"num:"<<number<<" min:"<<min<<" capacity:"<<capacity<<" size:"<<size<<endl;
		uint32_t p=list_head;
		while(p!=INVALID_INDEX){
			cout<<data[p].content.id<<" ";
			p=data[p].next;
		}
		cout<<endl;
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
	//把该块中目标顶点是id的边存入集合中，如果第一条边就是，那么返回1，否则返回0
	int get_contents(v_type id,list<Edge>& edges){
		uint32_t num=list_head;
                int flag=0;
		if(num!=INVALID_INDEX){
			if(data[num].content.id==id) flag=1;
		}
		while(num!=INVALID_INDEX){
			if(data[num].content.id==id) 
				edges.push_back(data[num].content);
			if(data[num].content.id>id) 
				break;//内部是排序的，当边的id大于目标id时，后面就找不到相应的边了，那么退出循环 
			num=data[num].next;
		}
		return flag;
	}
	//把该块中目标顶点是id的边存入Edge_u集合中，如果第一条边就是，那么返回1，否则返回0
	int get_contents(v_type s_id,v_type d_id,list<Edge_u>& edges){
		uint32_t num=list_head;
                int flag=0;
		if(num!=INVALID_INDEX){
			if(data[num].content.id==d_id) flag=1;
		}
		while(num!=INVALID_INDEX){
			if(data[num].content.id==d_id) 
				edges.push_back(data[num].content.to_edge_u(s_id));
			if(data[num].content.id>d_id) 
				break;//内部是排序的，当边的id大于目标id时，后面就找不到相应的边了，那么退出循环 
			num=data[num].next;
		}
		return flag;
	}
	//把该块中的所有的边存入到集合中
	void get_all_contents(v_type s_id,list<Edge_u>& edges){
		uint32_t num=list_head;
		while(num!=INVALID_INDEX){
			edges.push_back(data[num].content.to_edge_u(s_id));
			num=data[num].next;
		}
	}
}__attribute__((packed));




#endif
