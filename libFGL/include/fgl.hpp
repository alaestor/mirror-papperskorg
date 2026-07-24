#pragma once
#ifndef FGL_HPP_INCLUDED
#define FGL_HPP_INCLUDED

// should contain all top-level <fgl/*.hpp>

#include "./fgl/debug.hpp"
#include "./fgl/io.hpp"
#include "./fgl/types.hpp"
#include "./fgl/utility.hpp"

/**
@file
@brief libFGL's "(almost) everything" header
@details Includes all non-experimental headers. The file itself also
	contains general doxymentation for the library.
@note Does not expose literals or opt-in macros.
@see <tt>#include <@ref FGLM></tt>
*/

/**
@page page-fgl-older_versions Documentation for Older Versions
@parblock
	The documentation found here is only accurate for the latest version
	of libFGL, which is always the most recent state of the main github
	branch. Documentation for older versions must be generated from source
	by running <a href="https://www.doxygen.nl/">Doxygen</a> against the
	@c Doxyfile in the root project directory. After that, the
	documentation may be viewed by opening the @c docs/index.html file in
	a web browser.
@endparblock
*/

/**
@page page-fgl-getting_started Getting Started
@section page-fgl-about-compatibility Official Compatibility
	Refer to @ref group-environment-libfgl_compatibility_check documentation.
<hr  />
@section page-fgl-getting_started-installation Installing and using libFGL
@subsection page-fgl-getting_started-installation-conan With Conan
	<ol type="1">
	<li>
		If you don't have it already, install
		<a href="https://conan.io/">the Conan package manager</a>
	</li>
	<li>
		Enable <a href="https://docs.conan.io/en/latest/versioning/revisions.html#how-to-activate-the-revisions">Package Revisions</a>
		by adding <tt>revisions_enabled=1</tt> under <tt>[general]</tt> in
		your @c conan.conf file (<tt>~/.conan/conan.conf</tt> on Linux, and
		<tt>\%userprofile\%/.conan/conan.conf</tt> on Windows).
	</li>
	<li>
		Add the libFGL remote by running
		<tt>conan remote add fgl-conan https://fgl.jfrog.io/artifactory/api/conan/fgl-conan</tt>
	</li>
	<li>
		Add libFGL as a <a href="https://docs.conan.io/en/latest/using_packages/conanfile_txt.html">conan dependency</a>

		It's recommended to always use the latest version of libFGL by
		requiring the @c latest alias on the @c main channel
		@code
		[requires]
		libFGL/latest@alaestor/main
		@endcode

		Specific versions can be required by following the format
		<tt>branch-commit</tt>, and the @c alaestor/CHANNEL channel can be
		the name of the branch. For example:
		@code
		[requires]
		libFGL/main-0123456789abcdef@alaestor/main
		@endcode

		@note @parblock
			Some commits may not be available, including older commits before
			conan was supported and commits on branches other than
			<tt>main</tt>. Only the @c main branch is automatically packaged
			when a new commit is pushed, but other branches may occasionally
			be manually packaged.

			You can list all available versions by running
			<tt>conan search "libFGL/*" \--remote=fgl-conan</tt> (it may take a while).

		@attention @parblock
			A "live at head" approach is strongly recommended: don't pin to a
			specific commit without good reason. <b>Breaking changes are a
			feature, and interfaces will often change for good reason!</b>

			Only use packages from the @c main branch which are packaged on
			the <tt>@alaestor/main</tt> and <tt>@alaestor/stable</tt> channels.
			Using other branches and channels are <b>strongly discouraged</b>.

			Packages on the <tt>@alaestor/volatile</tt> channel are subject to
			random revisions and deletions without warning and should be
			viewed as <b>temporary</b>! They should avoided unless explicitly
			directed by a maintainer (for example, when being used temporarily
			as a trouble-shooting measure).
		@endparblock
		@endparblock
	</li>
	</ol>

@subsection page-fgl-getting_started-installation-standalone Without Conan
	<ol type="1">
	<li>
		Download the libFGL source to where ever you want to install it,
		either by running <tt>git clone https://github.com/alaestor/libFGL.git</tt>
		or by <a href="https://github.com/alaestor/libFGL/archive/refs/heads/main.zip">downloading it as a ZIP archive from Github</a>
	</li>
	<li>
		Add the library include path when you compile.
		<ul>
		<li>
			For <a href="https://clang.llvm.org/docs/ClangCommandLineReference.html#include-path-management">Clang</a>
			and <a href="(https://gcc.gnu.org/onlinedocs/gcc/Directory-Options.html">GCC</a>
			pass <tt>-I"your_path_to/libFGL/include"</tt> when compiling.
		</li>
		<li>
			For <a href="https://docs.microsoft.com/en-us/cpp/preprocessor/hash-include-directive-c-cpp?view=msvc-170">MSVC</a>
			pass <tt>/I"your_path_to/libFGL/include"</tt> when
			compiling.
		</li>
		</ul>
		Replace <i><tt>your_path_to</tt></i> with the path to where you
		installed libFGL.
	</li>
	</ol>

@subsection page-fgl-getting_started-installation-example_program Hello World
	@parblock
	@code
	// main.cpp
	#include <array>

	#include <FGLM> // all of libFGL + opt-in features

	//#include <fgl.hpp> // all of libFGL without opt-in features

	/// or you can be more selective:
	// #define FGL_SHORT_MACROS
	// #include <fgl/debug/constexpr_assert.hpp> // constexpr_assert
	// #include <fgl/debug/output.hpp> // debug::output
	// #include <fgl/types/range_constraints.hpp> // sized_range_same_as
	// #include <fgl/utility/zip.hpp> // czip

	/// silly example, implemented using fgl::zip & range constraints
	constexpr int sum_int_pairs(
		const fgl::sized_range_same_as<int> auto& a,
		const fgl::sized_range_same_as<int> auto& b)
	{
		int sum{ 0 };
		for (const auto& [av, bv] : fgl::czip(a, b))
			sum += av + bv;
		return sum;
	}

	int main()
	{
		[]() consteval
		{
			constexpr std::array a{ 1, 1, 1 }; // 3 elements
			std::array b{ 1, 1, 1, 1 }; // 4 elements, last will be ignored
			constexpr_assert(sum_int_pairs(a, b) == 6);
		}();

		fgl::debug::output("Hello, world!");
	}
	@endcode

	Example compiler arguments:

	@code
	g++ -std=c++20 -I"your_path_to/libFGL/include" main.cpp -o main.exe
	@endcode

	Output:
	@code
	[GENERIC] Hello, world!
	@endcode

	libFGL extensions will often require more elaborate dependencies and
	compilation steps. Refer to the extension's documentation.

	@note @parblock
		Depending on how your compiler arguments and environment
		configuration, you may need to include libFGL components with either
		<tt>""</tt> or <tt><></tt> include syntax. Including as a system
		header is recommended to avoid superfluous warning messages from
		compilers which aren't officially compatible.

		Other than for testing, libFGL doesn't have official guides or
		compatibility with any particular build systems or IDEs. You're on
		your own.
		@endparblock
	@endparblock
<hr  />
@section page-fgl-getting_started-running_tests Compiling and Running Tests
	@parblock
		libFGL is header-only, so it doesn't require compilation or linking.
		Just include the components you want and you're done. However, you
		should probably compile and run the unit tests to make sure
		everything's behaving properly in your environment.

		Remove the <tt>.DEFAULT</tt> extension from <tt>tup.config.DEFAULT</tt>.
		This file contains system-specific paths and user preferences.

		Use <a href="https://gittup.org/gittup/">the <i>tup</i> build-system</a>
		by executing <tt>tup</tt> in the directory that contains
		<tt>Tupfile.ini</tt>. The top-level <tt>Tupfile</tt> is configured to
		build and run all tests. If <tt>tup</tt> completes without warnings or
		errors, then the library should be fully-functional on the platform
		in question.

		Example of success / passing
@code{.unparsed}
[ tup ] [0.000s] Scanning filesystem...
[ tup ] [0.055s] Reading in new environment variables...
[ tup ] [0.056s] No Tupfiles to parse.
[ tup ] [0.056s] No files to delete.
[ tup ] [0.057s] Executing Commands...
 67) [0.073s] test\fgl_environment_build_info: [C] notmain.cpp

                      ...

  1) [0.089s] test\fgl_utility_matrix: [L] main.o
  0) [0.021s] test\fgl_utility_matrix: [TEST] fgl_utility_matrix.exe
 [                   ETA=<1s Remaining=0  Active=0                    ] 100%
[ tup ] [11.435s] Updated.
	@endcode

	Example of failure (where @c fgl_environment_build_info fails)
	@code{.unparsed}
[ tup ] [0.000s] Scanning filesystem...
[ tup ] [0.027s] Reading in new environment variables...
[ tup ] [0.028s] No Tupfiles to parse.
[ tup ] [0.028s] No files to delete.
[ tup ] [0.028s] Executing Commands...
* 5) test\fgl_environment_build_info: [C] main.cpp
src/main.cpp: In function 'int main()':
src/main.cpp:13:23: error: static assertion failed
   13 |         static_assert(fgl::debug_build);
      |                       ^~~~~~~~~~~~~~~~~~~~
 *** tup messages ***
 *** Command ID=3396 failed with return value 1
 [      ]  85%
 *** tup: 1 job failed.
	@endcode
	@endparblock
<hr  />
@section page-fgl-getting_started-extentions Extentions for LibFGL
	@parblock
		<ul>
		<li>
			(WIP) <b>libFGL-vulkan</b> &mdash; a simple and modern
				interface for using Vulkan compute shaders.

			Work In Progress ->
			<a href="https://github.com/KJNeko/LearningVulkan">
			KJNeko's LearningVulkan repository</a>
		</li>
		</ul>
	@endparblock
*/

