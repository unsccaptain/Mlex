#pragma once

#include <list>
#include "nfa.h"

namespace mlex {

	class MlexDfaState {
	private:
		map<char, shared_ptr<MlexDfaState>> _moveMap;
		vector<shared_ptr<MlexNfaState>> _nfaStates;
		uint32_t _nfaStatesSum;
		uint32_t _stateId;
		

	public:
		bool _final;
		
		/**
		 * 构造一个DFA状态
		 */
		MlexDfaState() {
			_final = false;
			_stateId = _stateIdCounter++;
		}

		/**
		 * 构造一个DFA状态
		 * @param	nfaStates		原始NFA的状态集
		 */
		MlexDfaState(vector<shared_ptr<MlexNfaState>>nfaStates) :_nfaStates(move(nfaStates)) {
			_final = false;
			_stateId = _stateIdCounter++;
			_nfaStatesSum = 0;
			for (auto iter1 : _nfaStates) {
				_nfaStatesSum += iter1->getStateId();
			}
		}

		/**
		 * 获取状态ID
		 */
		uint32_t getStateId() {
			return _stateId;
		}

		uint32_t getNfaStatesSum() {
			return _nfaStatesSum;
		}

		/**
		 * 获取DFA中NFA状态集
		 */
		vector<shared_ptr<MlexNfaState>>& getNfaStates() {
			return _nfaStates;
		}

		/**
		 * 添加一条输入字符位Input的弧
		 * @param	Input		Input输入字符
		 * @param	State		转换后的DFA状态
		 */
		void addMove(char Input, shared_ptr<MlexDfaState> State) {
			pair<char, shared_ptr<MlexDfaState>> newMove(Input, shared_ptr<MlexDfaState>(State));
			_moveMap.emplace(newMove);
		}

		/**
		 * 获取输入字符对应的DFA状态
		 * @param	Input		Input输入字符
		 */
		shared_ptr<MlexDfaState> getMove(char Input) {
			//字符表中的输入
			if (_moveMap[Input]) {
				return _moveMap[Input];
			}
			//如果可以接受任意字符的话
			else {
				return _moveMap[-1];
			}
		}

		//判断两个dfa状态是否等价
		bool operator==(const MlexDfaState& Another) {

			if (_nfaStates.size() != Another._nfaStates.size()) {
				return false;
			}

			auto iter2 = begin(Another._nfaStates);
			for (auto iter1:_nfaStates) {
				if (iter1->getStateId() != (*iter2)->getStateId())
					return false;
				iter2++;
			}

			return true;
		}
	};

	class MlexDfa {

	private:
		MlexNfa _nfa;
		map<uint32_t,vector<shared_ptr<MlexDfaState>>> _dfaStates;
		shared_ptr<MlexDfaState> _startState;

	public:
		MlexDfa(MlexNfa Nfa) :_nfa(Nfa) {

		}

		vector<shared_ptr<MlexNfaState>>::iterator findEmplacePosition(vector<shared_ptr<MlexNfaState>>& v,shared_ptr<MlexNfaState>& dfa) {

			if (v.size() == 0) {
				return end(v);
			}

			if (v[0]->getStateId() > dfa->getStateId()) {
				return begin(v);
			}

			auto startState = begin(v);
			auto endState = end(v) - 1;
			decltype(startState) midState;// = startState + (endState - startState) / 2;

			while (startState <= endState) {

				midState = startState + (endState - startState) / 2;
				
				if (dfa->getStateId() > (*midState)->getStateId()) {
					startState = midState + 1;
				}
				else if (dfa->getStateId() < (*midState)->getStateId()) {
					endState = midState - 1;
				}
				else {
					return midState;
				}
			}

			return startState;
		}

		/**
		 * 求出一个dfa状态中所有nfa状态对于某个输入Input的ε-closure(move(Input))
		 * @param	Input		输入字符
		 * @param	OldVector	原始NFA状态集
		 * @param	Final		新状态集中是否包含终态
		 */
		vector<shared_ptr<MlexNfaState>> getInputDfaState(char Input, vector<shared_ptr<MlexNfaState>>& OldVector,bool& Final) {

			vector<shared_ptr<MlexNfaState>> newVector;
			newVector.reserve(OldVector.size() * 4);

			for (auto iter :OldVector) {
				//对每个元素，求出move(Input)的集合
				auto moveState = iter->getMove(Input);
				for (auto iter2 :moveState) {
					//再对每个move(Input)求ε-closure(move(Input))
					//这里返回的是引用，所以不要使用move
					auto emptyState = iter2->emptyClosure();
					for (auto iter3 :emptyState) {
						//如果有nfa的终态的话，则当前dfa也是终态
						if (iter3->_final)
							Final = true;
						newVector.emplace_back(iter3);
					}
					//move(Input)本身也需要入栈
					if (iter2->_final)
						Final = true;
					newVector.emplace_back(iter2);
				}
				
			}

			//排序
			sort(begin(newVector), end(newVector),
				[](const shared_ptr<MlexNfaState>& a, const shared_ptr<MlexNfaState>& b) { 
				return a->getStateId() < b->getStateId();
			});
			//去重
			newVector.erase(unique(begin(newVector), end(newVector), 
				[](const shared_ptr<MlexNfaState>& a, const shared_ptr<MlexNfaState>& b) {
				return a->getStateId() == b->getStateId();
			}), end(newVector));

			return newVector;
		}

