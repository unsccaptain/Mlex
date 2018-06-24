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

			map<uint32_t, shared_ptr<MlexDfaState>> stateMap = _dfa.getDfaStatesMap();
			vector<char> inputTable = _dfa.getInputTable();

			s += "\n";
			s += "#define state_count " + to_string(stateMap.size()) + "\n";
			s += "#define input_count " + to_string(inputTable.size()) + "\n";

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
			s += "bool mlex_read(char* s){" + string("\n");
			s += "\tint s_idx = 0;" + string("\n");
			s += "\tint c_idx = mlex_input_map[s[s_idx]];" + string("\n");
			s += "\tint next = 0, *next_vt = 0;" + string("\n");
			s += "\twhile (c_idx != -1 && next != -1) {" + string("\n");
			s += "\t\tnext = mlex_state_metrix[next][c_idx];" + string("\n");
			s += "\t\tif (next == -1) {" + string("\n");
			s += "\t\t\tbreak;" + string("\n");
			s += "\t\t}" + string("\n");
			s += "\t\tnext_vt = mlex_state_metrix[next];" + string("\n");
			s += "\t\tc_idx = mlex_input_map[s[++s_idx]];" + string("\n");
			s += "\t}\n";
			s += "\tif (s[s_idx] != 0){" + string("\n");
			s += "\t\treturn 0;" + string("\n");
			s += "\t}\n";
			s += "\tif (next_vt != 0 && next_vt[input_count] == 1)" + string("\n");
			s += "\t\treturn 1;" + string("\n");
			s += "\telse" + string("\n");
			s += "\t\treturn 0;" + string("\n");
			s += "}\n";

			return s;
		}

	public:
		MlexCodeGen_C(MlexDfa& dfa) :_dfa(dfa) {

		}

		string gencode() {

			return genHeader() + genStateMetrix() + genMainFunc();
		}

	};

};