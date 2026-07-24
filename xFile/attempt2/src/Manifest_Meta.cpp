
#include <limits>
#include <stdexcept>
#include <cstddef> // std::byte
#include "../hdr/Manifest_Meta.h"

#define WINVER _WIN32_WINNT_WIN7
#include <windows.h> // system info

namespace xfile::internal {

struct PageSize
{
	static const std::size_t pageSize;

	[[nodiscard]] static std::size_t initPageSize()
	{ // for page-aligned internal buffer reservation size
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		return systemInfo.dwPageSize;
	}

	operator std::size_t() const noexcept { return pageSize; }
};

decltype(PageSize::pageSize) PageSize::pageSize{ PageSize::initPageSize() };

} // namespace xfile::internal



using Meta = xfile::internal::Meta;
using PageSize = xfile::internal::PageSize;


void Meta::resizeExact(const std::size_t size)
{
	// windows uses dword, must be compatible. TODO change except type?
	if (size > std::numeric_limits< unsigned long >::max()) throw
		std::runtime_error("xfile: resized() size is max unsigned long");
	m_buf.resize(size);
}

void Meta::resizeToPage(const std::size_t size)
{
	PageSize pageSize;
	if (size > 0)
	{ // reserve in page-sized chunks
		const std::size_t mask{ pageSize - 1 };
		const std::size_t trueValue{ size & (~mask) };
		const std::size_t roundResult{ size & mask };

		resizeExact(roundResult ? trueValue + pageSize : trueValue);
	} // reserve a single page for future use
	else resizeExact(pageSize);
}

void Meta::resize(const std::size_t size)
{ m_flags.resizeToPage ? resizeToPage(size) : resizeExact(size); }

void Meta::shrinkToFit() noexcept
{ m_buf.shrink_to_fit(); }

void Meta::eraseBuffer() noexcept
{ if (!m_buf.empty()) m_buf.erase(m_buf.cbegin(), m_buf.cend()); }


void Meta::freeBuffer() noexcept
{
	eraseBuffer();
	shrinkToFit();
}

bool Meta::isBufferEmpty() const noexcept
{ return m_buf.empty(); }

std::size_t Meta::size() const noexcept
{ return m_buf.size(); }

std::byte* Meta::data() noexcept
{ return m_buf.data(); }

const std::byte* Meta::data() const noexcept
{ return m_buf.data(); }

Meta::ResourceFlags& Meta::flags() noexcept
{ return m_flags; }

const Meta::ResourceFlags& Meta::flags() const noexcept
{ return m_flags; }

