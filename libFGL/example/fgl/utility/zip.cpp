/**
This file is an example for <fgl/utility/zip.hpp>

--- Example output
-------------------------------------------------------------------------------
110
3.14 += 1 + 1.1 = 5.24
3.14 += 2 + 2.2 = 7.34
3.14 += 3 + 3.3 = 9.44
*/

#include <iostream>
#include <array>
#include <vector>
#include <map>

#include <fgl/utility/zip.hpp>

consteval int calc()
{
	int sum{ 0 };
	std::array a1{ 0,1,2,3,4 }; // note: this is the shortest range
	std::array a2{ 5,6,7,8,9,10,2000,4500001 };
	std::array a3{ 11,12,13,14,15,16,251000,4500,145101 };
	for (const auto& [a1_v, a2_v, a3_v] : fgl::czip(a1, a2, a3))
		sum += a1_v + a2_v + a3_v;
	return sum;
}

int main()
{
	constexpr auto result{ calc() };
	std::cout << result << '\n';

	std::vector<float> vec(20, 3.14f);
	std::map<int, float> map{ {1, 1.1f}, {2, 2.2f}, {3, 3.3f} };
	for (const auto& [v1, m, v2] : fgl::zip(vec, map, vec))
	{
		const auto& [m_k, m_v]{ m }; // cant nest structured bindings
		std::cout<< v1 << " += " << m_k << " + " << m_v << " = ";
		v1 += static_cast<float>(m_k) + m_v;
		std::cout << v2 << '\n'; // same element as v1
	}
	std::cout << std::endl;
}
