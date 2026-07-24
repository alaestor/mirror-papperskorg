from conans import ConanFile, tools

class libFGL_conan(ConanFile):
	name = "libFGL"
	homepage = "https://alaestor.github.io/libFGL"
	url = "https://github.com/alaestor/libFGL"
	description = "A modern C++20 header-only library intended for use with Tempus Edax Rerum projects."
	author = "Alaestor Weissman"
	# No other fields are necessary; header only
	exports_sources = "include/*"
	no_copy_source = True

	"""
	VERSION MUST BE SUPPLIED TO `conan create`
	e.g.
		conan create . "libFGL/0.1.0@user/channel"
	"""
	def set_version(self):
		git = tools.Git(folder=self.recipe_folder)
		tag = git.get_tag()
		if (tag):
			self.version = tag
		else:
			self.version = f"{git.get_branch()}-{git.get_revision()}"

	def package(self):
		self.copy("*.hpp")
		self.copy("FGLM")

	def package_id(self):
		self.info.header_only()

