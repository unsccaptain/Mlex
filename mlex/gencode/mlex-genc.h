#pragma once

#include "../dfa.h"

namespace mlex {

	class MlexCodeGen_C {

	private:
		MlexDfa& _dfa;

		string genHeader() {
			string s;
			s += "\n#include <stdlib.h>\n";
			s += "\n#include <stdbool.h>\n";
			return s;
		}

		string genStateMetrix() {
			string s;

			map<uint32_t, shared_ptr<MlexDfaSimpleState>> stateMap = _dfa.getDfaSimpleStateMap();
			vector<char> inputTable = _dfa.getInputTable();

			s += "\n";
			s += "#define state_count " + to_string(stateMap.size()) + "\n";
			s += "#define input_count " + to_string(inputTable.size()) + "\n";
			s += "#define start_index " + to_string(_dfa._sStartState->getStateId()) + "\n\n";
			s += "char* mlex_char,mlex_next;" + string("\n\n");
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

			s += "\n";
			s += "int mlex_read(char* s){" + string("\n");
			s += "\tmlex_char = s;\n";
			s += "\tint s_idx = 0;" + string("\n");
			s += "\tint c_idx = mlex_input_map[s[s_idx]];" + string("\n");
			s += "\tint end_state, next = start_index, *next_vt = 0;" + string("\n");
			s += "\twhile ((c_idx != -1 || mlex_input_map[1] != -1)) {" + string("\n");
			s += "\t\tif (c_idx == -1) {" + string("\n");
			s += "\t\t\tc_idx = 0;" + string("\n");
			s += "\t\t}" + string("\n");
			s += "\t\tend_state = next;" + string("\n");
			s += "\t\tnext = mlex_state_metrix[next][c_idx];" + string("\n");
			s += "\t\tif (next == -1) {" + string("\n");
			s += "\t\t\tbreak;" + string("\n");
			s += "\t\t}" + string("\n");
			s += "\t\tnext_vt = mlex_state_metrix[next];" + string("\n");
			s += "\t\tc_idx = mlex_input_map[s[++s_idx]];" + string("\n");
			s += "\t\tif (s[s_idx] == 0){" + string("\n");
			s += "\t\t\tbreak;" + string("\n");
			s += "\t\t}" + string("\n");
			s += "\t}\n";
			s += "\tif (!(next_vt != 0 && next_vt[input_count] == 1))" + string("\n");
			s += "\t\treturn 0;" + string("\n");
			s += "\tmlex_next = s + s_idx;" + string("\n");
			s += "\tswitch(end_state){" + string("\n");
			
			for (auto iter : _dfa.getDfaSimpleStateMap()) {
				if (!iter.second->_final) {
					continue;
				}
				s += "\tcase " + to_string(iter.first) + string(":\n");
				s += "\t\t" + iter.second->_oldre._genCode + string("\n");
				s += "\t\tbreak;\n";
			}

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