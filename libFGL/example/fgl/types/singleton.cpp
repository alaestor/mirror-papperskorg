/**
This file is an example for <fgl/types/singleton.hpp>

--- Example output: none; the program compiles and exits successfully
*/
#include <fgl/types/cbyte_ptr_wrapper.hpp>

#include <cassert>
#include <fgl/types/singleton.hpp>

class obj_impl
{
	FGL_SINGLETON_BOILERPLATE(obj_impl); // optional
	public:
	int data{ 0 };
};

using obj = fgl::singleton<obj_impl>;

void f()
{
	obj::instance().data = 1337;
}

int main()
{
	f();
	assert(obj::instance().data == 1337);

	fgl::multiton<int>::instance<1>() = 1;
	fgl::multiton<int>::instance<2>() = 2;
	fgl::multiton<int>::instance<3>() = 3;
	assert(
		fgl::multiton<int>::instance<1>()
		+ fgl::multiton<int>::instance<2>()
		+ fgl::multiton<int>::instance<3>()
		== 6
	);
}
