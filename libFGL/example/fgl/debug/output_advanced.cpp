/**
This file is an example for <fgl/debug/output.hpp>

--- Example output
-------------------------------------------------------------------------------
[Entity] Lexy at coordinates (4.4,3.3,2.2)
[Entity] Nakamura at coordinates (3,5,10)
[Entity] Eralia at coordinates (100,50000,200)
*/

#include <string>
#include <sstream>

#include <fgl/debug/output.hpp>

/*
This example shows a more complex usecase of sending a formatted string
representation of a template type to the libFGL debug output stream. It
demonstrates how to define an `output_channel`, `output_formatter`, and then
specializes the `output_config` template in a way which satisfies the
output_handler concept, by abstracting the previously mentioned channel and
formatter.

The main function demonstrates sending output to the debug output stream.

I define the entity_channel separately from the formatter because the formatter
is a template. There can be many formatters, each corresponding to the type of
entity. However, there should only be one entity channel so that if I were to
call `entity_channel::turn_off()` in the main function, it would affect output
of all entity types regardless of the template-synthesized type.

This is the same reason that you wouldn't want to use the generic output
handler `fgl::debug::output_config<T>` for a user-defined template type:
`output_config<entity<int>>` and `output_config<entity<double>>` would be
distinct channels. By defining our own, we have can better express our intent
and desired behavior: "this is the single channel for all entity types". Note:
this would also let us specialize the formatter for each synthesis of the type.
*/

// the user defined type, which is a template in this case.
template <typename T_position = double>
struct entity
{
	T_position position[3];
	std::string name;
};

// an output channel for the user defined type
class entity_channel
{
	static inline bool m_enabled{ true };
	public:
	// should never be instantiated
	entity_channel(auto&&...) = delete;

	// members to satisfy the output_channel concept
	static void turn_on() noexcept { m_enabled = true; }
	static void turn_off() noexcept { m_enabled = false; }
	[[nodiscard]] static bool enabled() noexcept { return m_enabled; }
	[[nodiscard]] static std::string_view name() noexcept { return "Entity"; }

	[[nodiscard]] static fgl::debug::priority priority_level() noexcept
	{ return fgl::debug::priority::debug; }
};

// make sure we satisfy the output_channel concept
static_assert(fgl::debug::output_channel<entity_channel>);

// an output formatter for the user defined type (a template in this case)
template <typename T_position>
struct entity_formatter
{
	entity_formatter(auto&&...) = delete;
	[[nodiscard]] static std::string format(const entity<T_position>& entity)
	{
		std::ostringstream ss;
		ss
			<< entity.name << " at coordinates ("
			<< entity.position[0] << ","
			<< entity.position[1] << ","
			<< entity.position[2] << ")";
		return ss.str();
	}
};

// make sure we satisfy the output_formatter concept
static_assert(
	fgl::debug::output_formatter<entity_formatter<double>, entity<double>>
);

/*
With the above definitions in place, we can use the explcitly specified
`fgl::debug::output::custom<T, T_channel, T_formatter>(t)` member to send the
output to the stream but this can be quite verbose. Ideally, we would use the
combined `fgl::debug::output::handled<T, T_handler>(t)` member.

Better yet, if we specialize `fgl::debug::output_config` as a handler, we can
use the call operator `fgl::debug::output(t)` which is much more concise!
*/

template <typename T_position>
class fgl::debug::output_config<entity<T_position>>
{
	public:
	// redirect so all synethized output_config<T> types use the same channel.
	static void turn_on() { entity_channel::turn_on(); }
	static void turn_off() { entity_channel::turn_off(); }
	static bool enabled() { return entity_channel::enabled(); }
	static std::string_view name() { return entity_channel::name(); }

	static fgl::debug::priority priority_level()
	{ return entity_channel::priority_level(); }

	// call the appropriate formatter for entity<T_position>
	static std::string format(const entity<T_position>& entity)
	{ return entity_formatter<T_position>::format(entity); }
};

// make sure our specialization satisfies the output_handler concept
static_assert(
	fgl::debug::output_handler<
		fgl::debug::output_config<entity<double>>,
		entity<double>
	>
);

////////////////
/// Entry Point
////////

int main()
{
	entity<double> player1{ {4.4, 3.3, 2.2}, "Lexy" };
	entity<float> player2{ {3, 5, 10}, "Nakamura" };
	entity<int> player3{ {100, 50000, 200}, "Eralia" };

	// set the output stream (redundant; defaults to std::cout)
	fgl::debug::output::stream = std::cout;

	// channel priority must be >= output threshold to display output
	// note: defaults to priority::minimum, entity_channel is priority::debug
	fgl::debug::output::priority_threshold = entity_channel::priority_level();

	// channel enabled() must return true to display output
	entity_channel::turn_on(); // redundant in this case, defaults to enabled

	// note: '\n' is appended to the stream following the formatted string

	// output player entity
	fgl::debug::output::custom<
		entity<double>,
		entity_channel,
		entity_formatter<double>
	>(player1);

	// alternatively, since we satified the `output_handler` concept
	using p2_handler_t = fgl::debug::output_config<entity<float>>;
	fgl::debug::output::handled<entity<float>, p2_handler_t>(player2);

	// but, even better since we specialized `output_config` as our handler
	fgl::debug::output(player3); // note: uses the <char> specialized formatter

	// make sure we flush the stream since the program terminates after this
	fgl::debug::output::stream.flush();
}
