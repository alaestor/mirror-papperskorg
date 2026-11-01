#ifndef FGL_TYPES_FLAGGED_POINTER_HPP_INCLUDED
#define FGL_TYPES_FLAGGED_POINTER_HPP_INCLUDED

#include <cstddef>
#include <memory>

#include "../math/sqrt.hpp"

static_assert(
    (std::uintptr_t{}, true),
    "flagged_pointer: incompatible STL implementation"
);

static_assert(
    sizeof(void*) == sizeof(std::byte*),
    "flagged_pointer: incompatible environment"
);

static_assert(
    sizeof(void*) == sizeof(void(*)()),
    "flagged_pointer: incompatible environment"
);

namespace fgl {

/**
@brief A pointer whose least significant bits are encoded with boolean state
@details Flagged pointers use the least significant bit of a pointer to encode
	boolean state. It requires that the square root of the alignment of the
	underlying type be greater than or equal to the number of bits used for
	encoding.
@warning Alignment and pointer representation may differ on some
	platforms and implementations; therefore, usage of flagged pointers is
	discouraged as it's not portable.
@tparam T The pointer type
@tparam T_bits The number of bits used to encode the boolean state; must be
	less than or equal to the square root of the alignment of T
*/
template <typename T, std::size_t T_bits = 1>
requires (T_bits < fgl::sqrt(alignof(std::remove_pointer_t<T>)))
class flagged_pointer
{
    std::uintptr_t m_manipulated_pointer;

    [[nodiscard]] static consteval std::uintptr_t all_mask() noexcept
    {
        std::uintptr_t mask{ 0 };
        for (std::size_t i{ 0 }; i < T_bits; ++i)
            mask |= (1ull << i);
        return mask;
    }

public:

    [[nodiscard]] explicit
    flagged_pointer(T pointer)
    : m_manipulated_pointer(reinterpret_cast<std::uintptr_t>(pointer))
    { assert((m_manipulated_pointer & all_mask()) == 0); }
};

} // namespace fgl

