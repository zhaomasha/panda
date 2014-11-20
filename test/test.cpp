#include<iostream>
#include<arpa/inet.h>
using namespace std;

int main(){
	uint32_t o=0;
	uint32_t x=~o;
	if(23>x) cout<<"sds";
	else cout<<x;
}

