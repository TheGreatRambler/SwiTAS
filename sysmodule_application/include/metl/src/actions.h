/*
 @file
actions.h
defines actions to be taken when finding specific patterns in the parsed string. Used by PEGTL

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
#include "grammar.h"
#include "VarExpression.h"

#include "bool_constant.h"
#include "ConstexprBranching.h"
#include "Exceptions.h"

namespace metl
{
	namespace internal
	{
		template< typename Rule >
		struct action
			: nothing<Rule>
		{};

		template<class TargetType, class... Ts, class Compiler, class Converter>
		void convertLiteral(Compiler& compiler, const std::string& input, Converter& converter, TypeList<Ts...>)
		{
			constexpr_if_else(nostd::bool_constant<isInList<TargetType, Ts...>()>(),
				[&](auto _) ->void
			{
				_(compiler).stack_.push(makeConstExpression<typename Compiler::Expression>(converter.f(input)));
			},
				[&](auto _)->void
			{
				try
				{
					_(converter).f(input);
				}
				catch (const BadLiteralException& e)
				{
					throw e;
				}
			}
			);
		}

		template<>
		struct action<intLiteral>
		{
			template< typename Input, class Compiler >
			static void apply(const Input& in, Compiler& compiler)
			{
				auto& converter = compiler.literalConverters_.toInt;
				using intType = decltype(converter.f(in.string()));
				convertLiteral<intType>(compiler, in.string(), converter, compiler.getTypeList());
			}
		};

		template<>
		struct action<realLiteral>
		{
			template< typename Input, class Compiler >
			static void apply(const Input& in, Compiler& compiler)
			{
				auto& converter = compiler.literalConverters_.toReal;
				using realType = decltype(converter.f(in.string()));
				convertLiteral<realType>(compiler, in.string(), converter, compiler.getTypeList());
			}
		};

		template<>
		struct action<FunctionStart>
		{
			template< typename Input, class Compiler >
			static void apply(const Input& in, Compiler& compiler)
			{
				compiler.stack_.pushFunction(in.string());
			}
		};

		template<>
		struct action<one<'('>>
		{
			template< typename Input, class Compiler >
			static void apply(const Input&, Compiler& compiler)
			{
				compiler.stack_.open();
			}
		};

		template<>
		struct action<one<')'>>
		{
			template< typename Input, class Compiler >
			static void apply(const Input&, Compiler& compiler)
			{
				compiler.stack_.close();
			}
		};

		template<>
		struct action<one<','>>
		{
			template< typename Input, class Compiler >
			static void apply(const Input&, Compiler& compiler)
			{
				compiler.stack_.close();
				compiler.stack_.open();
			}
		};

		template<>
		struct action<Function>
		{
			template< typename Input, class Compiler >
			static void apply(const Input&, Compiler& compiler)
			{
				compiler.stack_.close();
			}
		};

		template<>
		struct action<ValidVariableID>
		{
			template< typename Input, class Compiler >
			static void apply(const Input& in, Compiler& compiler)
			{
				compiler.startAssignment(in.string());
			}
		};
	}
}
