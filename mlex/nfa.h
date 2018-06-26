#pragma once

#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

#include "regexp.h"

namespace mlex {

	//状态计数器，用于确定状态的唯一ID
	static uint32_t _stateIdCounter = 0;

	//被推入工作栈中的正则表达式运算符
	enum class MlexReOpTypes {
		contact = 0,
		select,
		closure,
		lbracket,
		rbracket,
		opmax
	};

	enum class MlexNfaWorkEleTypes {
		StateDiagram = 0,
		Operator
	};

	/**
	 * 工作栈中的元素
	 * 包括：
	 *		状态图中类
	 *		运算符类
	 */
	class MlexNfaWorkElement {
	public:
		MlexNfaWorkEleTypes _eleType;

		MlexNfaWorkElement(MlexNfaWorkEleTypes Type) 
			:_eleType(Type) {
		}
	};

	/**
	 * 正则表达式运算符类
	 */
	class MlexNfaReOperator :public MlexNfaWorkElement {
		
	public:
		MlexReOpTypes _opType;
		uint16_t _num_1;
		uint16_t _num_2;

		/**
		 * 初始化连接和选择运算符
		 */
		MlexNfaReOperator(MlexReOpTypes Type) 
			:_opType(Type), MlexNfaWorkElement(MlexNfaWorkEleTypes::Operator) {
		}

		/**
		 * 初始化闭包运算符
		 * @param	MinTime			最少闭包次数
		 * @param	MaxTime			最多闭包次数
		 * 目前只支持[0,+∞),[1,+∞)和{0,1}次的闭包
		 * 因为不知道怎么把次数信息转换到DFA
		 */
		MlexNfaReOperator(uint16_t MinTime, uint16_t MaxTime) 
			:_opType(MlexReOpTypes::closure), _num_1(MinTime), _num_2(MaxTime), MlexNfaWorkElement(MlexNfaWorkEleTypes::Operator) {
		}
	};

	class MlexClosureCount {
	public:
		uint16_t min;
		uint16_t max;

		MlexClosureCount(uint16_t Min, uint16_t Max) 
			:min(Min), max(Max) {
		}
	};

	//定义一个NFA状态
	class MlexNfaState{
	
	private:
		//转态转换弧
		//multimap<char, shared_ptr<MlexNfaState>> _moveMap;
		//基于这样一个事实，所有带非空串的状态转换都只会有一条弧
		vector<shared_ptr<MlexNfaState>> _multiMap;
		pair<char, shared_ptr<MlexNfaState>> _singleMap;
		//状态ID
		uint32_t _stateId;
		//状态的ε-closure
		vector<shared_ptr<MlexNfaState>> _emptyClosure;

	public:
		//确定当前状态是否是NFA的终态
		bool _final;
		//当前状态接受任意输入字符
		MlexRegexpContext _oldre;

		/**
		 * 初始化一个NFA状态
		 */
		MlexNfaState() {
			_final = false;
			_stateId = _stateIdCounter++;
		}

		/**
		 * 获取NFA状态ID
		 */
		uint32_t getStateId() {
			return _stateId;
		}

		/**
		 * 添加一条弧，条件是输入Input
		 * @param	Input		输入字符
		 * @param	State		转换后的状态
		 */
		void addMove(char Input, shared_ptr<MlexNfaState> State) {
			pair<char, shared_ptr<MlexNfaState>> newMove(Input, shared_ptr<MlexNfaState>(State));
			if (Input != 0) {
				_singleMap = newMove;
			}
			else {
				_multiMap.emplace_back(newMove.second);
			}
		}

		/**
		 * 添加一条带闭包次数的弧
		 * @param	Input		输入字符
		 * @param	State		转换后的状态
		 * @param	CCount		闭包次数信息
		 */
		void addMove(char Input, shared_ptr<MlexNfaState> State, MlexClosureCount& CCount) {
			pair<char, shared_ptr<MlexNfaState>> newMove(Input, shared_ptr<MlexNfaState>(State));
			if (Input != 0) {
				_singleMap = newMove;
			}
			else {
				_multiMap.emplace_back(newMove.second);
			}
		}

