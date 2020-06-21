/*
@file
sortAndAddDefaults.h
utility function that allows creating API-functions with arbitrary order of inputs and defaults for inputs

Copyright 2017, 2018 Till Heinzel

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
#include <tuple>

#include "TypeList.h"
#include "TypeMap.h"
#include "ConstexprBranching.h"
#include "bool_constant.h"
#include "Apply.h"

namespace metl
{
	namespace internal
	{
		template<class CheckPolicy, class DefaultsMap, class InputMap>
		auto sortAndAddDefaults_impl(const DefaultsMap defaults, const InputMap& inputs)
		{
			return internal::constexpr_ternary(nostd::bool_constant<InputMap::template isInList<CheckPolicy>()>(),
				[&](auto _)
			{
				return 
					std::make_tuple(_(inputs).template get<CheckPolicy>())
					;
			}, 
				[&](auto _)
			{
				return 
					std::make_tuple(_(defaults).template get<CheckPolicy>())
				;
			});
		}

		template<class CheckPolicy, class NextCheck, class... RemainingPolicies, class DefaultsMap, class InputMap>
		auto sortAndAddDefaults_impl(const DefaultsMap defaults, const InputMap& inputs)
		{
			auto currentPolicy = internal::constexpr_ternary(nostd::bool_constant<InputMap::template isInList<CheckPolicy>()>(),
				[&](auto _)
			{
				return 
					_(inputs).template get<CheckPolicy>()
				;
			},
				[&](auto _)
			{
				return 
					_(defaults).template get<CheckPolicy>()
				;
			});

			return std::tuple_cat(std::make_tuple(currentPolicy), sortAndAddDefaults_impl<NextCheck, RemainingPolicies...>(defaults, inputs));
		}

		template<class... DefaultIDs, class... DefaultPolicies, class... InputIDs, class... InputPolicies >
		auto sortAndAddDefaults(const TypeMap<std::tuple<DefaultIDs...>, std::tuple<DefaultPolicies...>>& map, TypeMap<std::tuple<InputIDs...>, std::tuple<InputPolicies...>> inputs)
		{
			return sortAndAddDefaults_impl<DefaultIDs...>(map, inputs);
		}
	}
}

namespace metl
{
	namespace internal
	{

		template<class... DefaultIDs>
		constexpr bool areInputIDsValid(TypeList<DefaultIDs...>, TypeList<>)
		{
			return true;
		}

		template<class... DefaultIDs, class InputID, class... InputIDs >
		constexpr bool areInputIDsValid(TypeList<DefaultIDs...> d, TypeList<InputID, InputIDs...>)
		{
			return isInList<InputID, DefaultIDs...>() ? internal::areInputIDsValid(d, TypeList<InputIDs...>{}) : false;
		}

		template<class ID, class Setting>
		struct SettingWrapper
		{
			Setting v;
		};

		template<class ID, class Setting>
		SettingWrapper<ID, Setting> wrapSetting(Setting setting)
		{
			return { setting };
		}

		template<class... IDs, class... Settings>
		TypeMap<std::tuple<IDs...>, std::tuple<Settings...>> toTypeMap(SettingWrapper<IDs, Settings>... settings)
		{
			return makeTypeMap(TypeList<IDs...>(), std::make_tuple(settings.v...));
		}

		template<class F, class M, class InputMap>
		auto callWithOrderedAndDefaultedSettings(F f, M orderedDefaultMap, InputMap inputMap)
		{
			//static_assert(internal::areInputIDsValid(internal::getKeys(internal::Type<M>()), internal::getKeys(internal::Type<InputMap>())), "One of the passed-in settings is not allowed for this function");
			return internal::apply(f, internal::sortAndAddDefaults(orderedDefaultMap, inputMap));
		}
	}
}
