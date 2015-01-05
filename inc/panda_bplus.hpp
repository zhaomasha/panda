#ifndef PANDA_BPLUS
#define PANDA_BPLUS
#include "panda_head.hpp"
/*class value{
public:
	int a;
	int b;
	value(int a,int b){
		this->a=a;
		this->b=b;
	}
};
ostream& operator<< (ostream& out,value& v){
	cout<<v.a<<"-"<<v.b;
	return out;
}*/
//B+树的节点基类型
template<typename T>
class Bnode{
	public:
		char type;//节点的类型，1代表叶子节点，2代表非叶子节点
		b_type number;//块号
		b_type pre;
		b_type next;//这两个指针在B+树中的叶子节点中充当双向链表指针
		char clean;//0表示干净，1表示脏
		b_type parent;//父节点
		uint32_t fix;//0表示没有被钉住，大于0表示被盯住
		uint16_t capacity;//节点容纳的最大的键数目
		uint16_t size;//已经存取的键数目
		T *keys;//键
};
//B+树的叶子节点
template<typename T,typename TV>
class Bleaf:public Bnode<T>{
	public:
		TV *values;//值
		void init(char node_type,uint32_t blocksz){
			//初始化叶子节点，键数目，已经存取的数目
			this->capacity=(blocksz-sizeof(Bleaf<char,char>))/(sizeof(T)+sizeof(TV));
			this->size=0;
			this->parent=INVALID_BLOCK;
			this->type=node_type;
		}
		void init_mem(){
			//初始化键和值
			this->keys=(T*)(this+1);
			values=(TV*)(this->keys+this->capacity);
		}
		void print(){
			cout<<"block number:"<<this->number<<endl;
			for(int i=0;i<this->size;i++){
				cout<<this->keys[i]<<":"<<values[i]<<" ";
			}
			cout<<endl;
		}
};
//B+树的非叶子节点
template<typename T>
class Bnoleaf:public Bnode<T>{
	public:
		b_type *points;//指针
		void init(char node_type,uint32_t blocksz){
			//初始化叶子节点，键数目，已经存取的数目
			this->capacity=(blocksz-sizeof(Bnoleaf<char>)-sizeof(b_type))/(sizeof(T)+sizeof(b_type));
			this->size=0;
			this->parent=INVALID_BLOCK;
			this->type=node_type;
		}
		void init_mem(){
			//初始化键和指针
			this->keys=(T*)(this+1);
			points=(b_type*)(this->keys+this->capacity);
		}
};

//b+树的头，存于文件中的内容
class BtreeHeader{
	public:
		uint32_t block_size;//块的大小
		b_type root_node;//根节点
		b_type first_node;//叶子节点的第一个节点
		b_type free_head;//空闲链表的头块
		b_type free_num;//空闲块的数目
		b_type block_num;//块的总数目
		void print(){
			cout<<block_size<<" "<<root_node<<" "<<first_node<<" "<<free_head<<" "<<free_num<<" "<<block_num<<endl; 
		}
};

template<typename T,typename TV>
class Btree{
	public:
		string filename;//索引对应的文件名
		fstream io;
		BtreeHeader head;
		c_type cache;//缓存
		Node* first;//内存中块链表的头
		Node* last;//尾
		int delete_count;
	
		//新建一个索引文件(文件存在，则会被清零)，初始化为默认的大小（操作系统的栈大小要调整，否则会段错误）
		void init(string name){
			//文件存在则清零，不存在则创建
			filename=name;
			io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
			first=last=NULL;
			delete_count=0;
			//初始化默认大小
			add_file(atoi(getenv("INITSZ")));
		}