/*
namespace fgl {

static constexpr bool OddPointerSafetyChecks = false;

template <class T, class Deleter = std::default_delete< T >>
class flagged_unique_ptr // lacks support for std::make_unique
{
	public:
	using value_type = T;
	using this_type = flagged_unique_ptr<value_type, Deleter>;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type*;
	using reference = value_type&;

	private:
	std::unique_ptr<value_type, Deleter> m_uptr;

	public:
	bool flag() const noexcept
	{ return (reinterpret_cast< difference_type >(m_uptr.get()) & 1); }

	bool setflag(const bool& state)
	{
		if (state)
		{
			const auto& addr = m_uptr.release();
			m_uptr.reset(reinterpret_cast< pointer >(
				reinterpret_cast< difference_type >(addr) | 1));
		}
		else
		{
			const auto& addr = m_uptr.release();
			m_uptr.reset(reinterpret_cast< pointer >(
				reinterpret_cast< difference_type >(addr) & ~1));
		}
		return state;
	}

	void toggleflag() noexcept
	{
		const auto& addr = m_uptr.release();
		m_uptr.reset(reinterpret_cast< pointer >(
			reinterpret_cast< difference_type >(addr) ^ 1));
	}

	pointer get() const noexcept
	{
		return reinterpret_cast< pointer >( // corrects address
			reinterpret_cast< difference_type >(m_uptr.get()) & ~1);
	}

	pointer release() noexcept // preserves flag state
	{
		const auto& oldFlagState = flag();
		setflag(false); // corrects address
		const auto& p = m_uptr.release();
		setflag(oldFlagState);
		return p;
	}

	void reset(const pointer& p = nullptr) noexcept // sets flag to false
	{
		setflag(false);
		m_uptr.reset(p);
	}

	void swap(decltype(m_uptr)& up) noexcept // sets flag to false
	{
		setflag(false); // corrects address
		m_uptr.swap(up);
	}

	void swap(this_type& fup) noexcept // preserves flag state
	{ m_uptr.swap(fup.m_uptr); }

	Deleter& get_deleter() noexcept
	{ return m_uptr.get_deleter(); }

	const Deleter& get_deleter() const noexcept
	{ return m_uptr.get_deleter(); }


	// container comparative operators
	bool operator==(const decltype(m_uptr)& up) const noexcept
	{ return m_uptr == up; }

	bool operator!=(const decltype(m_uptr)& up) const noexcept
	{ return !operator==(up); }

	bool operator==(const this_type& fup) const noexcept
	{ return m_uptr == fup.m_uptr; }

	bool operator!=(const this_type& fup) const noexcept
	{ return !operator==(fup); }


	// assignment
	this_type& operator=(this_type& fup) noexcept
	{
		reset(fup.m_ptr.release());
		return *this;
	}

	bool operator=(const bool& b) noexcept
	{ return setflag(b); }


	// pointer arithmatic operators
	pointer operator+(const difference_type& i) const noexcept
	{ return get() + i; }

	pointer operator-(const difference_type& i) const noexcept
	{ return get() - i; }

	difference_type operator-(const pointer& p) const noexcept // type?
	{ return get() - p; }


	// pointer dereference operators
	pointer operator->() const noexcept
	{ return (get()); }

	reference operator*()
	{ return *(get()); }

	reference operator[](const difference_type& i)
	{ return get()[i]; }

	const reference operator*() const
	{ return *(get()); }

	const reference operator[](const difference_type& i) const
	{ return get()[i]; }


	// pointer comparative operators
	bool operator<(const pointer& p) const noexcept
	{ return get() < p; }

	bool operator>(const pointer& p) const noexcept
	{ return get() > p; }

	bool operator<=(const pointer& p) const noexcept
	{ return get() <= p; }

	bool operator>=(const pointer& p) const noexcept
	{ return get() >= p; }

	bool operator==(const pointer& p) const noexcept
	{ return get() == p; }

	bool operator!=(const pointer& p) const noexcept
	{ return !operator==(p); }


	// boolean operators (affect flag)
	bool operator==(const bool& b) const noexcept
	{ return flag() == b; }

	bool operator!=(const bool& b) const noexcept
	{ return !operator==(b); }


	// conversion operators
	explicit operator pointer() const noexcept
	{ return get(); }

	explicit operator bool() const noexcept // unique_ptr behavior
	{ return get() != nullptr; }


	// stop annoying implicit conversions
	void operator=(const int&) = delete; // stop implicit int->bool convert.
	void operator=(const difference_type&) = delete; // stop (this = &) convert
	void operator=(const pointer&) = delete; // stop (this = &) convert


	flagged_unique_ptr(pointer ptr = nullptr)
	: m_uptr(ptr)
	{
		static_assert(alignof(value_type) >= 2, //old: sizeof(T) > sizeof(char)
			"FGL::FlaggedPointer: Type size must be aligned by >= 2");

		if constexpr (OddPointerSafetyChecks)
			if (reinterpret_cast< difference_type >(ptr) & 1)
				throw "constructed with odd address! Weird enviroment!";

	}

	~flagged_unique_ptr() // corrects address so unique_ptr can do it's thing
	{ setflag(false); }
};

template <typename T>
class flagged_ptr : std::random_access_iterator_tag
{
	public:
	using value_type = T;
	using this_type = flagged_ptr<value_type>;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = std::random_access_iterator_tag;

	private:
	pointer m_address;

	protected:
	pointer assign(const pointer& p) noexcept
	{
		const auto& oldFlagState = flag();
		m_address = p;
		setflag(oldFlagState);
		return p;
	}

	public:
	bool flag() const noexcept
	{ return (reinterpret_cast< difference_type >(m_address) & 1); }

	bool setflag(const bool& state) noexcept
	{
		if (state)
		{
			m_address = reinterpret_cast< pointer >(
				reinterpret_cast< difference_type >(m_address) | 1);
		}
		else
		{
			m_address = reinterpret_cast< pointer >(
				reinterpret_cast< difference_type >(m_address) & ~1);
		}
		return state;
	}

	void toggleflag() noexcept
	{
		m_address = reinterpret_cast< pointer >(
			reinterpret_cast< difference_type >(m_address) ^ 1);
	}

	pointer get() const noexcept
	{
		return reinterpret_cast< pointer >(
			reinterpret_cast< difference_type >(m_address) & ~1);
	}


	// container operators
	bool operator==(const this_type& fp) const noexcept
	{ return m_address == fp.m_address; }

	bool operator!=(const this_type& fp) const noexcept
	{ return !operator==(fp.m_address); }


	// assignment
	pointer operator=(const this_type& fp) noexcept
	{ return m_address = fp.m_address; }

	pointer operator=(const pointer& p) noexcept
	{ return assign(p); }

	bool operator=(const bool& b) noexcept
	{ return setflag(b); }


	// pointer arithmatic operators
	pointer operator+(const difference_type& i) const noexcept
	{ return get() + i; }

	pointer operator-(const difference_type& i) const noexcept
	{ return get() - i; }

	pointer operator*(const difference_type& i) const noexcept
	{ return get() * i; }

	pointer operator/(const difference_type& i) const noexcept
	{ return get() / i; }

	pointer operator%(const difference_type& i) const noexcept
	{ return get() % i; }

	pointer operator+=(const difference_type& i) noexcept
	{ return assign(operator+(i)); }

	pointer operator-=(const difference_type& i) noexcept
	{ return assign(operator-(i)); }

	pointer operator*=(const difference_type& i) noexcept
	{ return assign(operator*(i)); }

	pointer operator/=(const difference_type& i) noexcept
	{ return assign(operator/(i)); }

	pointer operator%=(const difference_type& i) noexcept
	{ return assign(operator%(i)); }

	pointer operator++() noexcept
	{ return operator+=(1); }

	pointer operator--() noexcept
	{ return operator-=(1); }

	pointer operator++(int) noexcept
	{
		const auto& p = get();
		operator++();
		return p;
	}

	pointer operator--(int) noexcept
	{
		const auto& p = get();
		operator--();
		return p;
	}

	difference_type operator-(const pointer& p) const noexcept
	{ return get() - p; }


	// pointer dereference operators
	pointer operator->() const noexcept
	{ return get(); }

	reference operator*()
	{ return *(get()); }

	reference operator[](const difference_type& i)
	{ return get()[i]; }

	const reference operator*() const
	{ return *(get()); }

	const reference operator[](const difference_type& i) const
	{ return get()[i]; }


	// pointer comparitive operators
	bool operator<(const pointer& p) const noexcept
	{ return get() < p; }

	bool operator>(const pointer& p) const noexcept
	{ return get() > p; }

	bool operator<=(const pointer& p) const noexcept
	{ return get() <= p; }

	bool operator>=(const pointer& p) const noexcept
	{ return get() >= p; }

	bool operator==(const pointer& p) const noexcept
	{ return get() == p; }

	bool operator!=(const pointer& p) const noexcept
	{ return !operator==(p); }


	// boolean operators (affect flag)
	bool operator=(const bool& b) const noexcept
	{ return setflag(b); }

	bool operator==(const bool& b) const noexcept
	{ return flag() == b; }

	bool operator!=(const bool& b) const noexcept
	{ return !operator==(b); }


	// conversion operators
	explicit operator pointer() const noexcept
	{ return get(); }


	// stop annoying implicit conversions
	void operator=(const int&) = delete; // stop implicit int->bool convert.
	void operator=(const difference_type&) = delete; // stop (this = &) convert


	flagged_ptr(T* ptr = nullptr)
	: m_address(ptr)
	{
		static_assert(alignof(value_type) >= 2, //old: sizeof(T) > sizeof(char)
			"FGL::FlaggedPointer: Type size must be aligned by >= 2");

		if constexpr (OddPointerSafetyChecks)
			if (reinterpret_cast< difference_type >(ptr) & 1)
				throw "constructed with odd address! Weird enviroment!";
	}

	~flagged_ptr() = default;
};

} // namespace fgl
//*/

#endif // FGL_TYPES_FLAGGED_POINTER_HPP_INCLUDED
