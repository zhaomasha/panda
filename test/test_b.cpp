#include "panda_bplus.hpp"
int main(){
	Btree<uint32_t,uint32_t> b;
	//b.recover("b.index");
	//b.init("c.index");
	//b.format();
	b.recover("c.index");
	uint32_t datas[100]={1,1,2,3,8,0,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,200,200,12,45,23,67,35,200,200,200,200,200,123,345,23,34,34,34,34,34,34,34,34,1,1,2,3,8,0,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,200,200,12,45,23,67,35,200,200,200,200,200,123,345,23,34,34,34,34,34,34,34,34};
	//b.head.free_num=2;
	//b.head.print();
	/*b_type no1=b.require(1);
	cout<<"no1 "<<no1<<endl;
	Bleaf<uint32_t,value>* bn1=(Bleaf<uint32_t,value>*)b.get_block(no1);	
	cout<<bn1->capacity<<endl;	
	bn1->keys[20]=3;
	bn1->values[30].a=4;
	bn1->values[30].b=5;
	bn1->clean=1;
	b_type no2=b.require(2);
	Bnode<uint32_t>* bn2=(Bnode<uint32_t>*)b.get_block(no2);
	b_type no3=b.require(2);
	Bnode<uint32_t>* bn3=(Bnode<uint32_t>*)b.get_block(no3);
	cout<<bn2->capacity<<endl;	
	bn1=(Bleaf<uint32_t,value>*)b.get_block(2558);	
	cout<<bn1->keys[20]<<"  "<<bn1->values[30].b<<endl;
	cout<<b.require(1)<<endl;
	cout<<"delete:"<<b.delete_count<<endl;*/
	//srand((unsigned)time(0));
	/*for(int i=0;i<500;i++){
		int r=rand()%500;
		b.insert_kv(r,r);
	}*/
	/*for(int i=0;i<600000;i++)
		b.insert_kv(i,i);*/
	b.scan();
	/*cout<<"~~~~~~~~"<<endl;
	list<uint32_t> values;
	b.find_values(1,values);
	list<uint32_t>::iterator it;
	for(it=values.begin();it!=values.end();it++){
		cout<<*it<<" ";
	}*/
	/*list<uint32_t> values1;
	b.find_values(2,values1);
	for(it=values1.begin();it!=values1.end();it++){
		cout<<*it<<" ";
	}
	list<uint32_t> values2;
	b.find_values(3,values2);
	for(it=values2.begin();it!=values2.end();it++){
		cout<<*it<<" ";
	}
	list<uint32_t> values3;
	b.find_values(40,values3);
	for(it=values3.begin();it!=values3.end();it++){
		cout<<*it<<" ";
	}
	list<uint32_t> values4;
	b.find_values(326,values4);
	for(it=values4.begin();it!=values4.end();it++){
		cout<<*it<<" ";
	}*/
}
