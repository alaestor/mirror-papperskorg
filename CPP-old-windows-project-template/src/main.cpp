#include <iostream>

/**
@page page-older_versions Documentation for Older Versions
@parblock
	The documentation found here is only accurate for the latest version.
	Documentation for older versions must be generated from source
	by running <a href="https://www.doxygen.nl/">Doxygen</a> against the
	@c Doxyfile in the root project directory. After that, the
	documentation may be viewed by opening the @c docs/index.html file in
	a web browser.
@endparblock
*/

/**
@page page-about About Stuff And Things
@section page-things The Things In Question
	@parblock
		Here I list a **thing**:
		- @subpage page-about-cont
	@endparblock
<hr  />
@section page-about-cont P.s.
	Some text
	@note
		@parblock
			<b>Q</b>: Is this an [example](https://en.wiktionary.org/wiki/example)??

			<b>A</b>: Yes; indeed it is.
		@endparblock
		@see @ref page-older_versions for documentation for older versions (in case you couldn't guess)
	@remarks
		@code
		int main();
		@endcode
		Is located in src/main.cpp
	@warning
		@parblock
			The cake is a lie!!

			This is a ~~triumph~~ warning.
		@endparblock
<hr  />
*/

/**
@internal
@page page-dev Development Documentation
	@parblock
		This page contains some information for internal development.
		You should make doxyfile's `INTERNAL_DOCS` is enabled.
		You can selectively conceal things by utilizing
		<a href="https://www.doxygen.nl/manual/config.html#cfg_enabled_sections">
		ENABLED_SECTIONS</a>
	@endparblock
<hr  />
*/

/**
@file
@version 0.1.0-alpha
@brief The Main.cpp source file
@details It contains some *main* stuff
*/

/**
@brief Entrypoint to the Hello World program
@details Salutations, Globe!
@todo Make the code worky
*/
int main()
{
	std::cout << "Hello, World!\n";
}
