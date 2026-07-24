/*
T H E         ,.__                            XXXXXxX  xxxXXXXXX
             ((-\)),                             xXX      xXX
             | . . {         ___.                  X      XX
T R U T H    (  _) )       /(()-/)                  X   xX
              \ _ /       / /. . |\                 xx xX
            __-\_/-__     ||  ]  )|        T  H  E   (X)   F  I  L  E
I S        / \ \@//  \    /|  -  ||                 xX xx
          |  | |{||__|\   /_\___/|,                Xx   Xx
          |  | |}||  ||  |    \/   \              XX     XX   (TM)
O U T     |  {.|{/|  ||  | |_| __| |            xXX       XX
          |  /.|__|  ||  \___\_____/         xXXXXXXx   XXXXxXx
           \/ .|=>|  |/   /   o|  \
T H E R E .exe
*/

#ifndef XFILE_H_INCLUDED
#define XFILE_H_INCLUDED

#include <string>
#include <limits>

namespace xfile {

template< typename T >
class Persistent : public ISerializable
{
	T var;

	public:

	//operator T() noexcept { return var; }

	[[nodiscard]] T& get(){ return &var; }

	[[nodiscard]] Serialized serialize()
	{
		Serialized s(sizeof(T), static_cast< const std::byte* >(new T(var)));
		return s;
	}

	void deserialize(Serialized s)
	{
		var = *s.buffer;
	}

	explicit Persistent(const std::string& filename)
	{
		???(filename, this);
	}

	explicit Persistent(const std::string& filename, T& initialize)
	: var(initialize)
	{
		???(filename, this);
	}

	~Persistent(){}
};

int main()
{
	Persistent<int>("EXP", 100) exp;
}






















/*

typedef const std::size_t Handle;
constexpr const Handle invalid_handle = std::numeric_limits<Handle>::max();

// xfile system
void start(const bool preloadFiles);
void start();
void stop();

// file creation and deletion
Handle open(const std::string& filename);
Handle create(const std::string& filename);
void deleteFile(Handle handle);
void deleteFile(const std::string& filename);

// file i/o
std::size_t read(void* bufferOut, const std::size_t size, Handle handle);
void write(Handle handle, const void* bufferIn, const std::size_t size);
void writeNoResize(
	Handle handle,
	const void* bufferIn,
	const std::size_t size);

// file sizing
std::size_t getFileSize(Handle handle);
void resizeFile(Handle handle, const std::size_t size);
void resizeFileExactly(Handle handle, const std::size_t size);
void shrinkFileToFit(Handle handle);
void shrinkAllFilesToFit(Handle handle);

*/

} // namespace xfile

#endif // XFILE_H_INCLUDED
