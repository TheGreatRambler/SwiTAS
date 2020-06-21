/*
@file
grammar.h
Defines the grammar to be used by PEGTL for parsing strings

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

#include <tao/pegtl.hpp>

#include "src/ConstexprBranching.h"

namespace metl
{
	namespace internal
	{
		// Implementations of special rules are in bottom of file.

		namespace pegtl = tao::pegtl;
		using namespace pegtl;

		struct padding : star<space> {};

		///////////////// ATOMICS ////////////////
		struct intLiteral
			: seq< plus<digit>> {};

		struct realLiteral
			: seq< plus<digit>, one<'.'>, plus<digit>> {};

		struct Variable; // constants and variables are equivalent wrt the grammar. 

		struct Expr;

		struct bracket : seq< one< '(' >, Expr, one< ')' > > {};

		/////////////// FUNCTIONS ///////////////

		struct FunctionName;

		struct FunctionStart : seq<FunctionName, at<one<'('>>> {};

		struct Function : if_must<FunctionStart, one<'('>, opt<list<Expr, one<','>, space>>, one<')'>> {};

		/////////////// EXPRESSIONS ///////////////

		struct UnaryOperator;

		struct Suffix;

		struct Atomic :
			seq<opt<UnaryOperator>, star<space>, sor<bracket, Function, Variable, seq<sor<realLiteral, intLiteral>, opt<Suffix>>>> {};

		struct Operator;

		struct Expr
			: list<Atomic, Operator, space> {};



		////////////////// ASSIGNMENT //////////////////////

		struct ValidVariableID :
			seq<alpha, star<alnum>>
		{};

		struct AssignmentBit :
			seq<padding, ValidVariableID, padding, one<'='>, padding>
		{};

		////////////////// GRAMMAR //////////////////////

		struct grammarWithAssignment
			: must<if_then_else<at<AssignmentBit>, seq<AssignmentBit, Expr>, Expr>, padding, pegtl::eof> {};

		struct grammarWithOutAssignment
			: must<Expr, opt<padding>, pegtl::eof> {};
	}
}

namespace metl
{
	namespace internal
	{
		struct Variable // constants and variables are equivalent wrt the grammar
		{
			using analyze_t = analysis::generic< analysis::rule_type::ANY >;

			template< apply_mode A,
				rewind_mode M,
				template< typename... > class Action,
				template< typename... > class Control,
				typename Input, class Compiler, class... Others>
				static bool match(Input& in, Compiler& s, const Others&...)
			{
				auto ret = s.bits_.matchVariable(in);

				if (std::get<0>(ret) == false) return false;
				const auto& it = std::get<1>(ret);

				s.stack_.push(it->second);
				in.bump(it->first.size());
				return true;
			}
		};

		struct Operator
		{
			using analyze_t = analysis::generic< analysis::rule_type::ANY >;

			template< apply_mode A,
				rewind_mode M,
				template< typename... > class Action,
				template< typename... > class Control,
				typename Input, class Compiler>
				static bool match(Input& in, Compiler& s)
			{
				auto ret = s.bits_.matchOperator(in);

				if (std::get<0>(ret) == false) return false;
				const auto& it = std::get<1>(ret);

				s.stack_.push(it->second);
				in.bump(it->first.size());
				return true;
			}
		};

		struct UnaryOperator
		{
			using analyze_t = analysis::generic< analysis::rule_type::ANY >;

			template< apply_mode A,
				rewind_mode M,
				template< typename... > class Action,
				template< typename... > class Control,
				typename Input, class Compiler>
				static bool match(Input& in, Compiler& s)
			{
				auto ret = s.bits_.matchUnaryOperator(in);

				if (std::get<0>(ret) == false) return false;
				const auto& it = std::get<1>(ret);

				s.stack_.push(it->second);
				in.bump(it->first.size());
				return true;
			}
		};

		struct Suffix
		{
			using analyze_t = analysis::generic<analysis::rule_type::ANY>;

			template< apply_mode A,
				rewind_mode M,
				template< typename... > class Action,
				template< typename... > class Control,
				typename Input, class Compiler>
				static bool match(Input& in, Compiler& s)
			{
				auto ret = s.bits_.matchSuffix(in);

				if (std::get<0>(ret) == false) return false;
				const auto& it = std::get<1>(ret);

				s.stack_.push(it->second);
				in.bump(it->first.size());
				return true;
			}
		};

		struct FunctionName
		{
			using analyze_t = analysis::generic< analysis::rule_type::ANY >;

			template< apply_mode A,
				rewind_mode M,
				template< typename... > class Action,
				template< typename... > class Control,
				typename Input, class Compiler>
				static bool match(Input& in, Compiler& s)
			{

				auto ret = s.bits_.matchFunctionName(in);
				if (std::get<0>(ret) == false) return false;

				const auto& it = std::get<1>(ret);

				in.bump(it->first.size()); // remove the operator from the input.
				return true;
			}
		};
	}
}
