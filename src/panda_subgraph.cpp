#include "panda_subgraph.hpp"
//����һ����ͼ���ļ�����ʼ����СΪ16M
    void Subgraph::init(string name){
		filename=name;
		char tmp[1024*1024*atoi(getenv("INITSZ"))];
		//�ļ����������㣬�������򴴽�
		io.open(filename.c_str(),fstream::out|fstream::in|ios::binary|fstream::trunc);
		io.write(tmp,sizeof(tmp));
	}
	
	//������ͼ��ͷ����ʼ�������ṹ
	void Subgraph::format(){
		head.free_head=INVALID_BLOCK;
		head.vertex_head=INVALID_BLOCK;		
		head.vertex_tail=INVALID_BLOCK;		
		head.vertex_num=0;
		head.block_size=atoi(getenv("BLOCKSZ"));
		//����ļ���С��������п�Ĵ�С
		io.seekg(0,fstream::end);
		uint32_t file_len=io.tellg();
		cout<<"filelen:"<<file_len<<endl;
		head.free_num=(file_len-sizeof(SubgraphHeader))/head.block_size;

		cout<<"freenum:"<<head.free_num<<endl;
		io.seekp(0);
		io.write((char*)&head,sizeof(SubgraphHeader));
		io.seekg(0);
		io.read((char*)&head,sizeof(SubgraphHeader));			
		cout<<"new  freenum:"<<head.free_num<<endl;	
	}