// mlex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "dfa.h"

#include <fstream>

int main()
{

	fstream fs("E:\\reg.lex",ios::in);

	char* test_strings[] = {
		"joker",
		"jokes",
		"jokex",
		"jccc",
		"Aa",
		"AbAcAd",
		"Aaaaa",
		"AbAcAAA",
		".3",
		"0.12",
		"0.123",
		"a.ddd",
		"acdq",
		"cds",
		"cdsdd"
	};

	if (fs.good()) {

		mlex::MlexRegexp regexp(fs);

		mlex::MlexNfa nfa(regexp);

		nfa.convert();

		mlex::MlexDfa dfa(nfa);

		dfa.convert();

		for (int i = 0;i < sizeof(test_strings) / sizeof(char*);i++) {
			string re;
			bool r = dfa.validateString(string(test_strings[i]), re);
			if (r) {
				printf("%s:匹配正确,表达式%s\n", test_strings[i], re.c_str());
			}
			else {
				printf("%s:匹配错误\n", test_strings[i]);
			}
		}
	}

    return 0;
}

