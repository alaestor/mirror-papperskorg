#ifndef XFILE_MANIFEST_META_H_INCLUDED
#define XFILE_MANIFEST_META_H_INCLUDED

#include <vector>
#include <memory> // shared_ptr
#include <cstddef> // std::byte

namespace xfile {


namespace internal {

class Meta
{
public:

	struct ResourceFlags
	{
		bool preloadResource:1;
		bool resizeToPage:1;
		bool toUpdate:1;
		bool toDelete:1;
		bool isNew:1;
		private:
		bool reserved:3;
	};

	static_assert(sizeof(Meta::ResourceFlags) == sizeof(char), // sanity check
		"XFILE::META sizeof ResourceFlags != char");

private:

	ResourceFlags m_flags;
	std::vector< std::byte > m_buf;
	void resizeExact(const std::size_t size);
	void resizeToPage(const std::size_t size);

public:

	void resize(std::size_t size);
	void shrinkToFit() noexcept;
	void eraseBuffer() noexcept;
	void freeBuffer() noexcept;
	[[nodiscard]] bool isBufferEmpty() const noexcept;
	[[nodiscard]] std::size_t size() const noexcept;
	[[nodiscard]] std::byte* data() noexcept;
	[[nodiscard]] const std::byte* data() const noexcept;
	[[nodiscard]] ResourceFlags& flags() noexcept;
	[[nodiscard]] const ResourceFlags& flags() const noexcept;
};

} // namespace internal
} // namespace xfile

#endif // XFILE_MANIFEST_META_H_INCLUDED

