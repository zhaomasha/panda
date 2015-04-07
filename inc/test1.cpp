#include "panda_bplus.hpp"
#include "panda_type.hpp"
int main(){
   Btree<Key,Value> index;
   index.recover("test.index");
   Key k1("12134");
   Key k2("11111");
   Value v1(1,1);
   Value v2(3,2);
   Value v3(4,5);
   index.insert_kv(k1,v1);
   index.insert_kv(k1,v2);
   index.insert_kv(k2,v3);
   list<Value> vs;
   list<Value>::iterator it;
   index.find_values(k1,vs);
   for(it=vs.begin();it!=vs.end();it++){
       cout<<(*it).s_id<<" "<<(*it).d_id<<endl;
   }
     
}
