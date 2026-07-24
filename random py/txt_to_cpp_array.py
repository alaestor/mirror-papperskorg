import logging
from typing import Liste

ENCODING = "UTF-8"

def readlines(path: str) -> List[str]:
	with open(path, encoding=ENCODING, mode="r") as file:
		return [line.rstrip() for line in file.readlines()] # if line[0] != '#'

if __name__ == "__main__":
	try:
		input_filepath="./input.txt"
		output_filepath="./output.txt"
		array_name="PLACEHOLDER"

		lines = readlines(input_filepath)
		elements = "\n".join([f'\t"{line}",' for line in lines])
		output = f"static constexpr std::array<std::string_view, {len(lines)}> {array_name}{{\n{elements}\n}};"
		with open(output_filepath, encoding=ENCODING, mode="w") as output_file:
			output_file.seek(0)
			output_file.write(output)
			output_file.truncate()
	except Exception as e:
		logging.critical(str(e))
