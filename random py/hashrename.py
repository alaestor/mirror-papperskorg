import shutil
from tkinter import filedialog
import os
import hashlib

def aquire_empty_subfolder(folder_path, folder_name):
	assert len(folder_path) > 0
	assert len(folder_name) > 0
	assert os.path.isdir(folder_path)
	subfolder_path = os.path.join(folder_path, folder_name)
	if not os.path.exists(subfolder_path):
		os.makedirs(subfolder_path)
	elif len(os.listdir(subfolder_path)) > 0:
		raise Exception(
			"The \""
			+ subfolder_path
			+ "\" subdirectory already exists and is not empty!"
		)

	print("Subfolder created: " + subfolder_path)
	return subfolder_path

def rename_with_hash(file_path):
	assert len(file_path) > 0
	assert(os.path.isfile(file_path))
	file_hash = hashlib.md5(open(file_path, 'rb').read()).hexdigest()
	folder_path = os.path.split(file_path)[0]
	file_extension = os.path.splitext(file_path)[1]

	new_file_name = file_hash + file_extension
	new_file_path = os.path.join(folder_path, new_file_name)

	try:
		os.rename(file_path, new_file_path)
		print("File renamed: " + new_file_path)
	except OSError:
		print(
			"Error renaming file: "
			+ file_path
			+ " (E: " + OSError.errno + ")"
		)

def main():
	input_path = filedialog.askdirectory()
	print("Renaming files in " + input_path)

	output_path = aquire_empty_subfolder(input_path, "hashrenamed")

	for file_name in os.listdir(input_path):
		input_file_path = os.path.join(input_path, file_name)
		if os.path.isfile(input_file_path):
			output_file_path = os.path.join(output_path, file_name)
			shutil.copy(input_file_path, output_file_path)
			rename_with_hash(output_file_path)

if __name__ == "__main__":
	main()
