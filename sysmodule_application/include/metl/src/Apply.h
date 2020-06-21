/*
@file Apply.h
Allows invoking a function with a tuple of arguments

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

namespace metl
{
	namespace internal
	{
		namespace detail {
			template <class F, class Tuple, std::size_t... I>
			constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
			{
				return f(std::get<I>(std::forward<Tuple>(t))...);
			}
		}  // namespace detail

		template <class F, class Tuple>
		constexpr decltype(auto) apply(F&& f, Tuple&& t)
		{
			return detail::apply_impl(
				std::forward<F>(f), std::forward<Tuple>(t),
				std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
		}
	}
}