		/**
		 * 获取输入Input的情况下所有的弧
		 * @param	Input		输入字符
		 */
		vector<shared_ptr<MlexNfaState>> getMove(char Input) {
			
			if (Input == 0) {
				return _multiMap;
			}
			else {
				vector<shared_ptr<MlexNfaState>> moveVector;
				if (_singleMap.second) {
					if ((_singleMap.first == Input) || (_singleMap.first == 1)) {
						moveVector.emplace_back(shared_ptr<MlexNfaState>(_singleMap.second));
					}
				}
				return moveVector;
			}
		}

		/**
		 * 获取状态的ε-closure
		 */
		vector<shared_ptr<MlexNfaState>>& emptyClosure() {

			//如果已经计算过了
			if (_emptyClosure.size() != 0) {
				return _emptyClosure;
			}

			vector<shared_ptr<MlexNfaState>> v_first = getMove(0);
			stack<shared_ptr<MlexNfaState>> s_prep;

			//将直接ε弧对应的边灌到stack中
			for (auto iter:v_first) {
				s_prep.emplace(iter);
			}
			//不断弹出es中的元素
			while (!s_prep.empty()) {
				//获取栈顶元素的所有ε弧对应的边
				shared_ptr<MlexNfaState> top = s_prep.top();
				s_prep.pop();

				v_first = top->getMove(0);
				_emptyClosure.emplace_back(top);

				for (auto iter:v_first) {
					s_prep.emplace(iter);
				}
			}
			return _emptyClosure;
		}
	};

	//定义一个NFA状态转换图
	class MlexNfaStateDiagram :public MlexNfaWorkElement {

	private:
		shared_ptr<MlexNfaState> _startState;
		shared_ptr<MlexNfaState> _endState;

	public:
		friend class MlexDfa;

		/**
		 * 构造NFA状态转换图
		 * @param	StartState		初态
		 * @param	EndState		终态
		 */
		MlexNfaStateDiagram(shared_ptr<MlexNfaState> StartState, shared_ptr<MlexNfaState> EndState) 
			:_startState(shared_ptr<MlexNfaState>(StartState)), _endState(shared_ptr<MlexNfaState>(EndState)), MlexNfaWorkElement(MlexNfaWorkEleTypes::StateDiagram) {
		}

		/**
		 * 获取初态
		 */
		shared_ptr<MlexNfaState> getStartState() {
			return _startState;
		}

		/**
		 * 获取终态
		 */
		shared_ptr<MlexNfaState> getEndState() {
			return _endState;
		}

		/**
		 * 进行状态图的连接运算
		 * @param	One			一端
		 * @param	Another		另一端
		 * @param	join_front	另一端在一段前面
		 * 
		 * @retval	新的状态图
		 */
		static shared_ptr<MlexNfaStateDiagram> contact(shared_ptr<MlexNfaStateDiagram> One, shared_ptr<MlexNfaStateDiagram> Another, bool join_front) {
			if (join_front) {
				//shared_ptr<MlexNfaState> myStartState = shared_ptr<MlexNfaState>(One->_startState);
				//One._startState = Another._startState;
				//One->_startState.swap(Another->_startState);
				//Another._endState.addMove(0, myStartState);
			}
			else {
				One->_endState->addMove(0, Another->_startState);
				One->_endState = Another->_endState;
			}

			return shared_ptr<MlexNfaStateDiagram>(new MlexNfaStateDiagram(One->_startState,One->_endState));
		}

		/**
		 * 进行状态图的选择运算
		 * @param	Branchs		选择运算的所有分支
		 *
		 * @retval	新的状态图
		 */
		static shared_ptr<MlexNfaStateDiagram> select(vector<shared_ptr<MlexNfaStateDiagram>>Branchs) {
			auto tmpStartState= shared_ptr<MlexNfaState>(new MlexNfaState());
			auto tmpEndState= shared_ptr<MlexNfaState>(new MlexNfaState());

			for (auto iter : Branchs) {
				tmpStartState->addMove(0, iter->_startState);
				iter->_endState->addMove(0, tmpEndState);
			}

			return shared_ptr<MlexNfaStateDiagram>(new MlexNfaStateDiagram(tmpStartState, tmpEndState));
		}

