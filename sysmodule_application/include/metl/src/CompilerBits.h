/*
@file
Compiler.h
Declares template class Compiler, which carries all the user-defined operators, functions, etc.

Copyright 2017-2018 Till Heinzel

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include <map>

#include "VarExpression.h"
#include "Associativity.h"

namespace metl
{
	namespace internal
	{
		template<class T>
		struct constexprFunction
		{
			T v;

			T operator()() const { return v; }
		};

		template<class Expression, class T>
		Expression makeConstExpression(const T& v)
		{
			auto f = constexprFunction<T>{ v };

			return Expression(exprType<T>(f), CATEGORY::CONSTEXPR);
		}

		template<class ExprT>
		class FunctionImpl
		{
			using FunctionType = std::function<ExprT(const std::vector<ExprT>&)>;
		public:
			FunctionImpl(FunctionType f) :f_(f) {}

			ExprT operator()(const std::vector<ExprT>& v) const;

			template<class T>
			FunctionImpl& operator=(T&& t)
			{
				f_ = t;
				return *this;
			}

		private:

			FunctionType f_;
		};

		template<class ExprT>
		class CastImpl
		{
			using FunctionType = std::function<ExprT(ExprT)>;
		public:
			CastImpl(FunctionType f) :f_(f) {}

			ExprT operator()(ExprT v) const;

			template<class T>
			CastImpl& operator=(T&& t)
			{
				f_ = t;
				return *this;
			}

		private:

			FunctionType f_;
		};


		struct opCarrier
		{
			std::string name;
			unsigned int precedence;
			ASSOCIATIVITY associativity;
			bool isUnary;
		};

		struct suffixCarrier
		{
			std::string name;
		};
	}
}
namespace metl
{
	namespace internal
	{
		template<typename Input, class T>
		static auto match_any_recursive(Input& in, const std::map<std::string, T>& map, std::string t)
			-> typename std::map<std::string, T>::const_iterator
		{
			// t starts out empty, and slowly becomes the same as in.current, adding one char on each recursion-step. t is then searched in the keys of map.
			if (in.size() > t.size()) {
				t += in.peek_char(t.size()); // append another char to teststring t
				const auto i = map.lower_bound(t); // not sure why this works
				if (i != map.end()) {
					// recursion step. try the next one down the line with an additional char.
					// This happend before the termination, because we are expected to be greedy.
					auto i2 = match_any_recursive(in, map, t);
					if (i2 != map.end())
					{
						return i2;
					}
					if (i->first == t)  // recursion stop: if we found a match, return.
					{
						return i;
					}
				}
			}
			return map.end();
		}

		template<class... Ts>
		class CompilerBits
		{

		public:
			using Expression = VarExpression<Ts...>;
			constexpr static auto getTypeList() { return TypeList<Ts...>(); }
			
			void setOperatorPrecedence(const std::string& op, unsigned int precedence, ASSOCIATIVITY associativity = ASSOCIATIVITY::LEFT);
			void setUnaryOperatorPrecedence(const std::string& op, unsigned int precedence);

			void setOperator(const std::string& token, const std::vector<TYPE>& paramTypes, const FunctionImpl<Expression>& op);

			void setUnaryOperator(const std::string& token, TYPE paramType, const FunctionImpl<Expression>& op);

			void setFunction(const std::string& token, const std::vector<TYPE>& paramTypes, const FunctionImpl<Expression>& function);

			void setCast(TYPE from, TYPE to, const CastImpl<Expression>& fs);

			void setSuffix(const std::string& token, TYPE from, const CastImpl<Expression>& conversion);

			void addConstantOrVariable(const std::string& token, const Expression& val);

			template<class T>
			constexpr static TYPE type();

			template<class Input>
			auto matchVariable(Input& in)
			{
				return match(in, constantsAndVariables_);
			}

			template<class Input>
			auto matchOperator(Input& in)
			{
				return match(in, opCarriers_);
			}

			template<class Input>
			auto matchUnaryOperator(Input& in)
			{
				return match(in, unaryCarriers_);
			}

			template<class Input>
			auto matchSuffix(Input& in)
			{
				return match(in, suffixes_);
			}

			template<class Input>
			auto matchFunctionName(Input& in)
			{
				return match(in, functionNames_);
			}

			const auto& getOperators() { return operators_; }
			const auto& getCandV() { return constantsAndVariables_; }
			const auto& getCarriers() { return opCarriers_; }
			const auto& getUnaryCarriers() { return unaryCarriers_; }
			const auto& getFunctionNames() { return functionNames_; }
			const auto& getFunctions() { return functions_; }
			const auto& getcastImplementations() { return castImplementations_; }
			const auto& getcastDeclarations() { return castDeclarations_; }
			const auto& getSuffixes() { return suffixes_; }
			const auto& getSuffixImplementations() { return suffixes_; }
			
			std::map<std::string, CastImpl<Expression>> castImplementations_;
			std::map<std::string, Expression> constantsAndVariables_; // maps identifiers for constants and variables to the expressions returning their values.

		public:

			template<class Input, class Map>
			auto match(Input& in, const Map& map)
			{
				auto it = match_any_recursive(in, map, std::string());

				bool varMatched = (it == map.end()) ? false : true;

				return std::make_tuple(varMatched, it);
			}


			std::map<std::string, CastImpl<Expression>> suffixImplementations_;
			std::map<std::string, suffixCarrier> suffixes_;
			std::map<TYPE, std::vector<TYPE>> castDeclarations_{ std::make_pair(type<Ts>(), std::vector<TYPE>{type<Ts>()})... };

			std::map<std::string, opCarrier> opCarriers_; // maps unmangled operators to their precedence
			std::map<std::string, opCarrier> unaryCarriers_; // maps unmangled operators to their precedence
			std::map<std::string, FunctionImpl<Expression>> operators_; //maps mangled names to implementations for operators
			std::map<std::string, FunctionImpl<Expression>> functions_; // maps mangled names to implementations for functions.
			std::map<std::string, std::string> functionNames_; // dummy, just so we have the sorting.
			
		};
	}
}
