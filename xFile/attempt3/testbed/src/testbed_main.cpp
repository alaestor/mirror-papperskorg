#include <iostream>
#include <string>

#include "../../hdr/manifest.hpp"
#include "../../hdr/file_utilities.hpp"
#include "../../hdr/xwriter.hpp"

#include <cstdlib>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	try
	{
		using std::cout, std::endl;
		using namespace xfile::internal;
		
		const auto execPath{ file_utilities::getPathToExecutingBinary() };
		cout
			<< "\n\n" << "Hello, World! " << "I'm main!\n"
			<< "execPath = " << execPath << endl;
		
		if (Manifest::fileContainsManifest(execPath))
		{
			Manifest m(execPath);
			cout << "I contain " << m.sectionCount() << " sections!" << endl;
			m.TEST_DEBUG_DELETEME_COUT_SECTIONS();
		}
		else
		{
			cout << "I'm brand new!" << endl;
			Manifest m;
			m.TEST_DEBUG_DELETEME_ADD_SECTOR("Test1");
			m.TEST_DEBUG_DELETEME_ADD_SECTOR("Test2");
			
			const auto filename{ xwriter::filename };
			file_utilities::createFileWithData(
				filename, xwriter::data, xwriter::length);
			
			m.serializeToFile(filename);
			
			auto l = std::string(".\\")
				+ std::string(xfile::internal::xwriter::filename)
				+ " \"" + execPath.string() + "\"";
			cout << "[[executing]] "<< l << "\n\n" << endl;
			system( l.c_str() );
		}
	}
	catch (const std::exception& e)
	{
	   std::cerr << "\n\n\n[[EXCEPTION]] CAUGHT: " << e.what() << std::endl;
	}
	return 0;
}