		/**
		 * 进行状态图的闭包运算
		 * @param	One			待进行运算的状态
		 * @param	CCount		闭包次数
		 *
		 * @retval	新的状态图
		 */
		static shared_ptr<MlexNfaStateDiagram> closure(shared_ptr<MlexNfaStateDiagram> One,MlexClosureCount& CCount) {
			auto tmpStartState = shared_ptr<MlexNfaState>(new MlexNfaState());
			auto tmpEndState = shared_ptr<MlexNfaState>(new MlexNfaState());


			tmpStartState->addMove(0, One->_startState);
			One->_endState->addMove(0, tmpEndState);

			if (CCount.max > 1)
				One->_endState->addMove(0, One->_startState);
			if (CCount.min == 0)
				tmpStartState->addMove(0, tmpEndState);

			return shared_ptr<MlexNfaStateDiagram>(new MlexNfaStateDiagram(tmpStartState, tmpEndState));
		}
	};

	class MlexNfa {

	private:
		//所有的正则表达式
		MlexRegexp& _regExps;
		//状态图集合
		vector<shared_ptr<MlexNfaStateDiagram>> _dirgramVector;

		/**
		 * 创建一个基本的状态图，即【初态】->Input->【终态】
		 * @param	Input		输入字符
		 */
		shared_ptr<MlexNfaStateDiagram> createBasicStateDiagram(char Input) {
			auto tmpStartState = shared_ptr<MlexNfaState>(new MlexNfaState());
			auto tmpEndState = shared_ptr<MlexNfaState>(new MlexNfaState());
	
			tmpStartState->addMove(Input, tmpEndState);

			return shared_ptr<MlexNfaStateDiagram>(new MlexNfaStateDiagram(tmpStartState, tmpEndState));
		}

		vector<char> readSelect(string s, size_t& pos) {
			vector<char> cv;
			char end, start;

			while (s[pos] != ']') {
				start = s[pos];
				pos++;
				
				if (s[pos] == '-') {
					pos++;
					end = s[pos];
					pos++;
				}
				else {
					end = start;
				}

				//匹配失败
				if ((!isLetter(start) && !isNumberic(start)) ||
					(!isLetter(end) && !isNumberic(end))) {
					return cv;
				}

				for (char i = start;i <= end;i++) {
					cv.emplace_back(i);
				}

				//去重
			}

			return cv;
		}

		/**
		 * 处理链接运算
		 * @param	stateDiagrams		状态图堆栈
		 */
		bool handlerContact(stack<shared_ptr<MlexNfaStateDiagram>>& stateDiagrams) {
			shared_ptr<MlexNfaStateDiagram> sd1 = move(stateDiagrams.top());
			if (sd1 == false) {
				return false;
			}
			stateDiagrams.pop();
			shared_ptr<MlexNfaStateDiagram> sd2 = move(stateDiagrams.top());
			if (sd2 == false) {
				return false;
			}
			stateDiagrams.pop();
			shared_ptr<MlexNfaStateDiagram> sd = move(MlexNfaStateDiagram::contact(sd2, sd1, false));
			stateDiagrams.emplace(sd);
			return true;
		}

		/**
		 * 处理选择运算
		 * @param	stateDiagrams		状态图堆栈
		 * @param	selects				选择分支
		 */
		bool handlerSelect(stack<shared_ptr<MlexNfaStateDiagram>>& stateDiagrams, vector<shared_ptr<MlexNfaStateDiagram>>& selects) {
			if (selects.size() == 0)
				return false;
			shared_ptr<MlexNfaStateDiagram> newSd = move(MlexNfaStateDiagram::select(selects));
			selects.clear();
			stateDiagrams.emplace(newSd);
			return true;
		}

		/**
		 * 处理闭包运算
		 * @param	stateDiagrams		状态图堆栈
		 * @param	op					运算符，需要其中的次数信息
		 */
		bool handlerClosure(stack<shared_ptr<MlexNfaStateDiagram>>& stateDiagrams,shared_ptr<MlexNfaReOperator>& op) {
			shared_ptr<MlexNfaStateDiagram> sd = move(stateDiagrams.top());
			if (sd == false) {
				return false;
			}
			stateDiagrams.pop();
			MlexClosureCount count = MlexClosureCount(op->_num_1, op->_num_2);
			shared_ptr<MlexNfaStateDiagram> newSd = move(MlexNfaStateDiagram::closure(sd, count));
			stateDiagrams.emplace(newSd);
			return true;
		}

