#include<iostream>
#include"CPU_dec.h"
#include"Exception_dec.h"
using namespace std;
CPU::CPU() : Functions(), Scopes(), Stacks(), layerNum(0), ret('i'), line_map(),op_line(0),
MainOps(), returnFlag(false), wait(), tempWait(-1), nowLine(-1), runnable(true), origin_ops()
{
}

CPU::~CPU()
{
}

void CPU::addOp(const vector<string>& ops) {
	for (int i = 0; i < ops.size(); i++) {
		line_map.insert(pair<int, int>(origin_ops.size(), op_line));
		origin_ops.push_back(ops[i]);
	}
	op_line++;
}
int CPU::getLineByLine(int line) {
	auto ret = line_map.find(line);
	if (ret == line_map.end())
		return -1;
	else
		return ret->second;
}
bool CPU::start() {
	
	load();//�����ڱ������
	
	if (MainOps.size() == 0)
		throw Compile_E("δ����main����");
	
	iniStack(0);
	run(MainOps);
	desStack();
	if(!ret.init)
		throw Compile_E("main ����û��return��� �� �������з�֧����return���");
	return !Bool(ret);
}
void CPU::load(){
	const vector<string>& ops = origin_ops;
	map<string, Function>::iterator func_it = Functions.end();
	int count;
	for (int i = 0; i < ops.size(); i++) {
		if (func_it == Functions.end()) {
			if (ops[i][0] == 'q' && ops[i][1] == '@') {
				if (ops[i].substr(2, 5) == "main,") {
					if (MainOps.size() != 0)
						throw Compile_E(i,"main�����������ض��������");
					if (ops[i].length() != 9)
						throw Compile_E(i,"main�����������в���");
					if (ops[i][7] != 'i')
						throw Compile_E("main������Ҫ����int��ֵ");
					count = 1; i++;
					for (; i < ops.size(); i++) {
						if (ops[i][0] == '0') {
							if (ops[i][1] == '}')
								count--;
							else if (ops[i][1] == 'i' ||
								ops[i][1] == 'e' ||
								ops[i][1] == 'w')
								count++;
							if (count == 0)//main������������
								break;
						}
						MainOps.push_back(pair<int,string>(i,ops[i]));
					}
				}
				else {
					func_it = newFunc(ops[i]);
					count = 1;
				}
			}
			else throw Other_E(i,string("����ʶ��Ĳ���:")+ops[i]);
		}
		else {
			func_it->second.ops.push_back(pair<int,string>(i,ops[i]));
			if (ops[i][0] == '0') {
				if (ops[i][1] == 'i' || ops[i][1] == 'e' || ops[i][1] == 'w') count++;
				else if (ops[i][1] == '}') {
					count--;
					if (count == 0)
						func_it = Functions.end();
				}
			}
		}
	}
}
void CPU::trueReturn(char ret_type,const string& funcSign) {
	if (ret_type == 'v') {
		if (ret.type != 'v')
			throw Simple_E(string("����(") + funcSign + ")��Ӧ���з���ֵ");
	}
	else if (ret.type =='v')
		throw Simple_E(string("����(") + funcSign + ")Ӧ���з���ֵ");
	else if (ret.type == 's') ret = vString(ret);
	else if (ret_type == 'i') ret = vInt(ret);
	else ret = vFloat(ret);
}
void CPU::callFunc(const string& func) {
	//form the layer [layer] call function with string:
	//^<func_name>,<para1_name>,<para2_name>,<...>,
	vector<string> element(0);
	string paras_type = "";
	Var* t_var;
	stack<Var> cache;
	int t_index = 1;

	for (int index = 2; index < func.length(); index++)
		if (func[index] == ',') {
			element.push_back(func.substr(t_index + 1, index - t_index - 1));
			t_index = index;
		}

	for (vector<string>::iterator it = element.begin() + 1; it != element.end(); it++) {
			t_var = &getVar(*it);

		cache.push(*t_var);
		paras_type.push_back(t_var->type);
	}
	string signature = element[0] + '|' + paras_type;
	map<string, Function>::iterator func_it = Functions.find(signature);
	if (func_it == Functions.end())
		throw Simple_E(string("������δ����ĺ���") + element[0]);
	else {
		//��ʼ����ջ�ռ�
		iniStack(cache,2);
		//��layer+1�����к���
		run(func_it->second.ops);
		desStack();
		trueReturn(func_it->second.ret_type,signature);
	}

}
void CPU::pushCall(const string& func) {
	callFunc(func);
	if (ret.type == 'v')
		throw Simple_E("voidֵ���ܱ�ʹ��");
	Stacks[layerNum - 1].push(ret);
	ret.type = 'v';
}
map<string, Function>::iterator CPU::newFunc(const string& func) {
	//q@[f_name],[r_type][p_type][p_type],[p_name],[p_name],
	vector<string> element(0);
	int t_index = 1;

	for (int index = 2; index < func.length(); index++)
		if (func[index] == ',') {
			element.push_back(func.substr(t_index + 1, index - t_index - 1));
			t_index = index;
		}
	string signature = element[0] + '|' + element[1].substr(1);
	map<string, Function>::iterator it = Functions.find(signature);
	if (it == Functions.end()) {
		Functions.insert(pair<string, Function>(signature, Function(vector<string>(element.begin() + 2, element.end()), element[1][0])));
		it = Functions.find(signature);
		//�ں������������� 
		//�ں����и�������ֵ
		//�� i[1,n]������������element[1][i] ����element[1+i]
		for (int i = 1; i < element[1].length(); i++)
		{
			
			it->second.ops.push_back(pair<int,string>(-1,string("q_") + element[1][i] + element[i + 1]));
			it->second.ops.push_back(pair<int, string>(-1, string("=") + element[i + 1] + ",?"));
		}
		return it;
	}
	else
		throw Simple_E(string("�ض��庯����") + element[0]);
	
	
}

