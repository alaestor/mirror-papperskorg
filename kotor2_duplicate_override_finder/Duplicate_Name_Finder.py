"""
@file
@date 2022-05-17
@author Alaestor Weissman (discord <tt>Honshitsu#9218</tt>)
@details Originally written to find KOTOR2 override mod conflicts
"""

import os

"""!
@brief Finds filenames that occur more than once in a directory tree.
@details Recursively searches the @p path and reports the paths to files whose
	names occur more than once in the directory tree.
@attention The filenames are made lowercase before comparison.
@param path The path to recursively search.
@param excluded_exts A list of lower-case file extentions to ignore.
@param aliased_exts A list of lists of lower-case file extentions. All
	extentions in a list will be treated as the first extention. For example,
	@c [[".tga", ".tpc"]] will treat both ".tga" and ".tpc" file names as
	".tga", meaning that if you have a @c name.tga and a @c name.tpc , the
	@c name.tpc will be listed as a duplicate of @c name.tga .
@return A dictionary of lists of paths correlated by filename
"""
def paths_to_duplicated_filenames(path, excluded_exts=[], aliased_exts=[[]]):
	d = {}
	# Create lists of paths correlated by filename
	for root, _, files in os.walk(path):
		# Filter out files with excluded extentions
		filenames = [f for f in files if os.path.splitext(f)[1].lower() not in excluded_exts]
		for name in filenames:
			# key will be lowercased filename with aliased extention
			key = name.lower()
			split = os.path.splitext(key)
			for exts in aliased_exts:
				if split[1] in exts: # if ext is listed as an alias
					key = split[0] + exts[0] # use first alias
			if key in d.keys(): # append to existing list of paths
				d[key].append(os.path.join(root, name))
			else: # create a new list of paths
				d[key] = [os.path.join(root, name)]
	# Remove entries that have only one path
	for key in list(d.keys()):
		if len(d[key]) == 1:
			d.pop(key)
	# The remainder are filenames which exist in more than one place
	return d

"""!
@details @parblock
@note asserts that the current working directory is <tt>override</tt>;
	this isn't crucial but helps prevent accidents with main.

Finds and prints the paths of filenames that occur two or more times in a
directory tree.

@endparblock
"""
def main():
	cwd = os.getcwd()
	excludes = [".txt", ".rtf"]
	sames = [[".tga", ".tpc"]]
	assert os.path.basename(os.path.normpath(cwd)).lower() == "override"
	print("\n\nFinding duplicated filenames.")
	print("  Excluding:", excludes)
	samestr = "'{}' <- {}"
	if len(sames) > 1:
		print("  Aliasing:")
		for alias in sames:
			print("\t" + samestr.format(alias[0], alias[1:]))
	else:
		print("  Aliasing:", samestr.format(sames[0][0], sames[0][1:]))
	print("  Path:", cwd, "\n")
	duplicates = paths_to_duplicated_filenames(cwd, excludes, sames)
	for count, filename in enumerate(duplicates.keys()):
		print("{}. {} has {} duplicates:".format(count, filename, len(duplicates[filename])))
		for path in duplicates[filename]:
			print("  ." + os.sep + os.path.relpath(path, cwd))
		print("\n")

if __name__ == "__main__":
	main()
