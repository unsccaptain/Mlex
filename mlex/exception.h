
#pragma once

#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <stack>

using namespace std;

namespace mlex {

	enum class MlexExceptionLevel {
		Warning,
		Error
	};

	enum class MlexExceptionType {
		InvalidExpression,
		UnrecognizedChar,
		NoMoreItems,
		InvalidInput,
		ErrorInNfaToDfa
	};

	using ExceptionUtype = underlying_type<MlexExceptionType>::type;

	class MlexException {
	public:
		MlexExceptionLevel _level;
		MlexExceptionType _type;
		string _desc;

		MlexException(MlexExceptionLevel Level, MlexExceptionType Type) :_level(Level), _type(Type) {
		}

		MlexException(MlexExceptionLevel Level, MlexExceptionType Type, string Desc) :_level(Level), _type(Type), _desc(Desc) {
		}

		string& translate(MlexExceptionType Type) {

			string human_friendly;
			switch (Type)
			{
			case mlex::MlexExceptionType::InvalidExpression:
				human_friendly = "表达式错误。";
				break;
			case mlex::MlexExceptionType::UnrecognizedChar:
				human_friendly = "无法识别的字符。";
				break;
			case mlex::MlexExceptionType::NoMoreItems:
				human_friendly = "容器越界。";
				break;
			case mlex::MlexExceptionType::InvalidInput:
				human_friendly = "输入流错误。";
				break;
			default:
				human_friendly = "未知错误。";
				break;
			}

			return human_friendly;
		}

		string& toString() {

			string ret;
			switch (_level)
			{
			case MlexExceptionLevel::Warning:
				ret = "警告（" + to_string(ExceptionUtype(_type)) + "）:" + translate(_type) + _desc;
				break;
			case MlexExceptionLevel::Error:
				ret = "错误（" + to_string(ExceptionUtype(_type)) + "）:" + translate(_type) + _desc;
				break;
			default:
				break;
			}
			
			return ret;
		}
	};

	stack<MlexException> MlexExceptionStack;

}