		/**
		 * 弹出优先级大于等于op的所有运算符
		 * @param	op			当前运算符
		 * @param	opStack		运算符栈
		 * @param	workStack	工作栈
		 */
		void pushOperator(shared_ptr<MlexNfaReOperator>op, stack<shared_ptr<MlexNfaReOperator>>& opStack, vector<shared_ptr<MlexNfaWorkElement>>& workStack) {
			//优先级最高，弹出所有闭包
			if (op->_opType == MlexReOpTypes::closure) {
				while (!opStack.empty()) {
					auto top_op = opStack.top();
					if (top_op->_opType < MlexReOpTypes::closure || top_op->_opType == MlexReOpTypes::lbracket)
						break;

					opStack.pop();
					workStack.emplace_back(top_op);
				}
				opStack.emplace(op);
				return;
			}

			//连接预算，只需要弹出其余的连接符
			if (op->_opType == MlexReOpTypes::contact) {
				shared_ptr<MlexNfaReOperator> top_op;
				while (!opStack.empty()) {
					top_op = opStack.top();
					if (top_op->_opType < MlexReOpTypes::contact || top_op->_opType == MlexReOpTypes::lbracket)
						break;

					opStack.pop();
					workStack.emplace_back(top_op);
				}
				opStack.emplace(op);
				return;
			}

			//选择运算，弹出其余的运算符
			if (op->_opType == MlexReOpTypes::select) {
				shared_ptr<MlexNfaReOperator> top_op;
				while (!opStack.empty()) {
					top_op = opStack.top();
					if (top_op->_opType < MlexReOpTypes::select || top_op->_opType == MlexReOpTypes::lbracket)
						break;

					opStack.pop();
					workStack.emplace_back(top_op);
				}
				opStack.emplace(op);
				return;
			}

			if (op->_opType == MlexReOpTypes::lbracket) {
				opStack.emplace(op);
				return;
			}

			if (op->_opType == MlexReOpTypes::rbracket) {
				shared_ptr<MlexNfaReOperator> top_op;
				while (!opStack.empty()) {
					top_op = opStack.top();
					if (top_op->_opType == MlexReOpTypes::lbracket)
						break;

					opStack.pop();
					workStack.emplace_back(top_op);
				}
				if (opStack.size() == 0 || opStack.top()->_opType != MlexReOpTypes::lbracket) {
					throw(0);
				}
				opStack.pop();

				return;
			}
		}

	public:
		//NFA的字母表
		vector<char> _char_tab;
		//NFA的最终转态转换图
		shared_ptr<MlexNfaStateDiagram> _main_diagram;

		MlexNfa(MlexRegexp& re) 
			:_regExps(re) {
		}

