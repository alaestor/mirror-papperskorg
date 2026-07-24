from conans.model import Generator

from conans.client.build.compiler_flags import (
	architecture_flag,
	build_type_define,
	build_type_flags,
	format_defines,
	format_include_paths,
	format_libraries,
	format_library_paths,
	libcxx_define,
	libcxx_flag,
	rpath_flags,
	sysroot_flag,
	visual_linker_option_separator,
	visual_runtime,
	format_frameworks,
	format_framework_paths)

class FormattedDeps(object):

	def fmt_str(self, function, content, settings):
		return " ".join(function(content, settings)).replace("\\", "/")

	def __init__(self, deps_cpp_info, settings, compiler):
		# NOTE: hacky, hardcoded, inefficient, gcc-specific,...
		self.include_paths = self.fmt_str(format_include_paths, deps_cpp_info.include_paths, settings)
        # .replace("-I", "-i")
		self.lib_paths = self.fmt_str(format_library_paths, deps_cpp_info.lib_paths, settings)
		self.libs = self.fmt_str(format_libraries, deps_cpp_info.libs, settings)
		self.defines = " ".join(format_defines(deps_cpp_info.defines)).replace("\\", "/")
		flagsep = " -"
		self.cppflags = flagsep.join(deps_cpp_info.cppflags).replace("\\", "/")
		self.cflags = flagsep.join(deps_cpp_info.cflags).replace("\\", "/")
		self.sharedlinkflags = flagsep.join(deps_cpp_info.sharedlinkflags).replace("\\", "/")
		self.exelinkflags = flagsep.join(deps_cpp_info.exelinkflags).replace("\\", "/")
		self.rootpath = "%s" % deps_cpp_info.rootpath.replace("\\", "/")

class MyGeneratorName(Generator):
	@property
	def filename(self):
		return "conan_dependencies.tup"

	@property
	def compiler(self):
		return self.conanfile.settings.get_safe("compiler")

	@property
	def _settings(self):
		settings = self.conanfile.settings.copy()
		if self.settings.get_safe("compiler"):
			settings.compiler = self.compiler
		return settings

	@property
	def content(self):
		deps = FormattedDeps(self.deps_build_info, self._settings, self.compiler)

		template = (
			'CONAN_INCLUDE_DIR{dep}= {deps.include_paths}\n\n'
			'CONAN_LIB_DIR{dep}= {deps.lib_paths}\n\n'
			'CONAN_LIBS{dep}= {deps.libs}\n\n'
			'CONAN_DEFINES{dep}= {deps.defines}\n\n'
			'CONAN_CPPFLAGS{dep}= {deps.cppflags}\n\n'
			'CONAN_CFLAGS{dep}= {deps.cflags}\n\n'
			'CONAN_SHAREDLINKFLAGS{dep}= {deps.sharedlinkflags}\n\n'
			'CONAN_EXELINKFLAGS{dep}= {deps.exelinkflags}\n\n'
			'CONAN_COMPILE{dep}= $(CONAN_INCLUDE_DIR{dep}) $(CONAN_DEFINES{dep}) $(CONAN_CPPFLAGS{dep}) $(CONAN_CFLAGS{dep}) $(CONAN_SHAREDLINKFLAGS{dep})\n\n'
			'CONAN_LINK{dep}= $(CONAN_COMPILE{dep}) $(CONAN_LIB_DIR{dep}) $(CONAN_LIBS{dep})\n\n')

		sections = [
			"# Generated tup macros for conan dependencies\n"
			"#\n"
			"# Global composate macros:\n"
			"#\tCONAN_INCLUDE_DIR\n"
			"#\tCONAN_LIB_DIR\n"
			"#\tCONAN_LIBS\n"
			"#\tCONAN_DEFINES\n"
			"#\tCONAN_CPPFLAGS\n"
			"#\tCONAN_CFLAGS\n"
			"#\tCONAN_SHAREDLINKFLAGS\n"
			"#\tCONAN_EXELINKFLAGS\n"
			"#\tCONAN_ALL\n"
			"#\n"
			"# Library specific macros use _library suffix\n"
			"# For example:\tCONAN_LIBS_opencv\n"]
		all_flags = template.format(dep="", deps=deps)
		sections.append(all_flags)

		template_deps = template + 'conan_rootpath{dep} = "{deps.rootpath}"\n'
		for dep_name, dep_cpp_info in self.deps_build_info.dependencies:
			deps = FormattedDeps(dep_cpp_info, self._settings, self.compiler)
			dep_name = dep_name.replace("-", "_")
			dep_flags = template_deps.format(dep="_" + dep_name, deps=deps)
			sections.append("\n\n\n# Conan package: " + dep_name + "\n")
			sections.append(dep_flags)

		return "\n".join(sections)

from conans import ConanFile

class ProjectConan(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = "libFGL/latest@alaestor/main" # comma-separated list of requirements
	generators = "MyGeneratorName"
	default_options = {}