/**
@page page-fgl-about About libFGL fascilities
@section page-fgl-header About libFGL Headers
	@parblock
		The <tt>fgl.hpp</tt> header includes the entire libFGL library,
		excluding experimental headers and libFGL extensions.

		<tt>#include <@ref fgl.hpp></tt> provides the following fascilities:
		- @subpage page-fgl-header-debug
		- @subpage page-fgl-header-environment
		- @subpage page-fgl-header-io
		- @subpage page-fgl-header-types
		- @subpage page-fgl-header-utility

		<b>Include Folder Structure</b>

		The <tt>@ref fgl.hpp</tt> header includes all top-level
		<tt>*.hpp</tt> in @c fgl/ , which themselves include headers from
		their namesake subdirectories; this recursive inclusion continues
		until all library fascilities are included, except for experimental
		and opt-in features.

		The <tt>@ref FGLM</tt> header includes <tt>@ref fgl.hpp</tt> but also
		all opt-in features like <tt>FGL_SHORT_MACROS</tt> and exposes
		<tt>using namespace fgl::literals;</tt>.

		libFGL extensions must be included separately. Thier include
		hierarchy follows the same scheme as the main library, but as
		extentions are installed separately, they aren't included by the main
		library's top level headers.
	@endparblock
<hr  />
@section page-fgl-macros About Preprocessor Macros
	@parblock
		libFGL's preprocessor macros come in two flavors: long names in a
		screaming-snake style which are prefixed with <tt>FGL_</tt> or
		<tt>FGL_DEBUG_</tt>, and short names which can vary in styling and are
		aliases to their longer counterparts.

	@note libFGL guarentees that no macros will be redefined, with the
		exception of short-macro symbols. If a preprocessor symbol is already
		defined, an error will be raised.

		Short macros are opt-in only to avoid name collisions, and will only be
		defined by a given header when that header's specific opt-in symbol is
		defined, or if the global-opt-in symbol <tt>FGL_SHORT_MACROS</tt> is
		defined.

	@note Opt-in symbol definitions must occur before including any libFGL
		headers. Some headers are used internally, so your include order may
		be misleading, causing the opt-in symbols to be defined after the
		definition headers, resulting in the short macros not being defined.

		Header specific opt-in symbols are styled as
		<tt>FGL_DEBUG_</tt><i>HEADER_NAME_WITHOUT_EXTENSION</i><tt>_MACROS</tt>
		and will be documented in the relevant modules.
		@see page-fgl-internals regarding <tt>FGL_INTERNAL_</tt> macros.

	@remarks
		Function-like preprocessor macros may have difficulty parsing
		expressions that contain comma characters (<tt>,</tt>) such as
		@code
		MACRO( f<a,b>() );
		@endcode
		in which case you may need to wrap the expression in parentheses:
		@code
		MACRO( ( f<a,b>() ) );
		@endcode
	@endparblock
<hr  />
@section page-fgl-literals About libFGL literals
	@parblock
		libFGL literals are contained within the <tt>fgl::literals</tt>
		namespace.
	@endparblock
<hr  />
@section page-fgl-internals About Internal and Experimental Features
	@parblock

		<b> Internal Features </b>

		Anything (folder, file, namespace, preprocessor definition, symbol,
		etc.) containing one or more of these words, regardless of underscores
		or other surrounding context, should be considered internal to the FGL
		library and not part of the public interface. You should never have to
		explicitly include these files or use these components directly. If
		you do end up using them, please do so with the understanding that
		they aren't intended for external use and may spontaneously explode at
		any time.

		<ul>
		<li><tt>experimental</tt></li>
		<li><tt>WIP</tt></li>
		<li><tt>internal</tt></li>
		<li><tt>details</tt></li>
		<li><tt>test</tt></li>
		</ul>

		Interal preprocessor definitions will always begin with
		<tt>FGL_INTERNAL</tt>. These are often used for code-generation or
		are the implementation details of public macros.

		<b> Experimental Features </b>

		Experimental components shouldn't be used and are things that may or
		may not become part of libFGL at some point. Some experimental headers
		are in active developement are used for rapid evolution and discovery;
		while others are old copy+paste code snippets from other projects
		(including personal projects) which may be used as idea-fodder or are
		to be rewritten as libFGL components.

		Generally speaking, experimental components should be viewed as
		broken and/or incomplete junk.
	@endparblock
*/

