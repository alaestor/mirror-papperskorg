#include <string>
#include <utility>
#include <concepts>


// this works too
//template <class T>
//concept ConvertableToString = std::convertible_to<T, std::string>;

// https://www.youtube.com/watch?v=PNRju6_yn3o

class Customer
{
	std::string m_first;
	std::string m_last;
	int m_id;

	public:
	template <typename S1, typename S2 = std::string>
	requires
		std::convertible_to<S1, std::string>
		&& std::convertible_to<S2, std::string>
	[[nodiscard]]
	Customer(S1&& first, S2&& last = "", const int id = 0)
	:
		m_first(std::forward<S1>(first)),
		m_last(std::forward<S2>(last)),
		m_id(id)
	{}
};

int main(void)
{
	std::string str{"first"};

	Customer a{ "t1", "t2", 1 };
	Customer b{ str, "t2", 1 };
	Customer c{ std::move(str), "t2", 1 };
	Customer d{ "t1" };
	Customer e = "t1";
	Customer f(e);

	// why use std::convertible_to and not std::constructable_from ?
	// convertible_to handles that implicitly
	auto test{ static_cast<std::string>("hello") };
	static_assert(std::same_as<decltype(test), std::string>);

	return 0;
}
