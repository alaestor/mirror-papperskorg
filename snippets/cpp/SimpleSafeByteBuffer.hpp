#pragma once
#ifndef SIMPLESAFEBYTEBUFFER_H_INCLUDED
#define SIMPLESAFEBYTEBUFFER_H_INCLUDED

#include <utility> // move
#include <memory> // unique_ptr
#include <cstddef> // byte size_t ptrdiff_t
#include <stdexcept>

namespace xfile {
namespace internal {

class SimpleSafeByteBuffer
{
	const std::size_t m_length;
	std::size_t m_cursor{ 0 };
	std::unique_ptr<std::byte[]> m_data_uptr;
	
	public:

	[[nodiscard]] const std::byte* get_raw() const noexcept;
	[[nodiscard]] std::size_t size() const noexcept;
	[[nodiscard]] std::size_t capacity() const noexcept;
	[[nodiscard]] std::size_t capacity_remaining() const noexcept;
	
	void clear() noexcept;
	void write(const std::byte* data, const std::size_t length);
	void read(
		std::byte* out_buffer,
		const std::size_t from,
		const std::size_t length) const;
	
	
	
	// Constructors
	explicit SimpleSafeByteBuffer(std::size_t length)
	: m_length(length), m_data_uptr(new std::byte[length])
	{
		if (m_length == 0)
			throw std::invalid_argument("Length must be > 0");
	}
	
	SimpleSafeByteBuffer(SimpleSafeByteBuffer&& o) noexcept
	:
		m_length(o.m_length),
		m_cursor(o.m_cursor),
		m_data_uptr(std::move(o.m_data_uptr))
	{}
	
	SimpleSafeByteBuffer(const SimpleSafeByteBuffer&) = delete;
	
	
	
	// Destructor
	~SimpleSafeByteBuffer() = default;
	
	
	
	// Operators
	SimpleSafeByteBuffer& operator=(const SimpleSafeByteBuffer&) = delete;
};

}// namespace internal
}// namespace xfile

#endif // SIMPLESAFEBYTEBUFFER_H_INCLUDED