		//文件扩张，默认大小可以配置，也可以指定
		void add_file(uint32_t size=atoi(getenv("INCREASZ"))){
			io.seekp(0,fstream::end);
			char tmp[1024*1024*size];
			io.write(tmp,sizeof(tmp));
		}
		//创建索引文件的头，初始化索引结构等等，参数是该索引文件的block大小
		void format(uint32_t block_size=atoi(getenv("INDEX_BLOCKSZ"))){
			head.free_head=INVALID_BLOCK;
			head.free_num=0;
			head.block_num=0;
			head.block_size=block_size;
			head.root_node=INVALID_BLOCK;
			head.first_node=INVALID_BLOCK;
			//初始化索引
			update_index();
		}
		//读取索引文件，还原一个内存中的索引结构
		void recover(string name){
			filename=name;
			io.open(filename.c_str(),fstream::out|fstream::in|ios::binary);
			io.seekg(0);
			io.read((char*)&head,sizeof(BtreeHeader));//读取子图文件中的子图头，这部分数据要事先读入内存
			first=last=NULL;//内存缓冲区是0
			delete_count=0;	
		}
		//析构函数，把子图头和内存中的缓存存到硬盘里
		~Btree(){
			cout<<"index: "<<filename<<" xigou"<<endl;
			//把子图头存入文件
			io.seekp(0);	
			//io.seekp(get_offset(block->number));
			io.write((char*)&head,sizeof(BtreeHeader));
			//遍历子图的缓存里面的块，脏了就写到文件里面
			c_it it;
			it=cache.begin();
			Node* node;
			while(it!=cache.end()){
				node=it->second;
				if(((Bnode<char>*)(node->block))->clean==1){
					//脏块，则写入到文件中
					io.seekp(get_offset(((Bnode<char>*)(node->block))->number));
					io.write((char*)(node->block),head.block_size);
					//cout<<((Bnode<char>*)(node->block))->number<<" dirty ";
				}	
				free(node);
				it++;
			}
		}
		//计算块对应的文件偏移
		f_type get_offset(b_type num){
			return sizeof(BtreeHeader)+num*head.block_size;
		}
		//对文件新增的部分建立索引，文件新增的部分还没有取到内存中，否则会有一致性的问题
		void update_index(){
			//计算新增部分的块的数目
			io.seekg(0,fstream::end);
			f_type file_size=io.tellg();
			b_type num=head.block_num;
			b_type blocks=((long)file_size-sizeof(BtreeHeader)-head.block_size*num)/head.block_size;
			//开辟一个块的内存(只需要块头的大小，不需要整个块)，依次为每个块建立索引，再写入到磁盘中对应的位置
			Bnode<char> *block=(Bnode<char>*)malloc(sizeof(Bnode<char>));
			int i;	
			for(i=0;i<blocks;i++){
				//block从0开始编号
				block->number=num+i;
				block->next=head.free_head;
				head.free_head=block->number;
				head.free_num++;
				head.block_num++;
				io.seekp(get_offset(block->number));
				io.write((char*)block,sizeof(Bnode<char>));
			}
			free(block);	
		}

		//分配一个空闲块，无论空闲块是否还有，参数为块的类型，返回块号(要注意的是，require后的这个块已经脱离了控制，如果当时没用到，则会丢失)
		b_type require(uint32_t type){
			if(head.free_num==0){
				//如果空闲块为0，则扩展子图文件，并建索引
				add_file();//按默认配置的大小分配
				update_index();//对分配的文件建立索引
			}
			if(head.free_num>0)
				return requireRaw(type);
			else
				return INVALID_BLOCK;//返回无效块

		}
		//空闲块还有的时候，分配一个块，返回块的编号，只是分配工作，还需要get_block函数来返回该块
		b_type requireRaw(uint32_t type){
			//如果还有空闲块，则直接剥离出一个块，初始化块头，修改子图的头，并返回块地址
			b_type number=head.free_head;
			//分配一块内存，再把文件中的块读入到这个内存里
			Bnode<char> *block=(Bnode<char>*)malloc(head.block_size);
			io.seekg(get_offset(number));
			io.read((char*)block,head.block_size);
			//初始化块头公共的部分，比如块的类型，块的大小，修改子图的一些字段，比如剩余空闲块，链表头，块内部的内容（索引等）由块初始化来完成，
			head.free_head=block->next;
			head.free_num--;
			//不同的块类型，容量不一样
			if(type==1){
				//叶子节点，初始化节点的容量
				((Bleaf<T,TV>*)block)->init(type,head.block_size);
			}

			if(type==2){
				//非叶子节点，初始化节点的容量
				((Bnoleaf<T>*)block)->init(type,head.block_size);
			}	
			//把块头的内容写入到文件里，可以只写块头的大小，也可以写块的大小
			io.seekp(get_offset(number));
			io.write((char*)block,head.block_size);
			free(block);
			return number;	
		}


