#include <memory> // unique_ptr
#include <cstddef> // byte, size_t
#include <stdexcept> // ...

#include "../hdr/SimpleSafeByteBuffer.hpp"

namespace xfile {
namespace internal {

using SSBB = SimpleSafeByteBuffer;

const std::byte* SSBB::get_raw() const noexcept
{ return m_data_uptr.get(); }

std::size_t SSBB::size() const noexcept
{ return m_cursor; }

std::size_t SSBB::capacity() const noexcept
{ return m_length; }

std::size_t SSBB::capacity_remaining() const noexcept
{ return capacity() - size(); }

void SSBB::write(
	const std::byte* data,
	const std::size_t length)
{
	if (data == nullptr)
		throw std::invalid_argument("recieved nullptr as data*");
	if (length == 0)
		throw std::invalid_argument("Length must be > 0");
	if (length > capacity_remaining())
		throw std::invalid_argument("length exceeds remaining capacity");
	
	// write to cursor
	// inc cursor
	//std::copy(begin(), mystring.end(), data);
	
}

void SSBB::read(
	std::byte* out_buffer,
	const std::size_t from,
	const std::size_t length) const
{
	if (out_buffer == nullptr)
		throw std::invalid_argument("recieved nullptr as out_buffer*");
	if (length == 0)
		throw std::invalid_argument("length must be > 0");
	if (from > size())
		throw std::invalid_argument("from exceeds current size");
	// TODO
}

void SSBB::clear() noexcept
{
	if (!m_data_uptr){}// throw
}

}// namespace internal
}// namespace xfile