		/**
		 * NFA装DFA
		 */
		void convert() {

			bool final = false;
			stack<shared_ptr<MlexDfaState>> new_states;

			//计算初始节点的ε-closure
			auto st_state = _nfa._main_diagram->_startState->emptyClosure();
			//排序
			sort(begin(st_state), end(st_state),
				[](const shared_ptr<MlexNfaState>& a, const shared_ptr<MlexNfaState>& b) {
				return a->getStateId() < b->getStateId();
			});
			//去重
			st_state.erase(unique(begin(st_state), end(st_state),
				[](const shared_ptr<MlexNfaState>& a, const shared_ptr<MlexNfaState>& b) {
				return a->getStateId() == b->getStateId();
			}), end(st_state));

			//将起始节点的ε-closure构成的dfa状态节点压入栈中
			auto newDfaState = shared_ptr<MlexDfaState>(new MlexDfaState(st_state));

			//保存起始DF状态，输入匹配从这里开始
			_startState = shared_ptr<MlexDfaState>(newDfaState);

			new_states.push(move(newDfaState));

			while (!new_states.empty()) {

				//弹出栈顶dfa状态节点
				auto instack_state = new_states.top();
				new_states.pop();

				//针对每个输入字符，求出其ε-closure(move(Input))集合
				for (auto iter : _nfa._char_tab) {

					//取得ε-closure(move(Input))集合
					st_state = move(getInputDfaState(iter, instack_state->getNfaStates(), final));
					if (st_state.size() == 0) {
						continue;
					}

					//利用集合构造新的dfa节点
					newDfaState = shared_ptr<MlexDfaState>(new MlexDfaState(st_state));
					//如果在nfa状态集中包含终态，则该dfa为终态
					if (final) {
						newDfaState->_final = true;
					}
					final = false;

					//用于确定新的DFA状态是否已经存在
					if (_dfaStates.count(newDfaState->getNfaStatesSum()) != 0) {
						vector<shared_ptr<MlexDfaState>>& v = _dfaStates[newDfaState->getNfaStatesSum()];
						auto find = find_if(begin(v), end(v), [newDfaState](const shared_ptr<MlexDfaState> One) {
							return (*One) == (*newDfaState);
						});
						//如果在dfa栈中不存在，则压入dfa栈
						if (find == end(v)) {
							instack_state->addMove(iter, newDfaState);
							new_states.emplace(newDfaState);
						}
						else {
							instack_state->addMove(iter, *find);
						}
					}
					else {
						instack_state->addMove(iter, newDfaState);
						new_states.emplace(newDfaState);
					}

					//否则销毁这个dfa节点
					
				}

				//如果DFA状态集中包含单个DFA的NFA状态集合之和和当前DFA状态的NFA状态集合之和相同的元素
				//这么设计是借助map内部高效的映射算法，来减少DFA状态比较的时间
				if( _dfaStates.count(instack_state->getNfaStatesSum())){
					vector<shared_ptr<MlexDfaState>>& v = _dfaStates[instack_state->getNfaStatesSum()];
					v.emplace_back(instack_state);
				}
				//如果不包含就新建一个
				else {
					vector<shared_ptr<MlexDfaState>> v;
					v.emplace_back(instack_state);
					_dfaStates.emplace(pair<uint32_t, vector<shared_ptr<MlexDfaState>>>(instack_state->getNfaStatesSum(), v));
				}
			}
		}

		/**
		 * DFA转简化DFA
		 */
		void simplify() {

		}

		/**
		 * 检验一个字符串是否匹配并返回原始正则表达式
		 */
		bool validateString(string& s,string& re) {
			size_t i = 0;
			shared_ptr<MlexDfaState> dfa_state = _startState->getMove(s[i]);
			if (!dfa_state)
				return false;
			while (i != s.length() - 1) {
				i++;
				dfa_state = dfa_state->getMove(s[i]);
				if (!dfa_state)
					return false;
			}
			if (!dfa_state->_final)
				return false;

			//由于之前是排过序的，这里肯定符合优先匹配原则
			for (auto iter : dfa_state->getNfaStates()) {
				if (iter->_final) {
					re = iter->_oldre;
					break;
				}
			}
			return true;
		}
	};


}