

#include "vml2.hpp"
#include "ctcontainers.hpp"
//#include "../external/compile-time-regular-expressions/single-header/ctre.h"
#include <ctre.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <string_view>
#include <ranges>
#include <charconv>

// C++23's constexpr from_char demo / C++26's operator bool() demo:
// std::optional<int64_t> to_int(const std::string_view s)
// {
// 	int64_t value{};
// 	if (std::from_chars(s.cbegin(), s.cend(), value).ec == std::errc{})
// 		return value;
// 	else
// 		return std::nullopt;
// };

// template <fgl::traits::numeric_type T>
// bool within_limits(const fgl::traits::numeric_type auto value)
// {
// 	using lim = std::numeric_limits<T>;
// 	const T v{ static_cast<T>(value) };
// 	return v >= lim::min() && v <= lim::max();
// }

// op_param_t get_valid_literal(const op_param_t value, std::size_t oplen)
// {
// 	const bool literal_in_bounds {
// 		[](const std::size_t l, const op_param_t v) -> bool
// 		{
// 			switch (l)
// 			{
// 			case 1: return within_limits<std::uint8_t>(v);
// 			case 2: return within_limits<std::uint16_t>(v);
// 			case 3: return within_limits<std::uint32_t>(v);
// 			case 4: return within_limits<std::uint64_t>(v);
// 			default: return false;
// 			}
// 		} (oplen, value)
// 	};
// 	assert(literal_in_bounds);
// 	return std::bit_cast<op_param_t>(value);
// }

struct vector_writer : public std::vector<std::byte>
{
	vector_writer(const std::size_t reserved = 10'000)
	{ reserve(reserved); }

	template <typename T>
	void write(const T value)
	{
		for (const auto b : std::bit_cast<std::array<std::byte, sizeof(T)>>(value))
			push_back(value);
	}

	template <typename T>
	void write(const T value, std::size_t at_index)
	{
		const auto a{ std::bit_cast<std::array<std::byte, sizeof(T)>>(value) };
		std::ranges::copy(a, &at(at_index));
	}

	void advance(const std::size_t bytes)
	{ resize(size() + bytes); }
};


struct ctstring_hasher {
	constexpr std::size_t operator()(const std::string_view s)
	{
		const int p = 31;
		const int m = 1e9 + 9;
		std::int64_t hash_value = 0;
		std::int64_t p_pow = 1;
		for (const char c : s) {
			hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
			p_pow = (p_pow * p) % m;
		}
		return static_cast<std::size_t>(hash_value);
	}
};

// template <typename T_key, typename T_value, typename T_hasher = ctstring_hasher>
// using unordered_map = ctumap<T_key, T_value, T_hasher>;

constexpr auto assemble(std::string_view source)
{
	(void) source; // TODO: remove
	struct locations {
		std::optional<op_param_t> definition;
		std::vector<op_param_t> usages;
	};

	ctumap<std::string, locations, ctstring_hasher> labels;
	// std::unordered_map<std::string, locations> labels;
	vector_writer bytecode;

	// groups 0:comment 1:label 2:instruction 3:op1 4:op2
	static constexpr auto matcher{
		ctre::match<"(?:;.+)|(?:\\s*(\\w+):)|(?:\\s*(\\w+)\\s*(\\w+)?(?:\\s*,\\s*(\\w+))?)">
	};

	/*
		const auto pattern {
			std::regex( // TODO use a constexpr regex
				"(?:;.+)|(?:\\s*(\\w+):)|(?:\\s*(\\w+)\\s*(\\w+)?(?:\\s*,\\s*(\\w+))?)",
				std::regex::optimize
			)
		};

		for (
			auto ri { std::cregex_iterator{ source.cbegin(), source.cend(), pattern } };
			ri != std::cregex_iterator{};
			++ri
		){
			std::cmatch cm{ *ri };

			if (cm[1].matched)
			{ // label definition
				const std::string name{ cm[1].str() };
				assert(!labels.contains(name)); // ODR
				labels[name] = locations{ bytecode.size(), {} };
			}
			if (cm[2].matched)
			{ // opcode
				const auto operation {
					[](const auto&& s)
					{
						auto opt{ wise_enum::from_string<instruction>(s) };
						if (!opt) throw -1; // invalid op, got s
						return *opt;
					}(cm[2].str())
				};

				// append opcode
				bytecode.push_back(static_cast<std::byte>(std::to_underlying(operation)));
				switch (operation) {
				{
					std::int64_t oplen{ 0 };
				/// instructions with two operands
				case instruction::movb:
				case instruction::addb:
				case instruction::subb:
				case instruction::mulb:
				case instruction::divb:
					oplen = 1;
					goto cm4;
				case instruction::movw:
				case instruction::addw:
				case instruction::subw:
				case instruction::mulw:
				case instruction::divw:
					oplen = 2;
					goto cm4;
				case instruction::movl:
				case instruction::addl:
				case instruction::subl:
				case instruction::mull:
				case instruction::divl:
					oplen = 3;
					goto cm4;
				case instruction::movq:
				case instruction::addq:
				case instruction::subq:
				case instruction::mulq:
				case instruction::divq:
					oplen = 4;
					cm4: // TODO RENAME

					assert(cm[4].matched); // TODO: better

					// /// <get OP 2>
					// std::string_view op2{ cm[4].first, cm[4].second };

					// // if opstr is int literal
					// if (const std::optional<op_param_t> value_opt{ to_int(op2) }; value_opt)
					// { // is int
					// 	variables.emplace_back("", oplen, get_valid_literal(*value_opt, oplen));
					// }
					// else // is str
					// {
					// 	std::string variable_name{ op2 };

					// 	if (const auto vnm{ variable_name_mapping.find(variable_name) };
					// 		vnm != variable_name_mapping.end())
					// 	{
					// 		// wrong
					// 		for (const auto b : std::bit_cast<std::array<std::byte, 8>>(vnm->second))
					// 			bytecode.push_back(b);
					// 	}
					// 	else
					// 	{

					// 	}
					// }

					// add 4,ebx
				}
				/// instructions with one operand
				[[fallthrough]] case instruction::jmp:
					if (!cm[3].matched)
						throw -1;
					std::string_view op1 { cm[3].first, cm[3].second };

				/// instructions with no operands
				break; case instruction::hcf: break;
				}

			}
		}
	//*/
}


#include <iostream>
int main()
{
	/*
	assemble(
		"jmp aplace\n"
		"movq myvar, data\n"
		"addq myvar, data2 ; Aadd my thingy\n"
		"; MY RESULT!\n"
		"mulw myvar, ; not allowed data2\n"
		"label: ; labels allowed\n"
		"mulw myvar, data2\n"
		"hcf\n"
		"; arbitrary, spacing doesn't matter\n"
		"\n"
		"data: dq 314159\n"
		"data2: dq 6432432\n"
		"myvar: dq ?;           valid comment, no data after means uninitialized\n"
	);//*/
}