Var& CPU::decVar(const string& name, char type) {
	//declare var in layer
	int layer = layerNum - 1;
	map<string, Var >::iterator it = Scopes[layer].vars.find(name);
	if (it != Scopes[layer].vars.end())
		throw Simple_E(string("�ض������:") + name);
	else 
		Scopes[layer].vars.insert(pair<string, Var>(name, Var(type)));
	return Scopes[layer].vars.find(name)->second;
}
Var CPU::getVar(const string& name) {
	Var ret = getVar_(name);
	return ret;
}
Var& CPU::getVar_(const string&name) {
	int layer = layerNum - 1;
	if (name[0] == '?')
		return popStack();
	else if (name[0] == '+' || name[0] == '-' || name[0] <= '9' && name[0] >= '0') {
		return vNum(name);
	}
	else if (name[0] == '\"') {
		//�ַ���
		return vString(Escape(name.substr(1, name.length() - 2)));
	}
	layer = findVar(name);
	if (layer == -1)
		return decVar(name, 'i');
	else
		return Scopes[layer].vars.find(name)->second;
}

void CPU::iniStack(stack<Var> & cache, char level) {
	iniStack(level);
	while (!cache.empty()) {
		Stacks[layerNum-1].push(cache.top());
		cache.pop();
	}
}
void CPU::iniStack(char level) {
	if (layerNum == Scopes.size()) {
		Scopes.push_back(Scope());
		Stacks.push_back(stack<Var>());
	}

	Scopes[layerNum].level = level;
	layerNum++;
}
void CPU::desStack() {
	layerNum--;
	for (auto var : Scopes[layerNum].vars)
		if (var.second.type == 's')
			delete var.second.var.str_p;
	Scopes[layerNum].vars.clear();
	while (!Stacks[layerNum].empty()) {
		throw Simple_E("�Ҹо����е�������Ǵ���,��һ��������ĳ����������ˣ�������ʱ�ı�����������GG.");
		Stacks[layerNum].pop();
	}
}
Var CPU::popStack() {
	if (layerNum < 1)
		throw Exe_E("ջ���쳣���");
	if (Stacks[layerNum - 1].empty()) {
		throw Exe_E("���Դӿ�ջȡջ����");
		return Var('v');
	}
	Var var = Stacks[layerNum - 1].top();
	Stacks[layerNum - 1].pop();
	return var;
}

