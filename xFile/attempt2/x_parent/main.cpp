#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

#define WINVER _WIN32_WINNT_WIN7
#include <windows.h>


#include "../hdr//ResourceManager.h"
#include "../hdr/ChildManager.h"
#include "../hdr/Manifest.h"

using namespace std;
using namespace xfile;
using namespace xfile::internal;

//ResourceManager resman;
//auto& manifest = Manifest::getInstance();


static constexpr const bool verbose = true;

static const char resNameArray[]{ "MyInt" };
static constexpr const char* t1{ &resNameArray[0] };


void xit()
{
	// TODO move to destructor of xfile
	ChildManager childmanager;
	childmanager.makeChildExecutable();
	childmanager.startChildExecutable();
}

std::string tf(bool b)
{ return b ? "true" : "false"; }

template < typename T >
std::pair< Handle, T* > obtainPersistent(const std::string& filename)
{
	auto& manifest{ Manifest::getInstance() };

	bool exists{ false };
	Handle h{ (exists = manifest.exists(filename)) ?
		manifest.open(filename) : manifest.create(filename) };

	if (h.isInvalid()) throw std::runtime_error("\tinvalid xfile Handle");

	if constexpr (verbose) cout
		<< "\t"
		<< '\"' << filename << '\"'
		<< (exists ? " Opened" : " Created")
		<< ':';

	if (h.size() == 0)
	{
		size_t s = sizeof(T);
		if constexpr (verbose) cout << " resize=(" << s << ')';
		h.resize(s);
	} else if constexpr (verbose) cout << " size=(" << h.size() << ')';

	T v = *reinterpret_cast< T* >(h.buffer());
	if constexpr (verbose) cout << " value=(" << v << ')' << endl;

	return std::pair(h, reinterpret_cast< T* >(h.buffer()));
}


template <typename T> // must have > and ++ operators
void incrementingTest(const std::string& filename, const T limit)
{
	cout << "\n";
	auto [h, p]{ obtainPersistent< T >(filename) };
	auto& v = *p;

	if (v > limit)
	{
		cout << "\tValue over " << limit << ". Flagging for deletion." << endl;
		h.toDelete(true);
	} else cout << "\tIncrementing value to " << ++v << endl;
}

//
void strTest(const std::string& filename)
{
	auto& manifest{ Manifest::getInstance() };
	bool exists{ false };
	Handle h{ (exists = manifest.exists(filename)) ?
		manifest.open(filename) : manifest.create(filename) };

	if (h.isInvalid()) throw std::runtime_error("\tinvalid xfile Handle");

	static constexpr const char* hello{ "Hello, World!" };
	static constexpr const char* world{ "Fuck The World..." };

	if (h.size() == 0)
	{ h.resize(255); }

	char* v = reinterpret_cast< char* >(h.buffer());

	if (!exists) memcpy(v, hello, strlen(hello)+1);

	if (verbose) cout << "\n\tTest Sring: \"" << v << '"' << endl;

	if (strcmp(v, hello) == 0) memcpy(v, world, strlen(world)+1);
	else memcpy(v, hello, strlen(hello)+1);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	incrementingTest<int>("MyInt", 1);
	incrementingTest<double>("MyDouble", 2.0);
	strTest("strTest");
	xit();

	return 0;
}
