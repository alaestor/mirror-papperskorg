#pragma once
#ifndef SINGLE_CLASS_HPP_INCLUDED
#define SINGLE_CLASS_HPP_INCLUDED

#include <fgl/types/singleton.hpp>

namespace internal {
class obj_singleton
{
	FGL_SINGLETON_BOILERPLATE(obj_singleton);
	public:
	int data{ 1337 };
};
}// namespace internal

using Obj = fgl::singleton<internal::obj_singleton>;

#endif // SINGLE_CLASS_HPP_INCLUDED

