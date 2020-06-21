/*
@file
ConstexprBranching.h
Utilities for branching at compile-time

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

namespace metl
{
	namespace internal
	{
		namespace detail
		{
			struct ForwardSame
			{
				template<class T>
				constexpr decltype(auto) operator() (T&& t)
				{
					return std::forward<T>(t);
				}
			};


			template<class TrueBranch, class FalseBranch>
			decltype(auto) constexpr constexpr_ternary_invoke(const std::true_type&, const TrueBranch& trueBranch, const FalseBranch&)
			{
				return trueBranch(ForwardSame());
			}

			template<class TrueBranch, class FalseBranch>
			decltype(auto) constexpr constexpr_ternary_invoke(const std::false_type&, const TrueBranch&, const FalseBranch& falseBranch)
			{
				return falseBranch(ForwardSame());
			}
		}

		template<class Condition, class TrueBranch, class FalseBranch>
		decltype(auto) constexpr constexpr_ternary(const Condition&, const TrueBranch& trueBranch, const FalseBranch& falseBranch)
		{
			return detail::constexpr_ternary_invoke(std::integral_constant<bool, Condition::value>{}, trueBranch, falseBranch);
		}
	}
}

namespace metl
{
	namespace internal
	{
		template<class Condition, class TrueBranch, class FalseBranch>
		void constexpr constexpr_if_else(const Condition&, const TrueBranch& trueBranch, const FalseBranch& falseBranch)
		{
			constexpr_ternary(Condition(), trueBranch, falseBranch);
		}

		template<class Condition, class TrueBranch>
		void constexpr constexpr_if(const Condition&, const TrueBranch& trueBranch)
		{
			constexpr_if_else(Condition(), trueBranch, [](auto _) {return _(0); });
		}

	}
}
