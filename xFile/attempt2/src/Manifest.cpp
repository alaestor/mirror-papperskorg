
//#include <vector>
//#include <unordered_map>
#include <string>
#include <memory> // std::addressof()
#include <cstddef> // std::byte
#include <cstring> // std::strncpy
#include "../hdr/Manifest.h"
#include "../hdr/Manifest_Meta.h"
#include "../hdr/Manifest_Handle.h"
#include "../hdr/ResourceManager.h"


#define WINVER _WIN32_WINNT_WIN7
#include <windows.h> // system info

using Manifest = xfile::internal::Manifest;
using Handle = xfile::Handle;
using Meta = xfile::internal::Meta;
using ResourceManager = xfile::internal::ResourceManager;


/***************
 * Constructor *
 ***************/

Manifest::Manifest(bool forceDisablePreloads)
{
	const ResourceManager resman;
	if (resman.exists(m_PackedResID))
	{
		std::size_t packedSize{ 0 };
		auto packed{ resman.expose(m_PackedResID, packedSize) };
		Unpacked unpacked{ unpack(packed) };

		// generate runtime manifest, load flags to metas
		for (const auto& [flag_p, filename] : unpacked)
		{
			auto& m{ m_metas[filename] };
			m.flags() = *flag_p; // flags assignment MUST be before resize!

			if (!forceDisablePreloads)
			{ // read resource to buffer
				std::size_t resSize{ resman.size(filename) };
				if (resSize == 0) continue;
				m.resize(resSize);
				resman.read(filename, m.data());
			}
		}
	}
}


/***********
 * Utility *
 ***********/


bool Manifest::validIterator(MetaList_const_iter iter) const
{ return iter != m_metas.end(); }


/****************************
 * Manifest General Methods *
 ****************************/

decltype(Manifest::m_metas)& Manifest::metas()
{ return m_metas; }

const decltype(Manifest::m_metas)& Manifest::metas() const
{ return m_metas; }

bool Manifest::isEmpty() const
{ return m_metas.empty(); }

bool Manifest::exists(const std::string& filename) const
{ return (m_metas.find(filename) != m_metas.end()) ? true : false; }

/*
	TODO implement a meta which accepts a shared pointer to memory, so the user
	can allocate and manage their own.

	TODO open(const std::string& filename, AccessPermissions = NEW | EXISTING)
	access perms:
		NEW will only create and open if it doesnt otherwise exist; else error
		EXISTING will only open existing file, else error
		NEW | EXISTING will ... obvious. No errors.

	read-only flag?
*/
Handle Manifest::open(const std::string& filename)
{
	const auto& iter{ m_metas.find(filename) };
	if (!validIterator(iter)) return Handle(nullptr);

	Handle h(std::addressof(iter->second));

	// h.preloadResource() &&
	if ((!h.isNew()) && (!h.isBufferEmpty()))
	{
		const ResourceManager resman;
		h.resize(resman.size(filename));
		resman.read(filename, h.buffer());
	}

	h.toUpdate(true);
	h.toDelete(false);

	return h;
}

Handle Manifest::create(const std::string& filename)
{
	if (exists(filename)) return Handle(nullptr);

	Handle h(std::addressof(m_metas[filename]));
	h.isNew(true);
	h.toUpdate(true);
	h.toDelete(false);

	return h;
}

void Manifest::remove(const std::string& filename, bool force /*= false*/)
{
	if (auto iter{ m_metas.find(filename) }; validIterator(iter))
	{
		Handle h(std::addressof(iter->second));
		h.freeBuffer();
		if (h.isNew() || force) m_metas.erase(iter);
		else h.toDelete(true); // flag for child proc to force
	}
}

/*
void manifest::flagToRemove(const Handle& handle)
{
	toDelete(handle, true);
	freeBuffer(handle);
}

void manifest::flagToRemove(const std::string& filename)
{ if (auto h = open(filename); h.isValid()) flagToRemove(h); }
*/

/***********
 * Packing *
 ***********/

using ResourceFlags = xfile::internal::Meta::ResourceFlags;

Manifest::Packed Manifest::pack() const
{
	Packed p;
	if (isEmpty()) return p; // nothing else to do

	//sum amt + flags + filename for buffer size. filenames will be nullterm'd
	const std::size_t elements{ m_metas.size() };
	std::size_t packedSize{ sizeof(elements) };
	packedSize += sizeof(ResourceFlags) * elements;
	for (const auto& pair : m_metas) packedSize += pair.first.length() + 1;

	p.resize(packedSize);
	auto write_cursor{ reinterpret_cast< char* >(p.data()) };
	memcpy(write_cursor, &elements, sizeof(elements));
	write_cursor += sizeof(elements);

	for (const auto& [filename, meta] : m_metas) // note: "structured binding"
	{ // Pack flags and null-terminated filenames
		constexpr const std::size_t sf{ sizeof(Meta::ResourceFlags) };
		std::memcpy(write_cursor, std::addressof(meta.flags()), sf);
		write_cursor += sf;
		std::strncpy(write_cursor, filename.c_str(), filename.length() + 1);
		write_cursor += filename.length() + 1;
	}

	return p;
}

Manifest::Unpacked Manifest::unpack(const std::byte* const packed) const
{
	auto count{ *reinterpret_cast< const std::size_t* >(packed) };

	Unpacked p;
	auto read_cursor{ packed + sizeof(count) };

	for (std::size_t elements = 0; elements < count; ++elements)
	{
		auto flags{ reinterpret_cast< const ResourceFlags* >(read_cursor) };
		read_cursor += sizeof(decltype(*flags));
		auto name{ reinterpret_cast< const char* >(read_cursor) };
		p.emplace_back(flags, name);
		read_cursor += p.back().second.length() + 1; // after null terminator
	}

	return p;
}
