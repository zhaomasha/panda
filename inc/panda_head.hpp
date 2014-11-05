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

#include<arpa/inet.h>
using namespace std;

typedef uint64_t v_type;//顶点id的类型
typedef uint32_t e_type;//边数目的类型
typedef uint32_t b_type;//块id的类型
typedef uint64_t t_type;//时间戳的类型
static b_type const INVALID_BLOCK=~0U;
#endif




