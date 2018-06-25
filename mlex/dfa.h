#pragma once

#include <list>
#include "nfa.h"

namespace mlex {

	static uint32_t _dfaStateCounter = 0;
	static uint32_t _dfaSimpleStateCounter = 0;

	//简化的DFA状态
	class MlexDfaSimpleState {
	private:
		uint32_t _stateId;
		map<char, shared_ptr<MlexDfaSimpleState>> _moveMap;

	public:
		bool _final;
		string _oldre;

		MlexDfaSimpleState() {
			_final = false;
			_stateId = _dfaSimpleStateCounter++;
		}

		uint32_t getStateId() {
			return _stateId;
		}

		void addMove(char Input, shared_ptr<MlexDfaSimpleState> State) {
			pair<char, shared_ptr<MlexDfaSimpleState>> newPair(Input, State);
			_moveMap.emplace(newPair);
		}

		shared_ptr<MlexDfaSimpleState> getMove(char Input) {
			return _moveMap[Input];
		}
	};

	//普通的DFA状态
	class MlexDfaState {
	private:
		map<char, shared_ptr<MlexDfaState>> _moveMap;
		vector<shared_ptr<MlexNfaState>> _nfaStates;
		uint32_t _nfaStatesSum;
		uint32_t _stateId;
		
	public:
		bool _final;
		string _oldre;
		shared_ptr<list<shared_ptr<MlexDfaState>>> _equalityGroup;

		/**
		 * 构造一个DFA状态
		 */
		MlexDfaState() {
			_final = false;
			_stateId = _dfaStateCounter++;
		}