		//得到一个块，该块号要存在，如果块缓存在cache中，则直接返回，如果没有缓存，则从文件中读取到cache，有必要的时候要移除一个cache，还没考虑到锁
		//并且创建链表，新块放置在链头，旧块放置在链尾
		void* get_block(b_type number){
			c_it node_it=cache.find(number);
			void *block;
			Node *node;
			if(node_it!=cache.end()){
				//如果该块在缓存中，则返回块指针
				return node_it->second->block;
			}
			else{
				//如果该块不在缓存中，则读入该块
				if(!(cache.size()<atoi(getenv("CACHESZ")))){
					//如果缓存满了，则移除链表中的最后一个块，被盯住的块不能移除，删除前要判断该块是否脏了，脏了就要写入到文件
					node=last;
					while(node!=NULL){
						//遍历缓存，遇到没盯住的块就停止，基本上是常数时间，被盯住的块一般是在链表头，还可以改进成LRU，但效果可能不是很明显
						if(((Bnode<char>*)(node->block))->fix==0) {
							break;
						}
						else {
							node=node->pre;
						}
					}
					if(node==NULL) {
						return NULL;//如果块都被盯住，则返回空指针
					}
					if(((Bnode<char>*)(node->block))->clean==1){
						//块脏了，则写入到文件中
						io.seekp(get_offset(((Bnode<char>*)(node->block))->number));
						io.write((char*)(node->block),head.block_size);
					}
					//释放块所占的内存，在缓存结构里移除，以及更新链表
					//更新链表要分3种情况，该块在末尾，链表中，头部，！！！！！！！！多线程的时候要重点考虑这里的锁机制，假设缓存不会只有一个块，否则会有问题
					if(node->pre==NULL){
						//头部
						Node *tmp=node->next;
						tmp->pre=NULL;
						first=tmp;
					}else{
						if(node->next==NULL){
							//尾部
							Node *tmp=node->pre;
							tmp->next=NULL;
							last=tmp;
						}else{
							//中间
							Node *tmp_pre=node->pre;
							Node *tmp_next=node->next;
							tmp_pre->next=tmp_next;
							tmp_next->pre=tmp_pre;
						}
					}
					Node *tmp=node;
					cache.erase(((Bnode<char>*)(node->block))->number);//在子图hash表里面移除元素	
					free(tmp);//释放该块所占的内存
					delete_count++;
				}
				//分配内存，把块读进来	
				node=(Node*)malloc(sizeof(Node)+head.block_size);
				node->block=node+1;
				block=node->block;
				io.seekg(get_offset(number));
				io.read((char*)(block),head.block_size);
				((Bnode<char>*)block)->clean=0;//刚进来的块是干净的	
				((Bnode<char>*)block)->fix=0;//刚进来的块没有被盯住
				//只有块在内存的时候，才会把块的data字段指向正确的块内容区域
				if(((Bnode<char>*)block)->type==1){
					((Bleaf<T,TV>*)block)->init_mem();
				}
				if(((Bnode<char>*)block)->type==2){
					((Bnoleaf<T>*)block)->init_mem();
				}
				//把块加入缓存中
				cache[number]=node;
				//更新链表
				if(first==NULL){
					first=last=node;
					node->pre=NULL;
					node->next=NULL;
				}else{
					node->pre=NULL;
					first->pre=node;
					node->next=first;
					first=node;
				}	
				return block;
			}
		}
		//叶子节点插入一条项目
		void insert_leaf(Bleaf<T,TV>* node,T key,TV value){
			int i;
			//找到要插入的槽
			for(i=0;i<node->size;i++){
				if(key<=node->keys[i]) break;
			}
			//把后面的元素依次后移，包括键和值
			for(int j=node->size-1;j>=i;j--){
				node->keys[j+1]=node->keys[j];
				node->values[j+1]=node->values[j];
			}
			node->keys[i]=key;
			node->values[i]=value;
			node->size++;
			node->clean=1;//脏位置1
			//split_leaf(node);
		}
		//非叶子节点插入一条项目
		void insert_noleaf(Bnoleaf<T>* node,T key,b_type point){
			int i;
			//找到要插入的槽
			for(i=0;i<node->size;i++){
				if(key<=node->keys[i]) break;
			}
			//把后面的元素依次后移，包括键和值
			for(int j=node->size-1;j>=i;j--){
				node->keys[j+1]=node->keys[j];
				node->points[j+2]=node->points[j+1];
			}
			node->keys[i]=key;
			node->points[i+1]=point;
			node->size++;
			node->clean=1;//脏位置1
			//split_noleaf(node);
		}
		//插入后检查叶子节点是否已经满了，满了则进行分裂，更新父节点
		void split_leaf(Bleaf<T,TV>* node,b_type parent_num){
			if(node->size<node->capacity) return;//如果还没满，则不用分裂
			//创建一个新块
			b_type right_node_num=require(1);
			node->fix++;//把旧块盯住
			Bleaf<T,TV>* right_node=(Bleaf<T,TV>*)get_block(right_node_num); 
			//复制后面一半的内容给新块
			uint16_t mid=node->capacity/2;
			for(int i=mid,j=0;i<node->capacity;i++,j++){
				right_node->keys[j]=node->keys[i];
				right_node->values[j]=node->values[i];
				right_node->size++;
			}
			node->size=mid;
			//更新双向链表
			right_node->pre=node->number;
			right_node->next=node->next;
			node->next=right_node->number;
			right_node->fix++;
			if(right_node->next!=INVALID_BLOCK){
				Bleaf<T,TV>* tmp_node=(Bleaf<T,TV>*)get_block(right_node->next);
				tmp_node->pre=right_node->number;
				tmp_node->clean=1;
			}		
			//更新父节点
			if(parent_num==INVALID_BLOCK){
				//如果父节点是空，则创建一个
				b_type parent_num=require(2);
				Bnoleaf<T>* parent_node=(Bnoleaf<T>*)get_block(parent_num);
				head.root_node=parent_num;//该节点一定是root
				//更新父节点
				parent_node->points[0]=node->number;
				parent_node->clean=1;
				insert_noleaf(parent_node,right_node->keys[0],right_node->number);
				
			}else{
				Bnoleaf<T>* parent_node=(Bnoleaf<T>*)get_block(parent_num);
				parent_node->clean=1;
				insert_noleaf(parent_node,right_node->keys[0],right_node->number);
			}
			right_node->clean=1;
			node->clean=1;
			node->fix--;
			right_node->fix--;
			
		}
		void split_noleaf(Bnoleaf<T>* node,b_type parent_num){
			if(node->size<node->capacity) return;//如果还没满，则不用分裂
			//创建一个新块
			b_type right_node_num=require(2);
			node->fix++;//把旧块盯住
			Bnoleaf<T>* right_node=(Bnoleaf<T>*)get_block(right_node_num); 
			//复制后面一半的内容给新块
			uint16_t mid=node->capacity/2;
			right_node->points[0]=node->points[mid+1];
			for(int i=mid+1,j=0;i<node->capacity;i++,j++){
				right_node->keys[j]=node->keys[i];
				right_node->points[j+1]=node->points[i+1];
				right_node->size++;
			}
			node->size=mid;
			T key=node->keys[mid];
			right_node->fix++;
			//更新父节点
			if(parent_num==INVALID_BLOCK){
				//如果父节点是空，则创建一个
				parent_num=require(2);
				Bnoleaf<T>* parent_node=(Bnoleaf<T>*)get_block(parent_num);
				head.root_node=parent_num;//该节点一定是root
				//更新父节点
				parent_node->points[0]=node->number;
				parent_node->clean=1;
				insert_noleaf(parent_node,key,right_node->number);
				
			}else{
				Bnoleaf<T>* parent_node=(Bnoleaf<T>*)get_block(parent_num);
				parent_node->clean=1;
				insert_noleaf(parent_node,key,right_node->number);
			}
			right_node->clean=1;
			node->clean=1;
			node->fix--;
			right_node->fix--;

		}
		//从根开始寻找key可能所在的叶子节点，递归调用，这个标志位用来决定当第一个叶子节点没有找到key时，需不需要再继续找下面的叶子节点了
		Bleaf<T,TV>* search_leaf_node(Bnode<T>* node,T key,int& flag){
			if(node->type==1){
				//如果是叶子节点，则返回该节点
				return (Bleaf<T,TV>*)node;
			}
			int i;
			for(i=0;i<node->size;i++){
				if(key<node->keys[i]) {
					flag=0;
					break;
				}
				if(key==node->keys[i]) {
					flag=1;//只有碰到和非叶子节点的key相等的键时，flag才会为1，而且一直为1
					break;
				}
			}
			b_type child_num=((Bnoleaf<T>*)node)->points[i];
			//不需要盯住块
			Bnode<T>* child_node=(Bnode<T>*)get_block(child_num);
			return search_leaf_node((Bnoleaf<T>*)child_node,key,flag); 
		}
		//查询key，结果存与list结构中
		void find_values(T key,list<TV>& values){
			b_type num=head.root_node;
			if(num==INVALID_BLOCK) return;//如果根节点不存在，直接返回，list中也就没有值
			Bnode<T>* root=(Bnode<T>*)get_block(num);
			int flag=0;
			Bleaf<T,TV>* node=search_leaf_node(root,key,flag);
			find_values_raw(node,key,values,flag);
		}	
		//key可能在第一个node里面，也可能不在，但是要根据flag的值继续扫描相邻的node
		void find_values_raw(Bleaf<T,TV>*node,T key,list<TV>& values,int& flag){
			int i;
			//for循环，遍历第一个node
			for(i=0;i<node->size;i++){
				if(key<node->keys[i]) return;
				if(key==node->keys[i]){
					values.push_back(node->values[i]);
					for(i++;i<node->size;i++){
						if(key==node->keys[i])	values.push_back(node->values[i]);
						else break;
					}
				}
			}
			//根据flag，决定要不要遍历后面的node
			if(flag==1){
				//flag为1，要继续遍历后面的，而且肯定是连续的，只要第一条不是，该块后面的也不会是
				b_type next_num=node->next;
				while(next_num!=INVALID_BLOCK){
					Bleaf<T,TV>* next_node=(Bleaf<T,TV>*)get_block(next_num);
					for(i=0;i<next_node->size;i++){
						if(key==next_node->keys[i]){
							values.push_back(next_node->values[i]);
						}else
							break;
					}
					if(i==next_node->size){
						next_num=next_node->next;
					}else{
						next_num=INVALID_BLOCK;
					}
				}	
			}
			
		}
		//b+树中插入一个key-value
		/*void insert_kv(T key,TV value){
			if(head.root_node==INVALID_BLOCK){
				//如果没有根，则创建一个叶子节点，并且作为根
			head.first_node=head.root_node=require(1);
				Bleaf<T,TV>* node=(Bleaf<T,TV>*)get_block(head.root_node);
				node->next=node->pre=INVALID_BLOCK;//更新双向链表
				insert_leaf(node,key,value);
			}else{
				Bnode<T>* node=(Bnode<T>*)get_block(head.root_node);
				if(node->type==1){
					//如果根节点是叶子节点，则直接插入
					insert_leaf((Bleaf<T,TV>*)node,key,value);
				}else{
					//如果不是叶子节点，则查找要插入的叶子节点
					Bleaf<T,TV>* leaf=search((Bnoleaf<T>*)node,key);
					insert_leaf(leaf,key,value);
				}
			}
		}*/
		//路径上的所有节点都会被盯着，直到开始回塑，才一个个释放
		void insert_kv_raw(b_type num,b_type parent_num,T key,TV value){
			Bnode<T>* node=NULL;	
			if(num==INVALID_BLOCK){
				//如果节点为空，则创建一个叶子节点，并且作为根
				head.first_node=head.root_node=require(1);
				node=(Bnode<T>*)get_block(head.root_node);
				node->next=node->pre=INVALID_BLOCK;//更新双向链表
			}else{
				node=(Bnode<T>*)get_block(num);
			}
			node->fix++;//第一次经过的时候要盯着，回塑的时候，会再次用到
			if(node->type==1){
				//如果是叶子节点
				insert_leaf((Bleaf<T,TV>*)node,key,value);
				split_leaf((Bleaf<T,TV>*)node,parent_num);//假设每个节点的最小值不会是1，否则还得检查新建的父节点是否split
				node->clean=1;	
				node->fix--;
			}else{
				//如果不是叶子节点
				int i;
				for(i=0;i<node->size;i++){
					if(key<=node->keys[i]) break;
				}
				b_type child_num=((Bnoleaf<T>*)node)->points[i];
				insert_kv_raw(child_num,num,key,value);
				split_noleaf((Bnoleaf<T>*)node,parent_num);
				node->clean=1;
				node->fix--;
			}
		}
		void insert_kv(T key,TV value){
			insert_kv_raw(head.root_node,INVALID_BLOCK,key,value);
		}
		//查找一个key的所有value，返回一个集合
		/*void find_key(T key,vector<TV>& values){
			int i;
			for(i=0;i<node->size;i++){
				if(key<=node->keys[i]) break;
			}
			b_type child_num=node->points[i];
			Bnode<T>* child_node=(Bnode<T>*)get_block(child_num);
			if(child_node->type==1) return (Bleaf<T,TV>*)child_node;
			else return search((Bnoleaf<T>*)child_node,key);
		}*/
		
		//输出所有叶子节点的值
		void scan(){
			b_type num=head.first_node;
			while(num!=INVALID_BLOCK){
				Bleaf<T,TV>* node=(Bleaf<T,TV>*)get_block(num);
				node->print();
				num=node->next;
			}
		}	
};



#endif