		/**
		 * 将一条正则表达式转换为一个NFA
		 * @param	re		正则表达式
		 *
		 * @retval	单条正则表达式最终形成的NFA状态图
		 * 
		 * 首先将正则表达式整理为后缀表达式
		 * 然后将后缀表达式组装为NFA状态图
		 */
		shared_ptr<MlexNfaStateDiagram> convertReToNfa(string& re) {
			stack<shared_ptr<MlexNfaReOperator>> opStack;
			vector<shared_ptr<MlexNfaWorkElement>> workStack;

			//循环遍历正则表达式并构造表达式的后缀形式
			for (size_t i = 0;i < re.length();i++) {

				if (isLetter(re[i]) || isNumberic(re[i])) {

					//向字母表添加元素
					_char_tab.emplace_back(re[i]);

					//单个元素的状态转换图：【state1】->c->【state2】
					workStack.emplace_back(createBasicStateDiagram(re[i]));

					//压入连接运算符
					goto CONTACT;
				}

				switch (re[i])
				{
				//重复0-n次的闭包
				case '*':
				{
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(0, 0xFFFF)), opStack, workStack);
					break;
				}
				//重复1-n次的闭包
				case '+':
				{
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(1, 0xFFFF)), opStack, workStack);
					break;
				}
				//重复0-1次的闭包
				case '?':
				{
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(0, 1)), opStack, workStack);
					break;
				}
				//选择运算
				case '|':
				{
					opStack.pop();
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(MlexReOpTypes::select)), opStack, workStack);
					continue;
				}
				//选择运算，从字符集中选择单个字符
				case '[':
				{
					i++;
					auto c = readSelect(re, i);
					vector<shared_ptr<MlexNfaStateDiagram>> selected;
					for (auto iter : c) {

						//向字母表添加元素
						_char_tab.emplace_back(iter);
						auto start = shared_ptr<MlexNfaState>(new MlexNfaState());
						auto end = shared_ptr<MlexNfaState>(new MlexNfaState());
						start->addMove(iter, end);
						shared_ptr<MlexNfaStateDiagram> sd = shared_ptr<MlexNfaStateDiagram>(new MlexNfaStateDiagram(start, end));
						selected.emplace_back(sd);
					}
					auto newDiagram = move(MlexNfaStateDiagram::select(selected));
					workStack.emplace_back(newDiagram);
					break;
				}
				//左括号，直接入栈
				case '(':
				{
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(MlexReOpTypes::lbracket)), opStack, workStack);
					continue;
				}
				//右括号，需要优先弹出多于的连接运算符
				case ')':
				{
					if (opStack.top()->_opType != MlexReOpTypes::contact) {
						throw(string(re + ":)前含有错误运算符。"));
					}
					opStack.pop();
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(MlexReOpTypes::rbracket)), opStack, workStack);
					break;
				}
				//转义运算符
				case '\\':
				{
					if (!isValidChar(re[i + 1])) {
						throw(string(re + ":包含非法字符。"));
					}
					i++;
					_char_tab.emplace_back(re[i]);
					workStack.emplace_back(createBasicStateDiagram(re[i]));
					break;
				}
				//任意字符匹配运算符
				case '.':
				{
					//如果接受任意字符，则用-1标识
					//但NFA的-1和DFA的-1不是同一个概念
					//NFA中-1标识任意合法字符，即(32,127)
					//DNF中表示除了字母表中的其他任意合法字符
					//否则DFA会出现多重映射
					_char_tab.emplace_back(1);
					workStack.emplace_back(createBasicStateDiagram(1));
					break;
				}
				default:
					break;
				}

				//除了闭包，其余符号的后面都需要追加一个隐性的连接运算符
			CONTACT:
				if (!isClosure(re[i + 1])) {
					pushOperator(shared_ptr<MlexNfaReOperator>(new MlexNfaReOperator(MlexReOpTypes::contact)), opStack, workStack);
				}
			}

			if (opStack.top()->_opType == MlexReOpTypes::contact)
				opStack.pop();

			//将符号栈中的多于元素弹出
			while (!opStack.empty()) {
				workStack.emplace_back(opStack.top());
				opStack.pop();
			}

			stack<shared_ptr<MlexNfaStateDiagram>> valStack;
			vector<shared_ptr<MlexNfaStateDiagram>> effSelect;
			shared_ptr<MlexNfaWorkElement> ele;

			//遍历后缀式并构造NFA
			for (auto iter : workStack)
			{
				ele = iter;

				if (ele->_eleType == MlexNfaWorkEleTypes::StateDiagram) {
					valStack.emplace(static_pointer_cast<MlexNfaStateDiagram>(ele));
				}
				else if (ele->_eleType == MlexNfaWorkEleTypes::Operator) {

					shared_ptr<MlexNfaReOperator> opt;
					opt = move(static_pointer_cast<MlexNfaReOperator>(ele));

					switch (opt->_opType)
					{
					case MlexReOpTypes::closure:
						handlerClosure(valStack, opt);
						break;
					case MlexReOpTypes::contact:
						handlerContact(valStack);
						break;
					case MlexReOpTypes::select:
						effSelect.emplace_back(valStack.top());
						valStack.pop();
						effSelect.emplace_back(valStack.top());
						valStack.pop();
						handlerSelect(valStack, effSelect);
						break;
					default:
						break;
					}
				}
			}

			//返回最终的状态图
			return valStack.top();
		}

		/**
		 * 依次将所有正则表达式转换为NFA并最终用选择运算连接所有的NFA
		 */
		shared_ptr<MlexNfaStateDiagram> convert() {
			
			vector<shared_ptr<MlexNfaStateDiagram>> reSds;

			for (size_t i = 0;i < _regExps.getReCount();i++) {

				MlexRegexpContext& re = _regExps.getRegExp(i);
				auto diag = convertReToNfa(re._regExp);
				diag->getEndState()->_oldre = re;
				diag->getEndState()->_final = true;
				reSds.emplace_back(diag);
			}

			//去除字母表中重复元素
			sort(_char_tab.begin(), _char_tab.end());
			_char_tab.erase(unique(_char_tab.begin(), _char_tab.end()), _char_tab.end());

			//所有re生成的NFA进行选择运算
			_main_diagram = move(MlexNfaStateDiagram::select(reSds));
			//_main_diagram->getEndState()->_final = true;

			return _main_diagram;
		}

	};
}