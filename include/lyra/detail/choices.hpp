// Copyright 2018-2022 René Ferdinand Rivera Morell
// Copyright 2017 Two Blue Cubes Ltd. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LYRA_DETAIL_CHOICES_HPP
#define LYRA_DETAIL_CHOICES_HPP

#include "../parser_result.hpp"
#include "from_string.hpp"
#include "result.hpp"
#include "unary_lambda_traits.hpp"
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

namespace lyra { namespace detail {

/*
Type erased base for set of choices. I.e. it's an "interface".
*/
struct choices_base
{
	virtual ~choices_base() = default;
	virtual parser_result contains_value(std::string const & val) const = 0;
};

/*
Stores a set of choice values and provides checking if a given parsed
string value is one of the choices.
*/
template <typename T>
struct choices_set : choices_base
{
	// The allowed values.
	std::vector<T> values;

	template <typename... Vals>
	explicit choices_set(Vals... vals)
		: choices_set({ vals... })
	{}

	explicit choices_set(const std::vector<T> & vals)
		: values(vals)
	{}

	// Checks if the given string val exists in the set of
	// values. Returns a parsing error if the value is not present.
	parser_result contains_value(std::string const & val) const override
	{
		T value;
		auto parse = parse_string(val, value);
		if (!parse)
		{
			return parser_result::error(
				parser_result_type::no_match, parse.message());
		}
		for (const T & allowed_value : values)
		{
			if (allowed_value == value)
				return parser_result::ok(parser_result_type::matched);
		}
		// We consider not finding a choice a parse error.
		return parser_result::error(parser_result_type::no_match,
			"Value '" + val
				+ "' not expected. Allowed values are: " + this->to_string());
	}

	// Returns a comma separated list of the allowed values.
	std::string to_string() const
	{
		std::string text;
		for (const T & val : values)
		{
			if (!text.empty()) text += ", ";
			std::string val_string;
			if (detail::to_string(val, val_string))
				text += val_string;
			else
				text += "<value error>";
		}
		return text;
	}

	protected:
	explicit choices_set(std::initializer_list<T> const & vals)
		: values(vals)
	{}
};

template <>
struct choices_set<const char *> : choices_set<std::string>
{
	template <typename... Vals>
	explicit choices_set(Vals... vals)
		: choices_set<std::string>(vals...)
	{}
};

/*
Calls a designated function to check if the choice is valid.
*/
template <typename Lambda>
struct choices_check : choices_base
{
	static_assert(unary_lambda_traits<Lambda>::isValid,
		"Supplied lambda must take exactly one argument");
	static_assert(std::is_same<bool,
					  typename unary_lambda_traits<Lambda>::ReturnType>::value,
		"Supplied lambda must return bool");

	Lambda checker;
	using value_type = typename unary_lambda_traits<Lambda>::ArgType;

	explicit choices_check(Lambda const & checker_function)
		: checker(checker_function)
	{}

	// Checks if the given string val exists in the set of
	// values. Returns a parsing error if the value is not present.
	parser_result contains_value(std::string const & val) const override
	{
		value_type value;
		auto parse = parse_string(val, value);
		if (!parse)
		{
			return parser_result::error(
				parser_result_type::no_match, parse.message());
		}
		if (checker(value))
		{
			return parser_result::ok(parser_result_type::matched);
		}
		// We consider not finding a choice a parse error.
		return parser_result::error(
			parser_result_type::no_match, "Value '" + val + "' not expected.");
	}
};

}} // namespace lyra::detail

#endif
