/*
@file
Compiler.impl.h
Implements functions for class Compiler

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
#include "Compiler.h"

#include <tao/pegtl.hpp>

#include "grammar.h"
#include "actions.h"

#include "nameMangling.h"
#include "Stack.impl.h"

#include "CompilerHelpers.h"
#include "CompilerBits.impl.h"
#include <locale>

namespace metl
{
	namespace internal
	{
		template <class LiteralConverters, class ... Ts>
		Compiler<LiteralConverters, Ts...>::Compiler(const LiteralConverters& literalConverters) :
			literalConverters_(literalConverters),
			stack_(bits_)
		{}

		template <class LiteralConverters, class... Ts>
		template <class T>
		constexpr TYPE Compiler<LiteralConverters, Ts...>::type()
		{
			return classToType2<T, Ts...>();
		}

		namespace
		{


			template<class Expression>
			void assignToVar_impl(Expression&, const Expression&)// assumes the two expression have the same type
			{
				throw std::runtime_error("Something weird happened here");
			}

			template<class T, class... RemainingTs, class Expression>
			void assignToVar_impl(Expression& existingVarExpression, const Expression& evaluatedExpr)// assumes the two expression have the same type
			{
				if(existingVarExpression.type() == existingVarExpression.template toType<T>())
				{
					*existingVarExpression.template get<T>().template target<VariableExpression<T>>()->v = evaluatedExpr.template get<T>()();
				}
				else 
				{
					assignToVar_impl<RemainingTs...>(existingVarExpression, evaluatedExpr);
				}
			}
			
			template<class... Ts>
			void assignToVar(VarExpression<Ts...>& existingVarExpression, const VarExpression<Ts...>& evaluatedExpr)// assumes the two expression have the same type
			{
				assignToVar_impl<Ts...>(existingVarExpression, evaluatedExpr);
			}
		}

		template <class LiteralConverters, class ... Ts>
		typename Compiler<LiteralConverters, Ts...>::Expression Compiler<LiteralConverters, Ts...>::finish()
		{
			auto expr = stack_.finish();

			if(assignToThis_ != "")
			{
				auto evaluatedExpr = evaluateConstExpr(expr);

				auto it = bits_.constantsAndVariables_.find(assignToThis_);
				if(it == bits_.constantsAndVariables_.end())
				{
					bits_.constantsAndVariables_.emplace(assignToThis_, evaluatedExpr);
				}
				else if(it->second.category() == CATEGORY::CONSTEXPR)
				{
					it->second = evaluatedExpr;
				}
				else // name exist, is dynexpr
				{
					if(evaluatedExpr.type() != it->second.type())
					{
						auto fromType = evaluatedExpr.type();
						auto toType = it->second.type();

						auto castIt = bits_.castImplementations_.find(mangleCast(fromType, toType));
						if (castIt == bits_.castImplementations_.end()) throw std::runtime_error("cannot assign to incompatible type");
						evaluatedExpr = castIt->second(evaluatedExpr);
					}
					assignToVar(it->second, evaluatedExpr);
				}
			}

			assignToThis_ = "";
			return expr;
		}

		template <class LiteralConverters, class ... Ts>
		void Compiler<LiteralConverters, Ts...>::startAssignment(const std::string& varName)
		{
			assignToThis_ = varName;
		}
	}
}