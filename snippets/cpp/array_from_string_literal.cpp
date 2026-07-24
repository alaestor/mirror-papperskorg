#include <array>
#include <utility> // index_sequence, make_index_sequence

template<std::size_t LEN> [[nodiscard]]
consteval auto make_array(const char (&cstr)[LEN])
{
	constexpr std::size_t arrayLength{ LEN-1 }; // no null term
	return
		[&]<std::size_t ... I>(std::index_sequence<I...>)
		consteval -> std::array<const char, arrayLength>
		{
			return {{cstr[I]...}};
		}(std::make_index_sequence<arrayLength>());
}

consteval auto consteval_f()
{
	return make_array("test");
}
