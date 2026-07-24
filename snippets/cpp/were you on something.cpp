
//import std;
// god fucking damn it

//#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <algorithm>
#include <source_location>
#include <utility>
#include <functional>
#include <concepts>
#include <string>

/// exception helpers

#define THROW_SRC(exception) \
	throw exception(std::source_location::current().function_name())

#define THROW_SRC_MSG(exception, msg) \
	throw exception( \
		std::string{ std::source_location::current().function_name() } \
		+ "\n\tmsg: " + msg)

// #include <dolLib.hpp>

// <dolLib.hpp>
template <typename T>
[[nodiscard]] inline constexpr T fmaths_roundup_power2_ceiling(
	T const Value, T const Ceiling) noexcept
{ // round up to the ceiling power of 2
    T const Mask = Ceiling - 1;
    T RoundResult = Mask & Value;
    T TrueValue = Value & (~Mask);

    return RoundResult ? TrueValue + Ceiling : TrueValue;
}
// </dolLib.hpp>

#include <fgl.hpp>

inline std::size_t get_file_size(const std::filesystem::path& path)
{
	// cast isn't useless; std::uintmax_t may differ from std::size_t
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wuseless-cast"
	return static_cast<std::size_t>(std::filesystem::file_size(path));
	#pragma GCC diagnostic pop
}

using inst_t = std::uint8_t;
using mem_t = inst_t;
using ram_t = std::vector<mem_t>;
using offset_t = std::vector<mem_t>::difference_type;
using op_param_t = std::uint64_t;

enum class instruction : inst_t
{
	hcf,
	movb,
	movw,
	movl,
	movq,
	addb,
	addw,
	addl,
	addq,
	subb,
	subw,
	subl,
	subq,
	mulb,
	mulw,
	mull,
	mulq,
	divb,
	divw,
	divl,
	divq,
	jmp,
};

/*
	vml2 can be executed at runtime by loading a file from a path specified in
	the argument to main(), or at compile time by providing a byte range
	containing the executable [proc]ess (which would otherwise be the binary
	file contents at runtime).

	memory will be 16-byte aligned and a multiple of page_size, determined by
	the layout component plus some provisioning. Layout: [block][proc][pool]
	memory may be changed at runtime by the proc.

	[block] (or, the 'PEB') will contain things relating to machine control,
	such as the uint64_t instruction "pointer" (offset) which resides at +0.
	The PEB block is pad-aligned to 16-bytes.

	[proc] is the process which the vml2 will execute, starting at +the
	aligned size of the [block].

	[pool] is proc memory which resides past the end of the proc.

	each execution step will call `execute` which will interpret and perform
	the instruction stored at the offset indicated by the instruction pointer.
*/
struct state_machine
{
	ram_t memory;
	const std::size_t page_size;

	/// helpers for bounds-checking

	[[nodiscard]] constexpr bool
	is_out_of_bounds(const mem_t* const p) noexcept
	{ return p < memory.data() || p >= memory.data() + memory.size(); }

	[[nodiscard]] constexpr bool
	is_out_of_bounds(const std::size_t index) noexcept
	{ return index > memory.size(); }

	template <typename T>
	requires std::same_as<std::remove_const_t<T>, mem_t>
	[[nodiscard]] constexpr bool
	is_out_of_bounds(const std::span<T> r) noexcept
	{
		return r.data() < memory.data()
			|| r.data() + r.size() >= memory.data() + memory.size();
	}


	/// helpers for memory access


	/// rounds up size to nearest page_size^2 and returns the new size.
	constexpr std::size_t resize_memory(const std::size_t size)
	{
		const std::size_t s{ fmaths_roundup_power2_ceiling(size, page_size) };
		memory.resize(s);
		return s;
	}

	/// byte-copies T_value the location span
	template <typename T>
	constexpr void write(const T value, std::span<mem_t> location)
	{
		if (sizeof(T) > location.size()) THROW_SRC(std::out_of_range);
		const auto arr{ std::bit_cast<std::array<mem_t, sizeof(T)>>(value) };
		std::copy(arr.begin(), arr.end(), location.begin());
	}

	/// returns a T value interpretted from byte(s) starting at location
	template <typename T>
	[[nodiscard]] constexpr T read(const mem_t* const location)
	{
		const std::span s(location, location + sizeof(T));
		if (is_out_of_bounds(s)) THROW_SRC(std::out_of_range);
		std::array<mem_t, sizeof(T)> arr;
		std::copy(s.begin(), s.end(), arr.begin());
		return std::bit_cast<T>(arr);
	}

	/// like read(mem_t*) but the location is at an index
	template <typename T>
	[[nodiscard]] constexpr T read(const std::size_t index)
	{ return read<T>(memory.data() + index); }

	/// constrained specialization for single-byte types
	template <fgl::traits::byte_type T>
	[[nodiscard]] constexpr T read(const std::size_t index)
	{
		if (is_out_of_bounds(index)) THROW_SRC(std::out_of_range);
		return static_cast<T>(memory[index]);
	}


	/// constructors and initialization


	/// NOTE: this is to be shared between vml2 and the assembler
	// needed for (at least) absolute offsets
	[[nodiscard]] constexpr ram_t create_block()
	{
		const uint64_t block{ 16 }; // rounded to ceil of 16
		ram_t v{};
		v.resize(block);
		// placeholder for peb stuff
		write(block, std::span{ v.begin(), v.end() });
		return v;
	}