void CPU::IF(const vector<pair<int,string>>& ops, int& start) {
	bool ifFlag;
	bool hasElse;
	int if_ = start,else_ = ops.size();
	vector<pair<int,string>> nextOps(0);
	ifFlag = Bool(getVar(ops[start].second.substr(2)));
	int count = 1;
	for (start++; start < ops.size(); start++) {
		if (ops[start].second[0] == '0') {
			if (ops[start].second[1] == 'i'||
				ops[start].second[1] == 'e' ||
				ops[start].second[1] == 'w') count++;
			else if (ops[start].second[1] == '}') {
				count--;
				if (count == 0) break;
			}
		}
	}
	start++;
	if (ops[start].second[0] == '0' && ops[start].second[1] == 'e') {//��else
		else_ = start;
		hasElse = true;
		count = 1;
		for (start++; start < ops.size(); start++) {
			if (ops[start].second[0] == '0') {
				if (ops[start].second[1] == 'i' ||
					ops[start].second[1] == 'e' ||
					ops[start].second[1] == 'w') count++;
				else if (ops[start].second[1] == '}') {
					count--;
					if (count == 0) break;
				}
			}
		}
	}
	else {
		start--;//��else
		hasElse = false;
	}
	int begin = if_ + 1, end = start;
	if (ifFlag) {
		if (hasElse)
			end = else_ - 1;
	}
	else {
		if (hasElse)
			begin = else_ + 1;
		else
			return;
	}
	while(begin<end)
		nextOps.push_back(ops[begin++]);
	iniStack(1);
	run(nextOps);
	desStack();

}
void CPU::WHILE(const vector<pair<int,string>>&ops, int &start) {
	//while ���������ֿ�
	//part 1 ()��Ĳ��� + r��
	//part 2 {}��Ĳ���
	// ��������Ӧ���� W part1 w part2 }
	//�����ڵ�ǰջ ������ջ ִ����� ȡջ��ֵ���жϾ����Ƿ�ִ��part 2
	vector<pair<int, string>> part1;
	vector<pair<int, string>> part2;
	start++;
	while (start < ops.size() && !(ops[start].second[0] == '0' && ops[start].second[1] == 'W'))
		part1.push_back(pair<int,string>(start,ops[start++].second));
	string* name = nullptr;
	if (ops[start].second.length() != 2)
		name = &ops[start].second.substr(2);

	start++;
	int count = 1;
	for (; start < ops.size(); start++) {
		if (ops[start].second[0] == '0') {
			if (ops[start].second[1] == 'i' || 
				ops[start].second[1] == 'e' ||
				ops[start].second[1] == 'w') count++;
			else if (ops[start].second[1] == '}') {
				count--;
				if (count == 0) break;
			}
		}
		part2.push_back(pair<int,string>(start,ops[start].second));
	}
	if (start >= ops.size())
		throw Other_E("while Ƕ���쳣��");
	bool whileFlag = true;
	while (true) {
		iniStack(3);
		run(part1);
		if (name == nullptr)
			whileFlag = Bool(popStack());
		else
			whileFlag = Bool(getVar(*name));
		desStack();
		if (whileFlag) {
			iniStack(3);
			run(part2);
			desStack();
		}
		else break;
	}
}

int CPU::firstInte(const string& op) {
	bool in = false;
	for (int i = 1; i < op.length(); i++) {
		if (in) {
			if (op[i] == '\"' && op[i - 1] != '\\')
				in = false;
		}
		else if (op[i] == '\"')
			in = true;
		else if (op[i] == ',') return i;
	}
	return -1;
}

void CPU::run(const vector<pair<int,string>>& ops) {
	for (int i = 0,layer = layerNum - 1; i < ops.size(); i++) {
		try {

			if (returnFlag){
				if (Scopes[layer].level%2==0)
					returnFlag = false;
				break;
			}
			Wait(ops[i].first);
			if (ops[i].second[0] == '0') {
				switch(ops[i].second[1])
				{
				case 'i':
					IF(ops, i);
					break;
				case 'w':
					WHILE(ops, i);
					break;
				case 'r':
					ret.type = 'v';
					returnFlag = true;
					break;
				case '}':
					throw Other_E("�в���ƥ���{��}");
					break;
				case 'W':
					throw Other_E("while ���ִ����쳣.");
					break;
				case 'e':
					throw Runtime_E("�в���ƥ�䵽if��else");
				default:
					throw Other_E(ops[i].first,string("����ʶ��Ĳ�����")+ops[i].second);
				}
			}
			else
				run(ops[i].second);
		}
		catch (Simple_E e) {
			throw Runtime_E(i,e);
		}
		catch (Compile_E e) {
			if (e.line == -1)
				e.line = i;
			throw e;
		}
		catch (Runtime_E e) {
			if (e.line == -1)
				e.line = i;
			throw e;
		}
		catch (Other_E e) {
			if (e.line == -1)
				e.line = i;
			throw e;
		}
		catch (Exe_E e) {
			throw e;
		}
		catch (...) {
			throw Exe_E("��������ȫδ֪��bug");
		}
	}
}
void CPU::assVar(Var& v1, const Var & v2) {
	InitCheck(v2);
	if (v1.type == 'i') v1 = vInt(v2);
	else if (v1.type == 'r') v1 = vFloat(v2);
	else v1 = vString(v2);
	v1.init = true;
}