/**
@internal
@page page-fgl-dev Development Documentation
	@parblock
		This page contains some "derp" information for internal library
		development.

		You should also make sure Doxygen is configured for internal
		documentation by adding <tt>FGL_INTERNAL_DOCS</tt> to the
		<a href="https://www.doxygen.nl/manual/config.html#cfg_enabled_sections">
		ENABLED_SECTIONS</a> in the <tt>Doxyfile</tt> file.
	@endparblock
<hr  />
@section page-fgl-dev-writing_tests Writing Tests
	@parblock
		Each component of the library, where applicable, will have a unit test
		in the <tt>test/</tt> directory with a folder name that matches the
		include structure with separators replaced with underscores. For
		example, <tt>include/fgl/debug/fixme.hpp</tt> corresponds with
		<tt>test/fgl_debug_fixme</tt>.

		Tests should have minimal includes and namespace polution. Test
		programs return <tt>EXIT_SUCCESS</tt> to signal that the test(s) have
		passed.

		Tests should strive to use compile-time <tt>static_assert</tt>
		and <tt>constexpr</tt> functions where ever possible. Avoid
		<tt>consteval</tt> with few exceptions, as <tt>constexpr</tt> is more
		flexible because sometimes running the tests at runtime can provide
		useful information. Where <tt>static_assert</tt> is not possible,
		use <tt>@ref constexpr_assert</tt> unless the test cannot be
		compile-time in which case <tt>assert</tt> is fine. Where assertions
		aren't possible, a test failure should cause the program to return
		code <tt>EXIT_FAILURE</tt>. Tests should not use standard input/out
		(i.e. <tt>cout</tt>, <tt>cin</tt>).

		Each tester (i.e. assertion) should be granular and represent a
		minimal subset of component behavior in order to improve debugability.

		Don't accumulate failures where possible; prefer to fail hard and
		fast. Use descriptive variable and function names. The intent and
		conditions of a test should be self-evident in code or at the very
		least clearly documented by comments.
	@endparblock
<hr  />
@section page-fgl-dev-test_build Test Build System & Dependencies
	@parblock
		Each test (<tt>test/fgl_component/</tt>) will contain a <tt>Tupfile</tt>
		which defines rules for compilation, linking, and running of the test.

		Optionally, a test can have a <tt>Tuprules.tup</tt> file which can be
		used to determine <i>test ordering</i> (prerequisites / dependencies)
		based on folder name.

		@warning @parblock
			An important limitation of "dependencies" is that the required test
			names must exist and be correct. If a required test doesn't exist,
			the requirement is silently ignored. That would be bad.

			<blockquote>
			They're more like <i>guidelines</i> than actual <b>rules</b>.
			</blockquote>
		@endparblock

		<b>Walkthrough and example</b>

		This also functions as a small tutorial for those unfamiliar with
		<tt>tup</tt>.

		Tup files look confusing but are quite simple. They work based on
		<tt>:</tt> rules which define some recipe like
		<tt>inputs |> shell command |> outputs</tt>. The inputs and outputs
		of the <tt>:</tt> rules are used by tup to create a directed acyclic
		graph which describes the build order and file-based dependencies.

		Tests are referred to by the name of the folder that contains the
		test's <tt>Tupfile</tt>, which is the "current working directory" for
		that test. (<tt>%d</tt> in rules, from here on <tt>[CWD]</tt>).

		An example <tt>Tupfile</tt>:

		@code{.sh}
		### Unmodified. If you modify this, remove this line and document your changes.
		include_rules
		: foreach src / *.cpp | $(TEST_PREREQUISITE) |> !C |> $(TEST_OBJ_DIR)/%d/%B.o {test_objs}
		: {test_objs} |> !L |> $(TEST_BIN_DIR)/%d/%d.exe {unit_test}
		: {unit_test} |> !RUN_TEST |> $(TEST_DIR)/<%d>
		: | $(TEST_DIR)/<%d> |> !PASSTHROUGH |> <unit_test_results>
		@endcode

		@note @parblock
			<tt>$()</tt> macros like <tt>TEST_OBJ_DIR</tt> are defined in a
			higher-level <tt>Tuprules.tup</tt>.

			Some tests require specialized rules, like passing arguments to the
			test program. In such cases, like <tt>fgl_io_binary_files</tt>, the
			@code{.sh}
			### Unmodified. If you modify this, remove this line and document your changes.
			@endcode
			comment is removed and the changes are documented.
		@endparblock

		<ol type="1">
		<li>
			<tt>include_rule</tt> includes all <tt>Tuprules.tup</tt> files
			from the CWD and above.
		</li>
		<li>
			The first <tt>:</tt> rule requires <tt>$(TEST_PREREQUISITES)</tt>
			which is optionally defined in the same-level <tt>Tuprules.tup</tt>.
			Assuming the requirements are met, it compiles each <tt>.cpp</tt>
			file to an <tt>.o</tt> file of the same name and output to
			<tt>$(TEST_OBJ_DIR)/[CWD]/</tt> which is then grouped into a
			local tup "bin" specific to the test, named <tt>{test_objs}</tt>.
		</li>
		<li>
			The second <tt>:</tt> rule links all of the object files referred
			to by <tt>{test_objs}</tt> to produce a
			<tt>$(TEST_BIN_DIR)/[CWD]/[CWD].exe</tt>, which is grouped into a
			local tup bin named <tt>{unit_test}</tt>.
		</li>
		<li>
			The third <tt>:</tt> rule executes the unit test program. If the
			test passes (returns <tt>EXIT_SUCCESS</tt>) it's added to the tup
			group <tt>$(TEST_DIR)/<[CWD]></tt>, so that other tests may refer
			to this one by it's directory.
		</li>
		<li>
			The fourth <tt>:</tt> rule forwards the <tt>$(TEST_DIR)/<[CWD]></tt>
			test result to a global <tt><unit_test_results></tt> group. This
			is the group which is required by the top-level project
			<tt>Tupfile</tt> (simply
			<tt>: | <unit_test_results> |> |></tt>; a rule which requires all
			test results as input).
		</li>
		</ol>

		An example <tt>Tuprules.tup</tt>:
		Using <tt>test/fgl_utility_enumerate</tt> as an example:
		@code{.sh}
		TEST_PREREQUISITE= $(TEST_DIR)/<fgl_utility_zip> $(TEST_DIR)/<fgl_types_traits>
		@endcode

		Here, <tt>fgl_utility_enumerate</tt> requires that the
		<tt>fgl_utility_zip</tt> and <tt>fgl_types_traits</tt> tests pass
		before <tt>fgl_utility_enumerate</tt>'s test is attempted.
	@endparblock
@endinternal
*/

#endif // FGL_HPP_INCLUDED
