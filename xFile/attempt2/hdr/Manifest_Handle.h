#ifndef XFILE_MANIFEST_HANDLE_H_INCLUDED
#define XFILE_MANIFEST_HANDLE_H_INCLUDED

#include <vector>
#include <cstddef> // std::byte
#include "../hdr/Manifest_Meta.h"

namespace xfile {

class Handle
{
	using Meta = xfile::internal::Meta;
	Meta* m_meta_p{ nullptr };

	protected:

	[[nodiscard]] Meta::ResourceFlags& getFlags() noexcept;
	[[nodiscard]] const Meta::ResourceFlags& getFlags() const noexcept;
	void invalidate();

	public:

	// meta pointer methods
	bool isInvalid() const noexcept { return m_meta_p == nullptr; }
	bool isValid() const noexcept { return !isInvalid(); }

	// Buffer Specific Methods

	void resize(const std::size_t size);
	void shrinkToFit();
	void eraseBuffer();
	void freeBuffer();
	[[nodiscard]] bool isBufferEmpty() const;
	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] std::byte* buffer();
	[[nodiscard]] const std::byte* buffer() const;


	// Flags
	[[nodiscard]] bool preloadResource() const;
	void preloadResource(const bool state);
	[[nodiscard]] bool resizeToPage() const;
	void resizeToPage(const bool state);
	[[nodiscard]] bool toUpdate() const;
	void toUpdate(const bool state);
	[[nodiscard]] bool toDelete() const;
	void toDelete(const bool state);
	[[nodiscard]] bool isNew() const;
	void isNew(const bool state);

	Meta* operator=(Meta* p) noexcept { return m_meta_p = p; }
	Meta* operator=(Meta& m) noexcept { return m_meta_p = &m; }

	explicit Handle() : m_meta_p(nullptr) {};
	explicit Handle(Meta* p) : m_meta_p(p) {};
	explicit Handle(Meta& m) : m_meta_p(&m) {};
	~Handle() = default;
};

} // namespace xfile

#endif // XFILE_MANIFEST_HANDLE_H_INCLUDED
