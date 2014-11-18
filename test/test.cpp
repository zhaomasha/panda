#include<iostream>
using namespace std;
template<typename T>
class A{
public:
	T param;
	void func();
};
//template<typename T>
void A::func(){
	cout<<param<<endl;
}
int main(){
	A<double> a;
	a.func();
}

