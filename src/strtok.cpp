#include<vector>
#include<iostream>
#include "panda_util.hpp"
using namespace std;
int main()
{
	string s("12.12.12.12:12.12.12.13");
	vector<string> v;
	parse_strings(s,v,":");
	cout<<v.size();
}
