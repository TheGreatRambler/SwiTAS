/*
@file
LiteralConverters.h
The utilities to create values from strings representing literals, such as "1701" or "42.31415"

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

#include "ConstexprBranching.h"
#include "Exceptions.h"

namespace metl
{
	namespace internal
	{
		// Type to carry functors f that convert strings representing literals. 
		// From_t is basically the index for the metl-compiler, so when From_t =int this converter will be invoked to deal with literals "1" or "42", 
		// while From_t = doube is invoked for "1.0" and "8.008"
		template<class From_t, class To_t, class F>
		struct Converter
		{
			using From = From_t;
			using To = To_t;
			F f;
		};

		template<class From_t, class F>
		auto makeConverter(F f)
		{
			using To_t = decltype(f(std::string()));
			return Converter<From_t, To_t, F>{f};
		}

		template<class IntConverter_, class RealConverter_>
		struct LiteralConverters
		{
			using IntConverter = IntConverter_;
			using RealConverter = RealConverter_;

			IntConverter toInt;
			RealConverter toReal;
		};

		template<class... Ts>
		auto makeLiteralConverters(Ts...ts) { return LiteralConverters<Ts...>{ts...}; }
	}

	namespace settings
	{
		struct IntConverter {};
		struct RealConverter {};
	}


	template<class F, class T = decltype(std::declval<F>()(std::declval<std::string>())) >
	auto makeIntConverter(F f) { return internal::wrapSetting<settings::IntConverter>(internal::Converter<int, T, F>{f}); }

	template<class F, class T = decltype(std::declval<F>()(std::declval<std::string>()))>
	auto makeRealConverter(F f) { return internal::wrapSetting<settings::RealConverter>(internal::Converter<double, T, F>{f}); }

	template<class... SupportedTypes>
	auto makeDefaultIntConverter()
	{
		return internal::wrapSetting<settings::IntConverter>
			(
				internal::constexpr_ternary(std::integral_constant<bool, internal::isInList<int, SupportedTypes...>()>(),
					[](auto _)
		{
			return _(internal::makeConverter<int>([](const std::string& s) {return std::stoi(s); }));
		},
					[](auto _)
		{
			return _(internal::makeConverter<int>([](const auto&) -> int
			{
				throw BadLiteralException("");
			}));
		}));
	}

	template<class... SupportedTypes>
	auto makeDefaultRealConverter()
	{
		return internal::wrapSetting<settings::RealConverter>
			(
				internal::constexpr_ternary(std::integral_constant<bool, internal::isInList<double, SupportedTypes...>()>(),
					[](auto)
		{
			return internal::makeConverter<double>([](const std::string& s) {return std::stod(s); });
		},
					[](auto)
		{
			return internal::makeConverter<double>([](const auto&) -> double
			{
				throw BadLiteralException("");
			});
		}));
	}

}
