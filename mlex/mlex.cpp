// mlex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "gencode\mlex-genc.h"
#include "docopt\docopt.h"

//mlex --lex LEX_FILE --matchfile FILE_NAME
//--matchfile FILE_NAME	和指定文件匹配。

static const char USAGE[] =
R"(mlex

    Usage:
      mlex --lex LEX_FILE --genlang LANG [--genre|--genlex] --output OUTPUT
	  
    Options:
		-h --help				显示帮助。
		--lex LEX_FILE			LEX规则文件。
		--genlang LANG			指定生成语言。[default:C]
		--genre					生成正则匹配代码。
		--genlex				生成词法分析代码。
		--output OUTPUT			输出文件。
)";

#include <fstream>

int main(int argc, const char** argv)
{

	std::map<std::string, docopt::value> args
		= docopt::docopt(USAGE,
		{ argv + 1, argv + argc },
			true,
			"mlex 1.0.1");

	fstream lexFile, outputFile;
	string genLang;
	bool deep;

	//首先打开lex文件
	try {
		lexFile.open(args["--lex"].asString(), ios::in);
	}
	catch (exception e) {
		cout << "规则文件无效。" << e.what() << endl;
		exit(0);
	}

	if(args.count("--genlang")==1){
		
		//获取生成语言
		genLang = args["--genlang"].asString();
		if (genLang != "C") {
			cout << "不支持的生成语言:" << genLang << endl;
		}

		//获取生成类型
		if (args.count("--genre") == 1) {
			deep = true;
		}
		else {
			deep = false;
		}

		//打开输出文件
		try {
			outputFile.open(args["--output"].asString(), ios::out | ios::trunc);
		}
		catch (exception e) {
			cout << "输出文件无效。" << e.what() << endl;
			exit(0);
		}

		try {

			//初始化正则表达式对象
			mlex::MlexRegexp regexp(lexFile);

			//正则表达式转换为NFA
			mlex::MlexNfa nfa(regexp);
			nfa.convert();

			//NFA转换为DFA
			mlex::MlexDfa dfa(nfa);
			dfa.convert();

			//对DFA进行简化
			dfa.simplify(deep);

			if (genLang == "C") {
				mlex::MlexCodeGen_C genc(dfa);
				outputFile << genc.gencode();
				outputFile.close();
			}
		}
		catch(mlex::MlexException e){

			while (!mlex::MlexExceptionStack.empty()) {
				mlex::MlexException es = mlex::MlexExceptionStack.top();
				mlex::MlexExceptionStack.pop();
				cout << es.toString() << endl;
			}
			cout << e.toString() << endl;
			exit(0);
		}
		catch(exception e){

			while (!mlex::MlexExceptionStack.empty()) {
				mlex::MlexException es = mlex::MlexExceptionStack.top();
				mlex::MlexExceptionStack.pop();
				cout << es.toString() << endl;
			}
			cout << e.what() << endl;
			exit(0);
		}

		cout << "代码文件生成成功！" << endl;
	}
	else {

	}

    return 0;
}

