#ifndef XFILE_MANIFEST_H_INCLUDED
#define XFILE_MANIFEST_H_INCLUDED

#include <vector>
#include <unordered_map>
#include <utility> // std::pair (redundent)
#include <string>
#include <cstddef> // std::byte
#include "../hdr/Manifest_Meta.h"
#include "../hdr/Manifest_Handle.h"

namespace xfile {

/*

enum AccessPermissions : char
{
	Existing = 1,
	New = 2
};

*/

namespace internal {

class Manifest
{ // the manifest containing the resource list and internal file buffers
	protected:

	explicit Manifest(bool forceDisablePreloads);

	typedef std::unordered_map< std::string, Meta > MetaList;
	typedef MetaList::iterator MetaList_iter;
	typedef MetaList::const_iterator MetaList_const_iter;
	[[nodiscard]] bool validIterator(MetaList_const_iter iter) const;

	MetaList m_metas;

	public:

	[[nodiscard]] decltype(m_metas)& metas();
	[[nodiscard]] const decltype(m_metas)& metas() const;

	// Manifest General Methods
	[[nodiscard]] bool isEmpty() const;
	[[nodiscard]] bool exists(const std::string& filename) const;
	[[nodiscard]] Handle open(const std::string& filename);
	[[nodiscard]] Handle create(const std::string& filename);
	void remove(const std::string& filename, bool force = false);
	//void flagToRemove(const std::string& filename);

	// Packing
	static constexpr const char* m_PackedResID{ "__xfile_PackedManifest" };
	typedef std::vector< std::byte > Packed;
	typedef std::vector<
		std::pair< const Meta::ResourceFlags*, const std::string >
	> Unpacked;

	[[nodiscard]] Packed pack() const;
	[[nodiscard]] Unpacked unpack(const std::byte* packed) const;


	// Manifest Object
	static Manifest& getInstance(bool forceDisablePreloads = false)
	{
		// Guaranteed to be lazy initialized
		// Guaranteed that it will be destroyed correctly
		static Manifest instance(forceDisablePreloads);
		return instance;
	}
	Manifest(Manifest const&) = delete;
	void operator=(Manifest const&) = delete;

	~Manifest(){};
};

} // namespace internal

// should use unordered_map's [] and bruteforce both new and existing?



} // namespace xfile

#endif // XFILE_MANIFEST_H_INCLUDED
