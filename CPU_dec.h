#pragma once
#include<vector>
#include<map>
#include<stack>
#include<set>
#include"DS_custom.h"
using namespace std;
class CPU
{
public:
	CPU();
	~CPU();

	bool start(const vector<string>& ops);
private:

	map<string, Function> Functions;//�洢���к���
	vector<Scope> Scopes;//�洢����ʱ�����Ĳ������Զ�����
	vector<stack<Var>> Stacks;//�洢����ʱ��������ʱ������ջ
	int layerNum;//���еĲ���
	Var ret;

	void iniStack(stack<Var>& cache,int level);//��ʼ����һ���ջ�ռ�
	void iniStack(int level);//��ʼ����һ���ջ�ռ�
	void desStack();//������һ���ջ�ռ�
	
	void decVar(const string& name,char type,int layer);//��������
	void assVar(const string& name, Var var, int layer);//��������ֵ
	Var getVar(const string& name,int layer);//ȡ����
	void IF(const vector<string>& ops,int &start,int layer);//����if
	void WHILE(const vector<string>&ops, int &start, int layer);//����while
	Function* newFunc(const string& func);//�½�һ������
	void load(const vector<string>& ops);//���ز�������ʵ�ֶ�ȫ�����Ķ���ļ���
	void run(const vector<string>& ops, int layer);//ִ�в�������
	void callFunc(const string& func, int layer);//ִ�к���
};
