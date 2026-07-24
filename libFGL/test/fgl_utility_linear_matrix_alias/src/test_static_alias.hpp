#pragma once
#ifndef TESTS_FGL_UTILITY_LINEAR_MATRIX_STATIC_HPP_INCLUDED
#define TESTS_FGL_UTILITY_LINEAR_MATRIX_STATIC_HPP_INCLUDED

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/linear_matrix_alias/static.hpp>
#include "./utility.hpp"

#include <concepts> // same_as

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

namespace test_static_alias {

template <fgl::major T_major, std::size_t ... T_bounds>
constexpr void common_test(auto alias)
{
	// test static members
	fgl::static_linear_matrix_alias<int*, T_major, T_bounds...> a{ nullptr };
	static_assert(!std::same_as<decltype(a), decltype(alias)>);
	constexpr_assert(&alias.bounds() == &a.bounds());
	constexpr_assert(&alias.offsets() == &a.offsets());
	constexpr_assert(alias.dimensions() == 4);
	constexpr_assert(alias.get_major() == T_major);
	if constexpr (T_major == fgl::major::row)
	{ constexpr_assert(alias.is_row_major() && !alias.is_column_major()); }
	else
	{ constexpr_assert((!alias.is_row_major()) && alias.is_column_major()); }
	// iterator getter / setter
	constexpr_assert(alias.iterator() != nullptr);
	auto orig{ alias.iterator() };
	alias.set_iterator(decltype(alias.iterator()){ nullptr });
	constexpr_assert(alias.iterator() == nullptr);
	alias.set_iterator(orig);
	constexpr_assert(alias.iterator() != nullptr);
}

constexpr bool test_major_row()
{
	using enum fgl::major;
	constexpr std::size_t x{5}, y{3}, z{4}, w{2};
	constexpr std::array<char, x*y*z*w> buffer{};
	const auto alias = fgl::make_matrix_alias<row, x,y,z,w>(buffer);
	static_assert(
		std::same_as<
			decltype(alias),
			const fgl::static_linear_matrix_alias<const char*, row, x, y, z, w>
		>
	);
	{ // <index behavior>
		const char* last_address{ buffer.data() - 1 };
		iterate_bounds<x,y,z,w>(
			[&](const std::array<std::size_t, 4>& indicies)
			{
				const char* const current_address{ &alias.at(indicies) };
				constexpr_assert(current_address == &alias[indicies]);
				constexpr_assert(current_address == last_address + 1);
				last_address = current_address;
			}
		);
		constexpr_assert(last_address == &buffer.back())
	} // </index behavior>
	common_test<row, x,y,z,w>(alias);
	return true;
}

template <fgl::major T_major, std::size_t ... T_valid_bounds>
[[nodiscard]] bool test_exceptions_impl(auto& invalid_sized_range) try
{
	// ctor exception
	got_expected_exception((
		(void)fgl::make_matrix_alias
			<T_major, T_valid_bounds...>(invalid_sized_range)
	));
	return true;
}
catch( ... )
{ return false; }

template <std::size_t T_offset, std::size_t ... T_valid_bounds>
bool test_exceptions()
{
	using enum fgl::major;
	constexpr std::array<char, (T_valid_bounds * ...)-T_offset> too_small{};
	constexpr std::array<char, (T_valid_bounds * ...)+T_offset> too_big{};
	assert((test_exceptions_impl<row, T_valid_bounds...>(too_small)));
	assert((test_exceptions_impl<column, T_valid_bounds...>(too_small)));
	assert((test_exceptions_impl<row, T_valid_bounds...>(too_big)));
	assert((test_exceptions_impl<column, T_valid_bounds...>(too_big)));
	return true;
}

constexpr bool test_major_column()
{
	using enum fgl::major;
	using T = unsigned short;
	constexpr std::size_t x{2}, y{3}, z{4}, w{5}, g{6};
	std::array<T, x*y*z*w*g> buffer{ 0 };
	auto alias = fgl::make_matrix_alias<column, x,y,z,w,g>(buffer);
	{ // <index behavior>
		unsigned short counter{ 0 };
		iterate_bounds<x,y,z,w,g>(
			[&](const std::array<std::size_t, 5>& indicies)
			{
				auto& r{ alias[indicies] };
				constexpr_assert(&alias.at(indicies) == &r);
				r = counter++;
			}
		);
	} // </index behavior>
	constexpr std::array<unsigned short, 720> expected{ 0, 360, 120, 480, 240, 600, 30, 390, 150, 510, 270, 630, 60, 420, 180, 540, 300, 660, 90, 450, 210, 570, 330, 690, 6, 366, 126, 486, 246, 606, 36, 396, 156, 516, 276, 636, 66, 426, 186, 546, 306, 666, 96, 456, 216, 576, 336, 696, 12, 372, 132, 492, 252, 612, 42, 402, 162, 522, 282, 642, 72, 432, 192, 552, 312, 672, 102, 462, 222, 582, 342, 702, 18, 378, 138, 498, 258, 618, 48, 408, 168, 528, 288, 648, 78, 438, 198, 558, 318, 678, 108, 468, 228, 588, 348, 708, 24, 384, 144, 504, 264, 624, 54, 414, 174, 534, 294, 654, 84, 444, 204, 564, 324, 684, 114, 474, 234, 594, 354, 714, 1, 361, 121, 481, 241, 601, 31, 391, 151, 511, 271, 631, 61, 421, 181, 541, 301, 661, 91, 451, 211, 571, 331, 691, 7, 367, 127, 487, 247, 607, 37, 397, 157, 517, 277, 637, 67, 427, 187, 547, 307, 667, 97, 457, 217, 577, 337, 697, 13, 373, 133, 493, 253, 613, 43, 403, 163, 523, 283, 643, 73, 433, 193, 553, 313, 673, 103, 463, 223, 583, 343, 703, 19, 379, 139, 499, 259, 619, 49, 409, 169, 529, 289, 649, 79, 439, 199, 559, 319, 679, 109, 469, 229, 589, 349, 709, 25, 385, 145, 505, 265, 625, 55, 415, 175, 535, 295, 655, 85, 445, 205, 565, 325, 685, 115, 475, 235, 595, 355, 715, 2, 362, 122, 482, 242, 602, 32, 392, 152, 512, 272, 632, 62, 422, 182, 542, 302, 662, 92, 452, 212, 572, 332, 692, 8, 368, 128, 488, 248, 608, 38, 398, 158, 518, 278, 638, 68, 428, 188, 548, 308, 668, 98, 458, 218, 578, 338, 698, 14, 374, 134, 494, 254, 614, 44, 404, 164, 524, 284, 644, 74, 434, 194, 554, 314, 674, 104, 464, 224, 584, 344, 704, 20, 380, 140, 500, 260, 620, 50, 410, 170, 530, 290, 650, 80, 440, 200, 560, 320, 680, 110, 470, 230, 590, 350, 710, 26, 386, 146, 506, 266, 626, 56, 416, 176, 536, 296, 656, 86, 446, 206, 566, 326, 686, 116, 476, 236, 596, 356, 716, 3, 363, 123, 483, 243, 603, 33, 393, 153, 513, 273, 633, 63, 423, 183, 543, 303, 663, 93, 453, 213, 573, 333, 693, 9, 369, 129, 489, 249, 609, 39, 399, 159, 519, 279, 639, 69, 429, 189, 549, 309, 669, 99, 459, 219, 579, 339, 699, 15, 375, 135, 495, 255, 615, 45, 405, 165, 525, 285, 645, 75, 435, 195, 555, 315, 675, 105, 465, 225, 585, 345, 705, 21, 381, 141, 501, 261, 621, 51, 411, 171, 531, 291, 651, 81, 441, 201, 561, 321, 681, 111, 471, 231, 591, 351, 711, 27, 387, 147, 507, 267, 627, 57, 417, 177, 537, 297, 657, 87, 447, 207, 567, 327, 687, 117, 477, 237, 597, 357, 717, 4, 364, 124, 484, 244, 604, 34, 394, 154, 514, 274, 634, 64, 424, 184, 544, 304, 664, 94, 454, 214, 574, 334, 694, 10, 370, 130, 490, 250, 610, 40, 400, 160, 520, 280, 640, 70, 430, 190, 550, 310, 670, 100, 460, 220, 580, 340, 700, 16, 376, 136, 496, 256, 616, 46, 406, 166, 526, 286, 646, 76, 436, 196, 556, 316, 676, 106, 466, 226, 586, 346, 706, 22, 382, 142, 502, 262, 622, 52, 412, 172, 532, 292, 652, 82, 442, 202, 562, 322, 682, 112, 472, 232, 592, 352, 712, 28, 388, 148, 508, 268, 628, 58, 418, 178, 538, 298, 658, 88, 448, 208, 568, 328, 688, 118, 478, 238, 598, 358, 718, 5, 365, 125, 485, 245, 605, 35, 395, 155, 515, 275, 635, 65, 425, 185, 545, 305, 665, 95, 455, 215, 575, 335, 695, 11, 371, 131, 491, 251, 611, 41, 401, 161, 521, 281, 641, 71, 431, 191, 551, 311, 671, 101, 461, 221, 581, 341, 701, 17, 377, 137, 497, 257, 617, 47, 407, 167, 527, 287, 647, 77, 437, 197, 557, 317, 677, 107, 467, 227, 587, 347, 707, 23, 383, 143, 503, 263, 623, 53, 413, 173, 533, 293, 653, 83, 443, 203, 563, 323, 683, 113, 473, 233, 593, 353, 713, 29, 389, 149, 509, 269, 629, 59, 419, 179, 539, 299, 659, 89, 449, 209, 569, 329, 689, 119, 479, 239, 599, 359, 719 };
	constexpr_assert(buffer == expected);
	return true;
}

bool test_passes()
{
	static_assert(test_major_row());
	static_assert(test_major_column());
	assert((test_exceptions<1, 2,2>()));
	assert((test_exceptions<4, 2,2>()));
	assert((test_exceptions<1, 3,3,3>()));
	assert((test_exceptions<27, 3,3,3>()));
	return true;
}

} // namespace test_static_alias

#endif // TESTS_FGL_UTILITY_LINEAR_MATRIX_STATIC_HPP_INCLUDED
