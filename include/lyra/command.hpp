// Copyright 2020-2022 René Ferdinand Rivera Morell
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LYRA_COMMAND_HPP
#define LYRA_COMMAND_HPP

#include "group.hpp"
#include "literal.hpp"
#include <functional>
#include <string>

namespace lyra {

/* tag::reference[]

[#lyra_command]
= `lyra::command`

A parser that encapsulates the pattern of parsing sub-commands. It provides a
quick wrapper for the equivalent arrangement of `group` and `literal` parsers.
For example:

[source]
----
lyra::command c = lyra::command("sub");
----

Is equivalent to:

[source]
----
lyra::command c = lyra::group()
	.sequential()
	.add_argument(literal("sub"))
	.add_argument(group());
lyra::group & g = c.get<lyra::group>(1);
----

I.e. it's conposed of a `literal` followed by the rest of the command arguments.

Is-a <<lyra_group>>.

*/ // end::reference[]
class command : public group
{
	public:
	// Construction, with and without, callback.
	explicit command(const std::string & n);
	command(
		const std::string & n, const std::function<void(const group &)> & f);

	// Help description.
	command & help(const std::string & text);
	command & operator()(std::string const & description);

	// Add arguments.
	template <typename P>
	command & add_argument(P const & p);
	template <typename P>
	command & operator|=(P const & p);

	// Settings.
	command & brief_help(bool brief = true);

	// Internal.
	virtual std::unique_ptr<parser> clone() const override
	{
		return make_clone<command>(this);
	}

	virtual std::string get_usage_text(
		const option_style & style) const override
	{
		return parsers[0]->get_usage_text(style) + " "
			+ parsers[1]->get_usage_text(style);
	}

	virtual help_text get_help_text(const option_style & style) const override
	{
		if (expanded_help_details)
		{
			help_text text;
			text.push_back({ "", "" });
			auto c = parsers[0]->get_help_text(style);
			text.insert(text.end(), c.begin(), c.end());
			text.push_back({ "", "" });
			auto o = parsers[1]->get_help_text(style);
			text.insert(text.end(), o.begin(), o.end());
			return text;
		}
		else
			return parsers[0]->get_help_text(style);
	}

	protected:
	bool expanded_help_details = true;

	virtual void print_help_text_details(
		printer & p, const option_style & style) const override
	{
		// This avoid printing out the "internal" group brackets "{}" for the
		// command arguments.
		p.heading("OPTIONS, ARGUMENTS:");
		for (auto const & cols : parsers[1]->get_help_text(style))
		{
			p.option(cols.option, cols.description, 2);
		}
	}
};

/* tag::reference[]

[#lyra_command_ctor]
== Construction

[source]
----
command::command(const std::string & n);
command::command(
	const std::string & n, const std::function<void(const group &)>& f);
----

To construct an `command` we need a name (`n`) that matches, and triggers, that
command.


end::reference[] */
inline command::command(const std::string & n)
{
	this->sequential()
		.add_argument(literal(n))
		.add_argument(group().required());
}
inline command::command(
	const std::string & n, const std::function<void(const group &)> & f)
	: group(f)
{
	this->sequential()
		.add_argument(literal(n))
		.add_argument(group().required());
}

/* tag::reference[]

[#lyra_command_specification]
== Specification

end::reference[] */

/* tag::reference[]

[#lyra_command_help]
=== `lyra:command::help`

[source]
----
command & command::help(const std::string& text)
command & command::operator()(std::string const& description)
----

Specify a help description for the command. This sets the help for the
underlying literal of the command.

end::reference[] */
inline command & command::help(const std::string & text)
{
	this->get<literal>(0).help(text);
	return *this;
}
inline command & command::operator()(std::string const & description)
{
	return this->help(description);
}

/* tag::reference[]
[#lyra_command_add_argument]
=== `lyra::command::add_argument`

[source]
----
template <typename P>
command & command::add_argument(P const & p);
template <typename P>
command & command::operator|=(P const & p);
----

Adds the given argument parser to the considered arguments for this `comand`.
The argument is added to the sub-group argument instead of this one. Hence it
has the effect of adding arguments *after* the command name.

end::reference[] */
template <typename P>
command & command::add_argument(P const & p)
{
	this->get<group>(1).add_argument(p);
	return *this;
}
template <typename P>
command & command::operator|=(P const & p)
{
	return this->add_argument(p);
}

/* tag::reference[]
[#lyra_command_abrief_help]
=== `lyra::command::brief_help`

[source]
----
command & command::brief_help(bool brief = true);
----

Enables, or disables with `false`, brief output of the top level help. Brief
output only prints out the command name and description for the top level
help (i.e. `std::cout << cli`). You can output the full command, options, and
arguments by printing the command (i.e. `std::cout << command`).

end::reference[] */
inline command & command::brief_help(bool brief)
{
	expanded_help_details = !brief;
	return *this;
}


} // namespace lyra

#endif