	[[nodiscard]] explicit constexpr state_machine(
		const ram_t& proc,
		const std::size_t page_size_ = 0x1000)
	: memory(create_block()), page_size(page_size_)
	{
		const offset_t block_size{ static_cast<offset_t>(memory.size()) };
		memory.resize(fmaths_roundup_power2_ceiling(
			memory.size() + proc.size(), page_size));
		std::copy(proc.begin(), proc.end(), memory.begin() + block_size);
	}

	[[nodiscard]] explicit state_machine(
		const std::filesystem::path path,
		const std::size_t page_size_ = 0x1000)
	: memory(create_block()), page_size(page_size_)
	{
		const std::size_t s{ get_file_size(path) };
		const auto block_size{ static_cast<offset_t>(memory.size()) };

		memory.resize(fmaths_roundup_power2_ceiling(memory.size() + s, page_size));
		std::span proc_range(memory.begin() + block_size, s);
		fgl::read_binary_file(path, proc_range, s);

		memory.resize(memory.size() * 2);
	}


	/// instructions


	/// TODO: C++23 replace with std::to_underlying
	[[nodiscard]] static constexpr std::underlying_type_t<instruction>
	inst_id(const instruction i) noexcept
	{ return static_cast<std::underlying_type_t<instruction>>(i); }


	constexpr void mov(
		const inst_t op,
		const op_param_t src,
		const op_param_t dst)
	{
		const uint8_t len{
			[op]() constexpr
			{
				if (const auto opdiff{ op - inst_id(instruction::movb) };
					opdiff <= 3)
				{
					return static_cast<uint8_t>(1 << opdiff);
				}
				else THROW_SRC(std::runtime_error);
			}()
		};

		const auto rc{ memory.begin() + static_cast<offset_t>(src) };
		std::copy(rc, rc + len, memory.begin() + static_cast<offset_t>(dst));
	}

	template <typename T, template<class U> class T_op> // no better way?...
	constexpr void math_int_impl_impl(
		const op_param_t src,
		const op_param_t dst)
	{
		write<T>(
			T_op<T>{}(read<T>(src), read<T>(dst)),
			std::span{ memory.begin() + static_cast<offset_t>(dst), sizeof(T) }
		);
	}

	template <template<class T> class T_op, instruction T_op_base>
	constexpr void math_int_impl(
		const inst_t op, const op_param_t src, const op_param_t dst)
	{
		// TODO find a way to correlate op_base with T_op in a concise way...
		// maybe a using with a ton of conditional template? lululul
		switch (constexpr inst_t op_base{ inst_id(T_op_base) }; op - op_base)
		{
		break;case 0: memory[dst] = T_op<uint8_t>{}(memory[src], memory[dst]);
		break;case 1: math_int_impl_impl<std::uint16_t, T_op>(src, dst);
		break;case 2: math_int_impl_impl<std::uint32_t, T_op>(src, dst);
		break;case 3: math_int_impl_impl<std::uint64_t, T_op>(src, dst);
		break;default: THROW_SRC(std::runtime_error);
		}
	}

	template <template<class T> class T_op, instruction T_op_base>
	constexpr void execute_math_impl(
		const std::uint64_t rip, const instruction opcode)
	{
		math_int_impl<T_op, T_op_base>(
			inst_id(opcode),
			read<op_param_t>(rip + 1),
			read<op_param_t>(rip + 1 + sizeof(op_param_t))
		);
	}

	/// executes the instruction specified by RIP. false on termination.
	[[nodiscard]] constexpr bool execute()
	{
		const auto rip = read<std::uint64_t>(memory.data());
		const instruction opcode{ memory[rip] };

		switch (opcode)
		{
		break;case instruction::hcf:
			return false;

		break;case instruction::movb:
		[[fallthrough]];case instruction::movw:
		[[fallthrough]];case instruction::movl:
		[[fallthrough]];case instruction::movq:
			mov(
				inst_id(opcode),
				read<op_param_t>(rip + 1),
				read<op_param_t>(rip + 1 + sizeof(op_param_t))
			);

		break;case instruction::addb:
		[[fallthrough]];case instruction::addw:
		[[fallthrough]];case instruction::addl:
		[[fallthrough]];case instruction::addq:
			execute_math_impl<std::plus, instruction::addb>(rip, opcode);

		break;case instruction::subb:
		[[fallthrough]];case instruction::subw:
		[[fallthrough]];case instruction::subl:
		[[fallthrough]];case instruction::subq:
			execute_math_impl<std::minus, instruction::subb>(rip, opcode);

		break;case instruction::mulb:
		[[fallthrough]];case instruction::mulw:
		[[fallthrough]];case instruction::mull:
		[[fallthrough]];case instruction::mulq:
			execute_math_impl<std::multiplies, instruction::mulb>(rip, opcode);

		break;case instruction::divb:
		[[fallthrough]];case instruction::divw:
		[[fallthrough]];case instruction::divl:
		[[fallthrough]];case instruction::divq:
			execute_math_impl<std::divides, instruction::divb>(rip, opcode);

		break;case instruction::jmp:
			write(
				read<op_param_t>(rip + 1),
				std::span{ memory.begin(), sizeof(op_param_t) }
			);

		break;default:
			THROW_SRC_MSG(std::invalid_argument, "unknown instruction");
		}
		return true;
	}
};

#include <iostream>

#define CEASE std::endl;

consteval bool f()
{
	std::vector<mem_t> v{ 10, 0 };
	state_machine sm(std::move(v));
	return sm.execute();
}

int main(
    [[maybe_unused]] const int argc,
    [[maybe_unused]] const char* const argv[])
{
	std::cout << "Hello, world!" << CEASE;

	// TODO:
	if (argc != 2)
		return -1;

	//state_machine sm(std::filesystem::path{ argv[1] });

	static_assert(f() == true);
}