		/**
		 * 构造一个DFA状态
		 * @param	nfaStates		原始NFA的状态集
		 */
		MlexDfaState(vector<shared_ptr<MlexNfaState>>nfaStates) :_nfaStates(move(nfaStates)) {
			_final = false;
			_stateId = _dfaStateCounter++;
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
		map<uint32_t,vector<shared_ptr<MlexDfaState>>> _dfaStatesOnIdSummary;
		map<uint32_t, shared_ptr<MlexDfaState>> _dfaStatesMap;
		map<uint32_t, shared_ptr<MlexDfaSimpleState>> _dfaSimpleStateMap;

	public:
		shared_ptr<MlexDfaState> _startState;
		shared_ptr<MlexDfaSimpleState> _sStartState;

		MlexDfa(MlexNfa Nfa) :_nfa(Nfa) {
		}

		vector<char>& getInputTable() {
			return _nfa._char_tab;
		}

		map<uint32_t, shared_ptr<MlexDfaState>>& getDfaStatesMap() {
			return _dfaStatesMap;
		}

		map<uint32_t, shared_ptr<MlexDfaSimpleState>>& getDfaSimpleStateMap() {
			return _dfaSimpleStateMap;
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
					if (_dfaStatesOnIdSummary.count(newDfaState->getNfaStatesSum()) != 0) {
						vector<shared_ptr<MlexDfaState>>& v = _dfaStatesOnIdSummary[newDfaState->getNfaStatesSum()];
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
							_dfaStateCounter--;
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
				if(_dfaStatesOnIdSummary.count(instack_state->getNfaStatesSum())){
					vector<shared_ptr<MlexDfaState>>& v = _dfaStatesOnIdSummary[instack_state->getNfaStatesSum()];
					v.emplace_back(instack_state);
				}
				//如果不包含就新建一个
				else {
					vector<shared_ptr<MlexDfaState>> v;
					v.emplace_back(instack_state);
					_dfaStatesOnIdSummary.emplace(pair<uint32_t, vector<shared_ptr<MlexDfaState>>>(instack_state->getNfaStatesSum(), v));
				}

				//保存DFA对应的正则表达式
				for (auto iter : instack_state->getNfaStates()) {
					if (iter->_final) {
						instack_state->_oldre = iter->_oldre;
						break;
					}
				}
				
				_dfaStatesMap.emplace(pair<uint32_t, shared_ptr<MlexDfaState>>(instack_state->getStateId(), instack_state));
			}

			//这里NFA中所有节点都会被销毁
			_dfaStatesOnIdSummary.clear();
			for (auto iter : _dfaStatesMap) {
				iter.second->getNfaStates().clear();
			}
		}

		/**
		 * DFA转简化DFA
		 * @param	deep		深度简化
		 * 
		 * 由于需要保留原始正则表达式的信息，所以在简化终态组时，不仅需要符合蔓延性条件
		 * 还需要将状态对应的正则表达式不同的状态进行分割
		 * 如果deep位true，则状态最少，则只能进行字符串匹配
		 * 如果deep为false，则状态会多一些，但可以用于代码生成
		 */
		void simplify(bool deep) {

			stack<shared_ptr<list<shared_ptr<MlexDfaState>>>> _workGroup;
			vector<shared_ptr<list<shared_ptr<MlexDfaState>>>> _finalGroup;

			//首先分割成两个组，一个是终态，一个是非终态
			auto _group1 = shared_ptr<list<shared_ptr<MlexDfaState>>>(new list<shared_ptr<MlexDfaState>>);
			auto _group2 = shared_ptr<list<shared_ptr<MlexDfaState>>>(new list<shared_ptr<MlexDfaState>>);
			for (auto iter : _dfaStatesMap) {
				if (!iter.second->_final) {
					iter.second->_equalityGroup = shared_ptr<list<shared_ptr<MlexDfaState>>>(_group1);
					_group1->emplace_back(iter.second);
				}
				else {
					iter.second->_equalityGroup = shared_ptr<list<shared_ptr<MlexDfaState>>>(_group2);
					_group2->emplace_back(iter.second);
				}
			}
			_workGroup.emplace(_group1);
			_workGroup.emplace(_group2);

			while (!_workGroup.empty()) {
				bool not_final = false;
				auto top = _workGroup.top();
				_workGroup.pop();

				for (auto iter : _nfa._char_tab) {
					auto _tag_group = (top->front()->getMove(iter) ? top->front()->getMove(iter)->_equalityGroup.get() : nullptr);
					auto _tag_re = top->front()->_oldre;
					auto _group1 = shared_ptr<list<shared_ptr<MlexDfaState>>>(new list<shared_ptr<MlexDfaState>>);

					//将不等价的状态放到另一个新的组里
					top->remove_if([_tag_group,_tag_re,_group1,iter,deep](shared_ptr<MlexDfaState>& state) {
						if (!(
							(_tag_group == nullptr && state->getMove(iter) == nullptr) ||
								(state->getMove(iter) && (state->getMove(iter)->_equalityGroup.get() == _tag_group))
								)
							|| (!deep && state->_final && (state->_oldre.compare(_tag_re) != 0))
							) {
							state->_equalityGroup = shared_ptr<list<shared_ptr<MlexDfaState>>>(_group1);
							_group1->emplace_back(state);
							return true;
						}
						else {
							return false;
						}
					});

					if (_group1->size() != 0) {
						_workGroup.emplace(_group1);
					}
				}

				_finalGroup.emplace_back(top);
			}

			map<list<shared_ptr<MlexDfaState>>*, shared_ptr<MlexDfaSimpleState>> stateMap;

			for (auto iter : _finalGroup) {
				shared_ptr<MlexDfaSimpleState> sstate = shared_ptr<MlexDfaSimpleState>(new MlexDfaSimpleState());
				stateMap.emplace(pair<list<shared_ptr<MlexDfaState>>*, shared_ptr<MlexDfaSimpleState>>(iter.get(), move(sstate)));
			}

			for (auto iter : _finalGroup) {
				auto sstate = stateMap[iter.get()];
				//对于每个输入符号，依次剔除不等价的状态
				for (auto iter2 : *iter) {
					for (auto iter3 : _nfa._char_tab) {
						if (iter2->getMove(iter3)) {
							auto moveTag = stateMap[iter2->getMove(iter3)->_equalityGroup.get()];
							sstate->addMove(iter3, moveTag);
						}
					}
					if (iter2->getStateId() == 0) {
						_sStartState = sstate;
					}
					if (iter2->_final) {
						sstate->_final = true;
						sstate->_oldre = iter2->_oldre;
					}
				}
			}
		}

		/**
		 * 检验一个字符串是否匹配并返回原始正则表达式
		 */
		bool validateString(string& s,string& re) {
			size_t i = 0;
			shared_ptr<MlexDfaSimpleState> dfa_state = _sStartState->getMove(s[i]);
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

			re = dfa_state->_oldre;
			return true;
		}
	};


}