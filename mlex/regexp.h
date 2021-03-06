#pragma once

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

#include "exception.h"

using namespace std;

namespace mlex {

	/**
	 * 校验是一个英文字母
	 * @param	c			字符
	 */
	bool isLetter(char c) {
		if ((c >= 'a'&&c <= 'z') || (c >= 'A'&&c <= 'Z')) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * 校验是一个阿拉伯数字
	 * @param	c			字符
	 */
	bool isNumberic(char c) {
		if ((c >= '0'&&c <= '9')) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * 校验是一个可显示字符
	 * @param	c			字符
	 */
	bool isValidChar(char c) {
		if ((c >= 32) && (c <= 128)) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * 校验是一个闭包运算
	 * @param	c			字符
	 */
	bool isClosure(char c) {
		if ((c == '*') || (c == '+') || (c == '?')) {
			return true;
		}
		else {
			return false;
		}
	}

	class MlexRegexpContext {
	public:
		string _regExp;
		string _genCode;

		MlexRegexpContext() {
		}

		MlexRegexpContext(string RegExp, string Code) :_regExp(RegExp), _genCode(Code) {
		}
	};

	class MlexRegexp {

	private:
		vector<MlexRegexpContext> _regExps;
		//fstream _inputStream;
		char* _inputString;

	public:
		string _define_part;
		string _code_part;

		/** 
		 * 从字符串中初始化正则表达式
		 * @param	Input			包含正则表达式的字符串
		 */
		MlexRegexp(char* Input) :_inputString(Input) {

			string sline;
			stringbuf sbuf(_inputString);
			istream is(&sbuf);

			while (!is.eof()) {

				getline(is, sline);
				if (sline.compare("%%") == 0) {
					break;
				}
				else {
					_define_part += sline + "\n";
				}
			}

			while (!is.eof()) {
				
				getline(is, sline);
				if (sline.compare("%%") == 0) {
					break;
				}

				if (sline == "") {
					continue;
				}

				size_t wspos = min(sline.find_first_of(' '), sline.find_first_of('\t'));
				string part2, part1 = sline.substr(0, wspos);
				if (wspos != size_t(-1)) {
					part2 = sline.substr(wspos, sline.size() - wspos);
					part2 = part2.substr(max(part2.find_first_not_of(' '), part2.find_first_not_of('\t')));
				}
				else {
					part2 = "";
				}

				if (validateRegExp(part1)) {
					_regExps.push_back(MlexRegexpContext(part1, part2));
				}
			}
		}

		/** 
		 * 从文件中初始化正则表达式
		 * @param	FileStream		包含正则表达式的文件
		 */
		MlexRegexp(fstream& FileStream) {

			string sline;

			if (!FileStream.good()) {
				throw(MlexException(MlexExceptionLevel::Error, MlexExceptionType::InvalidInput, "输入文件无效。"));
				exit(0);
			}

			while (!FileStream.eof()) {

				getline(FileStream, sline);
				if (sline.compare("%%") == 0) {
					break;
				}
				else {
					_define_part += sline + "\n";
				}
			}

			while (!FileStream.eof()) {

				getline(FileStream, sline);
				if (sline.compare("%%") == 0) {
					break;
				}

				if (sline == "") {
					continue;
				}

				size_t wspos = min(sline.find_first_of(' '), sline.find_first_of('\t'));
				string part2,part1 = sline.substr(0, wspos);
				if (wspos != size_t(-1)) {
					part2 = sline.substr(wspos, sline.size() - wspos);
					part2 = part2.substr(max(part2.find_first_not_of(' '), part2.find_first_not_of('\t')));
				}
				else {
					part2 = "";
				}
				
				if (validateRegExp(part1)) {
					_regExps.push_back(MlexRegexpContext(part1, part2));
				}
			}
		}

		MlexRegexp() :_inputString(nullptr) {
		}

		/**
		 * 简单的校验一条正则表达式
		 * @param	re			表达式对象
		 */
		bool validateRegExp(string& re) {

			uint32_t sbk = 0, mbk = 0;
			for (size_t i = 0;i < re.length();i++) {

				//特殊字符检测
				//switch (re[i])
				//{
				//case '(':
				//	sbk++;
				//	continue;
				//case '[':
				//	mbk++;
				//	continue;
				//case ')':
				//	sbk--;
				//	continue;
				//case ']':
				//	mbk--;
				//}

				if (!isValidChar(re[i])) {
					throw(MlexException(MlexExceptionLevel::Error,MlexExceptionType::UnrecognizedChar));
					return false;
				}
			}

			//if (sbk != 0) {
			//	throw(string(re + ":未闭合小括号。"));
			//	return false;
			//}

			//if (mbk != 0) {
			//	throw(string(re + ":未闭合中括号。"));
			//	return false;
			//}

			return true;
		}

		/**
		 * 追加一条正则表达式
		 * @param	re			表达式对象
		 */
		bool appendRegExp(string re) {

			if (validateRegExp(re)) {
				_regExps.push_back(MlexRegexpContext(re, ""));
				return true;
			}
			else {
				return false;
			}
		}

		/**
		 * 追加一条正则表达式
		 * @param	re			表达式对象
		 */
		bool appendRegExp(string re, string code) {

			if (validateRegExp(re)) {
				_regExps.push_back(MlexRegexpContext(re, code));
				return true;
			}
			else {
				return false;
			}
		}

		/**
		 * 获取第i条正则表达式
		 * @param	i			表达式索引
		 */
		MlexRegexpContext& getRegExp(uint16_t i) {
			
			if (i >= _regExps.size()) {
				throw(MlexException(MlexExceptionLevel::Warning, MlexExceptionType::NoMoreItems, "获取正则表达式越界。"));
				return MlexRegexpContext();
			}

			return _regExps[i];
		}

		/**
		 * 获取表达式数量
		 */
		size_t getReCount() {

			return _regExps.size();
		}
	};

}