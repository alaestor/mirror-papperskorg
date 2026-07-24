#include <cstddef> // std::byte
#include "../hdr/Manifest_Meta.h"
#include "../hdr/Manifest_Handle.h"

#define WINVER _WIN32_WINNT_WIN7
#include <windows.h> // system info

using Handle = xfile::Handle;
using Meta = xfile::internal::Meta;

/*
	templates make unique types?
	could i template <metaptr> and make ptr static?
	so all that point to same address are metas
*/

/***********
 * Utility *
 ***********/

// for resizeToPage()




Meta::ResourceFlags& Handle::getFlags() noexcept
{ return m_meta_p->flags(); }

const Meta::ResourceFlags& Handle::getFlags() const noexcept
{ return m_meta_p->flags(); }

void Handle::invalidate()
{ m_meta_p = nullptr; }

/***************************
 * Buffer Specific Methods *
 ***************************/

void Handle::resize(const std::size_t size)
{ m_meta_p->resize(size); }

void Handle::shrinkToFit()
{ m_meta_p->shrinkToFit(); }

void Handle::eraseBuffer()
{ m_meta_p->eraseBuffer(); }

void Handle::freeBuffer()
{ m_meta_p->freeBuffer(); }

bool Handle::isBufferEmpty() const
{ return m_meta_p->isBufferEmpty(); }

std::size_t Handle::size() const
{ return m_meta_p->size(); }

std::byte* Handle::buffer()
{ return m_meta_p->data(); }

const std::byte* Handle::buffer() const
{ return m_meta_p->data(); }


/***************************
 * Flag Specific Methods *
 ***************************/

bool Handle::preloadResource() const
{ return getFlags().preloadResource; }
void Handle::preloadResource(const bool state)
{ getFlags().preloadResource = state; }

bool Handle::resizeToPage() const
{ return getFlags().resizeToPage; }
void Handle::resizeToPage(const bool state)
{ getFlags().resizeToPage = state; }

bool Handle::toUpdate() const
{ return getFlags().toUpdate; }
void Handle::toUpdate(const bool state)
{ getFlags().toUpdate = state; }

bool Handle::toDelete() const
{ return getFlags().toDelete; }
void Handle::toDelete(const bool state)
{ getFlags().toDelete = state; }

bool Handle::isNew() const
{ return getFlags().isNew; }
void Handle::isNew(const bool state)
{ getFlags().isNew = state; }


