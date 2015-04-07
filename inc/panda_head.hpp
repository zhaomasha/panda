#ifndef PANDA_HEAD 
#define PANDA_HEAD
//公共的头文件，以及一些常量和类型别名
#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<memory>
#include<string>
#include<vector>
#include<list>
#include<set>
#include<map>
#include<tr1/unordered_map>
#include<exception>
#include<algorithm>

#include<cstring>
#include<cstdlib>
#include<cerrno>
#include<cstdarg>
#include<cassert>
#include<cstdio>
#include<ctime>
#include<csignal>
#include<climits>
#include<cstddef>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/mman.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/socket.h>
#include<sys/eventfd.h>
#include<sys/time.h>
#include<sys/signalfd.h>
#include<sys/timerfd.h>
#include<sys/epoll.h>


#include<unistd.h>
#include<fcntl.h>
#include<execinfo.h>
#include<glob.h>
#include<netdb.h>
#include<string.h>

#include<arpa/inet.h>
#include<pthread.h>
#define BLOGID_LEN 32 //边的blog_id属性的最大长度
#define NICKNAME_LEN 64 //顶点的昵称属性的最大长度

using namespace std;
using namespace std::tr1;

typedef uint64_t v_type;//顶点id的类型
typedef uint32_t e_type;//边数目的类型
typedef uint32_t b_type;//块id的类型
typedef time_t t_type;//时间戳的类型
typedef pthread_mutex_t lock_t;//锁的类型
typedef fstream::pos_type f_type;//文件偏移的类型
static v_type V_ZERO=0;
static b_type B_ZERO=0;
static b_type const INVALID_BLOCK=~B_ZERO;//无效的块号
static v_type const INVALID_VERTEX=~V_ZERO;//无效的顶点号
static uint32_t const INVALID_INDEX=~0U;//无效的索引号
//typedef unordered_map<b_type,void*> c_type;//缓存的类型
//typedef unordered_map<b_type,void*>::iterator c_it;
//这是一个包装类，为缓冲区中的块添加链表，存储块的缓冲区是hash加链表的结构
class Node{
public:
	void *block;
	Node *pre;
	Node *next;
        lock_t *lock;//缓存中每个block有一个锁
}__attribute__((packed));

typedef unordered_map<b_type,Node*> c_type;//子图内存储块的hash结构
typedef unordered_map<b_type,Node*>::iterator c_it;


#endif




