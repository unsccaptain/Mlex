#pragma once
#pragma once

#include "../dfa.h"

namespace mlex {

	class MlexCodeGen_C {

	private:
		MlexDfa& _dfa;

		string genHeader() {
			string s;
			s += "\n#include <stdlib.h>";
			s += "\n#include <string.h>\n";
			return s;
		}

		string genStateMetrix() {
			string s;

			map<uint32_t, shared_ptr<MlexDfaSimpleState>> stateMap = _dfa.getDfaSimpleStateMap();
			vector<char> inputTable = _dfa.getInputTable();

			s += "#define state_count " + to_string(stateMap.size()) + "\n";
			s += "#define input_count " + to_string(inputTable.size()) + "\n";
			s += "#define start_index " + to_string(_dfa._sStartState->getStateId()) + "\n\n";
			s += "const char* mlex_char,*mlex_next;" + string("\n\n");
			s += "char mlex_token[256];\n\n";
			s += "int mlex_input_map[128] = {" + string("\n");

			int counter = 0;
			for (int i = 0;i<128;i++) {
				if (inputTable[counter] == i) {
					s += to_string(counter++);
				}
				else {
					s += to_string(-1);
				}
				s += ",";
			}
			s.pop_back();
			s += "\n};\n\n";

			s += "int mlex_state_metrix[state_count][input_count + 1] = {\n";

			for (auto iter1 : stateMap) {
				s += "{";
				for (auto iter2 : inputTable) {
					if (iter1.second->getMove(iter2))
						s += to_string(iter1.second->getMove(iter2)->getStateId()) + ",";
					else
						s += "-1,";
				}
				if (iter1.second->_final) {
					s += "1";
				}
				else {
					s += "0";
				}
				s += "},\n";
			}
			s.pop_back();
			s.pop_back();
			s += "\n};\n";

			return s;
		}

		string genMainFunc() {
			string s;

			s = R"(
			
JsTokenType mlex_read_token(const char* s){

	mlex_char = s;
	//字符串的索引
	int s_idx = 0;
	//字符在输入表中索引
	int c_idx = mlex_input_map[s[s_idx]];
	int end_state, next = start_index, *next_vt = 0;
	memset(mlex_token, 0 , 256);

	while ((c_idx != -1 || mlex_input_map[1] != -1)) {

		//如果是-1，说明存在任意字符的规则
		if (c_idx == -1) {
			c_idx = 0;
		}

		//保存最后一个状态
		end_state = next;

		//获取下一个状态
		next = mlex_state_metrix[next][c_idx];
		if (next == -1) {
			break;
		}

		//保存下一个状态的状态数组
		next_vt = mlex_state_metrix[next];

		c_idx = mlex_input_map[s[++s_idx]];
		if (s[s_idx] == 0){
			break;
		}
	}

	if (!(next_vt != 0 && next_vt[input_count] == 1)){
		return JsTokenType::Unknown;
	}

	mlex_next = s + s_idx;
	memcpy(mlex_token, s, s_idx);

	switch(end_state){
)";

			for (auto iter : _dfa.getDfaSimpleStateMap()) {
				if (!iter.second->_final) {
					continue;
				}
				s += "\tcase " + to_string(iter.first) + string(":\n");
				s += "\t\t" + iter.second->_oldre._genCode + string("\n");
				s += "\t\tbreak;\n";
			}

			s += R"(
	default:
		return JsTokenType::Unknown;

)";

			s += "\t}\n";

			s += "}\n";

			return s;
		}

	public:
		MlexCodeGen_C(MlexDfa& dfa) :_dfa(dfa) {

		}

		string gencode() {

			return genHeader() + _dfa._nfa._regExps._define_part + genStateMetrix() + genMainFunc();
		}

	};

};