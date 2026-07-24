#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>

#include "../../hdr/manifest.hpp"
#include "../../hdr/file_utilities.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	using std::cout, std::endl;
	cout
		<< "\n\n" << "Hello, World! " << "I'm xwriter!\n"
		<< "my parameters are:" << endl;
	
	for (auto n{ 0 }; n < argc; ++n)
		cout << "\t" << n << " : " << argv[n] << endl;
	
	if (argc < 2) throw std::runtime_error("expected filePath in argv!");
	std::filesystem::path parent{ argv[1] };
	
	// will be handled by xfile in the future
	const auto myPath{
		xfile::internal::file_utilities::getPathToExecutingBinary()
	};
	xfile::internal::Manifest me(myPath);
	cout << "I contain " << me.sectionCount() << " sections!" << endl;
	me.TEST_DEBUG_DELETEME_COUT_SECTIONS();
	
	
	// dirty test
	auto newparent{ parent.string()+".new.exe" };
	std::filesystem::copy(parent, newparent);
	
	
	me.serializeToFile(newparent);
	
	
	auto l = ".\\" + parent.filename().string()+".new.exe";
	cout << "[[executing]] "<< l << "\n\n" << endl;
	system( l.c_str() );
	
	return 0;
}