void CPU::OpWith2(const string& op) {
	stack<Var>& stk = Stacks[layerNum - 1];
	int i = firstInte(op); 
	Var v2 = getVar(op.substr(i + 1));
	if (op[0] == '=')//��ֵ���⴦��
	{
		assVar(getVar_(op.substr(1, i - 1)), v2);
		return;
	}
	Var v1 = getVar(op.substr(1, i - 1));
	switch (op[0])
	{
	case '+':// +
		stk.push(Plus(v1, v2));
		break;
	case '-':// -
		stk.push(Minus(v1, v2));
		break;
	case '*':// *
		stk.push(Mul(v1, v2));
		break;
	case '/':// /
		stk.push(Div(v1, v2));
		break;
	case '%': // %
		stk.push(Mod(v1, v2));
		break;
	case '$': // $
		stk.push(Connect(v1, v2));
		break;
	case '#': // #
		stk.push(Delete(v1, v2));
		break;
	case ')': // >=
		stk.push(GEqual(v1, v2));
		break;
	case '<': // <
		stk.push(Less(v1, v2));
		break;
	case '(': // <=
		stk.push(LEqual(v1, v2));
		break;
	case '>': // >
		stk.push(Greater(v1, v2));
		break;
	case 'E': // ==
		stk.push(Equal(v1, v2));
		break;
	case 'N': // ��=
		stk.push(NEqual(v1, v2));
		break;
	case '&': // &&
		stk.push(And(v1, v2));
		break;
	case '|': // ||
		stk.push(Or(v1, v2));
		break;
	case 'O': //
		Out(v1, v2);
		break;
	default:
		throw Other_E(string("����ʶ��Ĳ�����"+op));
	}

}
void CPU::OpWith1(const string& op) {
	//�����ر���
	if (op[1] == 'I') {
		In(getVar_(op.substr(2)));
		return;
	}
	Var v = getVar(op.substr(2));
	stack<Var>& stk = Stacks[layerNum - 1];
	static int i = 0;
	switch (op[1])
	{
	case 'z':
		Pos(v);
		stk.push(v);
		break;
	case 'f':
		stk.push(Neg(v));
		break;
	case '!':
		stk.push(Not(v));
		break;

	case 'O':
		Out(v);
		break;
	case 'r':			
		ret = v;
		returnFlag = true;
		break;
	case 'p':
		stk.push(v);
	default:
		throw Other_E(string("����ʶ��Ĳ���:")+op);
		break;
	}
}
void CPU::OpWithq(const string&op) {
	switch (op[1])
	{
	case '@':
		throw Runtime_E("������������������");
		break;
	case '_':
		decVar(op.substr(3), op[2]);
		break;
	case ':':
		callFunc(op);
		break;
	case ';':
		pushCall(op);
		break;
	default:
		throw Other_E(string("����ʶ��Ĳ���:") + op);
		break;
	}
}

void In(Var& v) {
	if (v.type == 'i')
		cin >> v.var.Int;
	else if (v.type == 'r')
		cin >> v.var.Real;
	else
		cin >> *v.var.str_p;
	v.init = true;

}
void Out(const Var& v) {
	cout<<String(v);
}
void Out(const Var& v1, const Var& v2) {
	if (isNum(v1)) {
		for (int i = 0; i < Int(v1); i++)
			Out(v2);
	}
	else {
		Out(v1);
		Out(v2);
	}
}
void CPU::run(const string&op) {
	stack<Var>& stk = Stacks[layerNum - 1];
	switch (op[0])
	{
	case '1':
		OpWith1(op);
		break;
	case 'q':
		OpWithq(op);
		break;
	case '0':
		throw Exe_E("if-else,while,����,����Ƕ�׳����쳣");
		break;
	default:
		OpWith2(op);
		break;
	}
}
void CPU::clearWait() {
	wait.clear();
}
void CPU::delWait(int line) {
	auto it = wait.find(line);
	if (it == wait.end())
		throw Simple_E("����ɾ�������ڵĶϵ�.");
	else
		wait.erase(it);
}
void CPU::addWait(int line) {
	auto it = wait.find(line);
	if (it == wait.end())
		wait.insert(line);
	else
		throw Simple_E("�����ظ����ͬһ���ϵ�.");
}
void CPU::nextLine() {
	tempWait = nowLine + 1;
	runnable = true;
}
void CPU::nextPoint() {
	runnable = true;
}
Var* CPU::getValue(const string& name,int layer) {
	if (layer >= layerNum)
		throw Simple_E("�ṩ�˴����layer.");
	layer = findVar(name, layer);
	if (layer == -1)
		return nullptr;
	else
		return &Scopes[layer].vars.find(name)->second;
}
int CPU::getMaxLayer() {
	return layerNum - 1;
}
int CPU::findVar(const string& name) {
	return findVar(name, layerNum - 1);
}
int CPU::findVar(const string& name,int layer){
	int ori_layer = layer;
	map<string, Var>::iterator it;

	do{
		it = Scopes[layer].vars.find(name);
		if (it != Scopes[layer].vars.end())
			return layer;
		else if (Scopes[layer].level & 1)
			layer--;
		else
			return -1;
	} while (true);
}
void CPU::Wait(int line) {
	if ((line == tempWait) || (wait.find(line) != wait.end())) {
		while (!runnable) _sleep(500);
		runnable = false;
	}
	else;
}