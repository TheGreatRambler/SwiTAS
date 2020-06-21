/*
@file
Stack.fwd.h
Declares class Stack, which implements the shunting-yard algorithm.

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
#include <vector>
#include <functional>
#include <string>

#include "VarExpression.h"
#include "CompilerBits.h"
#include "Associativity.h"


namespace metl
{
	namespace internal {
		template<class... Ts>
		class Stack
		{
			using Expression = VarExpression<Ts...>;

			class Substack
			{
			public:

				explicit Substack(const CompilerBits<Ts...>& bits);


				void push(const Expression l);
				void push(const opCarrier& b);
				void pushFunction(std::string FunctionName);
				void push(const suffixCarrier& suffix);

				Expression finish();

				void evaluateFunction();

				void reduce();
				void reduceBinary();
				void reduceUnary();

				bool empty() const { return expressions_.empty(); }
			private:
				bool plannedSignSwitch = false;

				std::vector< Expression > expressions_;
				std::vector< opCarrier > operators_;

				std::unique_ptr<std::string> function_;
				
				const CompilerBits<Ts...>& bits_;


			private:
				//void tryCast()
				bool are_all_const(const std::vector<Expression>& expressions);
				void castTo(const std::vector<TYPE>& targetTypes);
			};

		public:
			Stack(const CompilerBits<Ts...>& bits);

			void push(const Expression& t);

			void pushFunction(std::string functionName);

			void push(const opCarrier& op);

			void push(const suffixCarrier& suffix);

			void open();

			void close();

			Expression finish();
			void clear();

		private:
			std::vector<Substack> subStacks_;

			const CompilerBits<Ts...>& bits_;
		};
	}
}
