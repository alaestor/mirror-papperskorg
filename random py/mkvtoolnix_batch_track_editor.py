import os
import shutil
from tkinter import filedialog
import tkinter
import pymkv
import logging
import glob

def aquire_empty_subfolder(folder_path, folder_name):
	assert len(folder_path) > 0
	assert len(folder_name) > 0
	assert os.path.isdir(folder_path)
	subfolder_path = os.path.join(folder_path, folder_name)
	if not os.path.exists(subfolder_path):
		os.makedirs(subfolder_path)
		logging.info(f"Subfolder {subfolder_path} created.")
	elif len(os.listdir(subfolder_path)) > 0:
		raise Exception(f"The \"{subfolder_path}\" subdirectory already exists and is not empty!")
	else:
		logging.info(f"Subfolder {subfolder_path}")

	return subfolder_path

### BROKEN didny bother fixing
def main():
	input_path = filedialog.askdirectory()
	logging.basicConfig(level=logging.DEBUG)
	logging.info("Looking for MKV files in " + input_path)

	output_path = aquire_empty_subfolder(input_path, "__output")


	# for every MKV in directory
	# filenames = glob.glob(os.path.join(input_path, "*.mkv"), recursive=False)
	filenames = [f for f in os.listdir(input_path)
		if os.path.isfile(os.path.join(input_path, f))
			and os.path.splitext(f)[1].lower() == os.extsep + "mkv"]

	logging.info(f"found files:{filenames}")

	files = {}
	i = 0
	for filename in filenames:
		files[filename] = pymkv.MKVFile(os.path.join(input_path, filename))
		i = i + 1
		if (i == 3):
			break

	tracks = []
	for k,file in files.items():
		if (len(tracks) == 0):
			tracks = file.get_track()
			logging.debug(f"initial tracks from {k}: {tracks}")
			continue
		t = file.get_track()
		logging.debug(f"getting tracks for {k}: {t}")
		assert(tracks == t)

	exit()

	new_tracks = tracks

	for t in tracks:
		print(f"{t.track_id} - {t.track_type} - {t.language} [{t.default_track}]")



	# for filename in filenames:
	# 	original = os.path.join(input_path, filename)
	# 	backup = os.path.join(backup, filename)
	# 	shutil.copy(original, backup)
	# 	#os.remove(original)
	# 	files[filename] = pymkv.MKVFile(backup)


if __name__ == "__main__":
	main()
