#ifndef PANDA_UTIL
#define PANDA_UTIL
#include "panda_head.hpp"
void parse_strings(string params,vector<string>& v,string delims);
void parse_env(string param_name,vector<string>& v,string delims);
pair<uint32_t,string> parse_sub_ip(string params,string delims);
pair<string,uint32_t> parse_ip_num(string params,string delims);
string metapath_key(char* path);
string graph_key(char* path);
#endif
