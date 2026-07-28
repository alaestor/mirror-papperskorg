import logging
import re
import shutil
import traceback
from pathlib import Path

import frontmatter
from thefuzz import process, fuzz

from util import setup_logging, exception_prompter

# --- CONFIGURATION ---
SHOW_PATH = Path('/mnt/Vault/Media/Anime/')  # Target directories
FILE_PATH = Path('/mnt/Vault/obsidian/media/DB/Anime/')  # Source *.md files
POSTER_PATH = FILE_PATH / 'storage'  # Source *.poster.jpg files

# --- CONSTANTS ---
NFO_TEMPLATE = '''<?xml version="1.0" encoding="utf-8"?>
<tvshow>
	<title>{name}</title>
	<originaltitle>{aliases}</originaltitle>
	<thumb aspect="poster">poster.jpg</thumb>
{studios}
{genres}
{tags}
	<tag>Anime</tag>
</tvshow>
'''
LEADING_ARTICLES_RE = re.compile(r'^(A|An|The)\s+', re.IGNORECASE)
METADATA_RE = {
	'genres': re.compile(r'Media/Genres/(.+)'),
	'tags': re.compile(r'Media/Tags/(.+)'),
	'studios': re.compile(r'Media/Studios/(.+)'),
}

def seasonderp(path: str|Path) -> str:
	# 0 season 1  -> [00] season 1
	def replacer(match):
		number = int(match.group(1))
		return f"[{number:02d}]"
	return re.sub(r"^(\d+)", replacer, str(path))


def handle_backup(target_path: Path):
	'''
	If target_path exists, backs it up to target_path.old.
	If a .old backup already exists, the original file is left to be overwritten.
	'''
	if not target_path.exists():
		return

	backup_path = target_path.with_suffix(target_path.suffix + '.old')
	if not backup_path.exists():
		try:
			target_path.rename(backup_path)
			logging.info(f'Backed up {target_path.name} to {backup_path.name}')
		except OSError as e:
			logging.error(f'Could not back up {target_path}: {e}')

"""
def get_user_choice(file_stem: str, choices: list) -> int:
	'''Displays choices and gets a validated user selection.'''
	print(f"\n--- Matching ---\n  -> {file_stem}")
	if not choices:
		print('No potential matches found.')
		return 0

	for i, (name, score) in enumerate(choices, 1):
		print(f'  {i}: {name} (Score: {score})')

	while True:
		try:
			selection = int(input('Select a match (1-9) or 0 to skip: '))
			if 0 <= selection <= len(choices):
				return selection
			else:
				print(f'Invalid input. Please enter a number between 0 and {len(choices)}.')
		except ValueError:
			print('Invalid input. Please enter a number.')
"""

from typing import Optional

def get_user_choice(file_stem: str, choices: list[tuple[str, int]]) -> str|None:
	'''
	Displays choices and gets a validated user selection or custom path.

	Args:
		file_stem: The name of the file being matched.
		choices: A list of tuples, where each tuple contains a
				 potential match (str) and its score (int).

	Returns:
		The selected match name (str), a custom path (str), or None if
		the user chooses to skip.
	'''
	print(f'\n--- Matching ---\n  -> {file_stem}')
	if not choices:
		print('No potential matches found.')
	for i, (name, score) in enumerate(choices, 1):
		print(f'  {i}: {name} (Score: {score})')
	prompt = (f'Select a match (1-{len(choices)}), a custom path, or 0 to skip: ')
	while True:
		selection = input(prompt)
		if selection.startswith('/'):
			return selection.lstrip('/')
		try:
			choice_num = int(selection)
			if choice_num == 0:
				return None
			if 1 <= choice_num <= len(choices):
				return choices[choice_num - 1][0]
			print(f'Invalid number. Please enter a number between 0 and {len(choices)}.')
		except ValueError:
			print('Invalid input. Please enter a number or a path starting with "/"')

#def season_renamer(path: str) -> str:
#	def replacer(match):
#		number = int(match.group(1))
#		return f"[{number:02d}]"
#	return re.sub(r"^(\d+)", replacer, path).strip()

def process_selection(md_file: Path, selected_dir_name: str):
	'''
	Handles all file operations for a user-confirmed match.
	'''
	try:
		# 1. Parse Frontmatter
		post = frontmatter.load(md_file)
		metadata = post.metadata
		if not metadata:
			logging.warning(f'No metadata found in {md_file.name}. Skipping.')
			return

		# 2. Prepare data
		file_stem = md_file.stem
		selected_dir_path = SHOW_PATH / selected_dir_name
		image_filename = (metadata.get('image') or '').strip('[]')
		source_poster_path = POSTER_PATH / image_filename

		# 3. Copy Poster
		if image_filename and source_poster_path.exists():
			dest_poster_path = selected_dir_path / 'poster.jpg'
			handle_backup(dest_poster_path)
			shutil.copy(source_poster_path, dest_poster_path)
			logging.debug(f'Copied poster to "{dest_poster_path}"')
		elif image_filename:
			logging.warning(f'Poster file not found: "{source_poster_path}"')

		# 4. Create tvshow.nfo
		nfo_path = selected_dir_path / 'tvshow.nfo'
		handle_backup(nfo_path)

		# Extract and format tags, genres, studios
		extracted_meta = {'genres': [], 'tags': [], 'studios': []}
		for tag in metadata.get('tags', []):
			for key, pattern in METADATA_RE.items():
				if match := pattern.match(tag):
					extracted_meta[key].append(match.group(1))
					break # A tag can only match one category

		nfo_content = NFO_TEMPLATE.format(
			name=file_stem,
			aliases=' ; '.join(metadata.get('aliases', [])),
			studios='\n'.join([f'\t<studio>{s}</studio>' for s in extracted_meta['studios']]),
			genres='\n'.join([f'\t<genre>{g}</genre>' for g in extracted_meta['genres']]),
			tags='\n'.join([f'\t<tag>{t}</tag>' for t in extracted_meta['tags']])
		)

		nfo_path.write_text(nfo_content, encoding='utf-8')
		logging.debug(f'Created NFO file at "{nfo_path}"')

		# 5. Rename Folder
		new_name = LEADING_ARTICLES_RE.sub('', file_stem)
		new_dir_path = SHOW_PATH / new_name

		if new_dir_path == selected_dir_path:
			logging.debug(f'Directory "{selected_dir_path.name}" already has the correct name.')
			return

		handle_backup(new_dir_path)
		selected_dir_path.rename(new_dir_path)
		logging.info(f'Renamed directory "{selected_dir_path.name}" to "{new_dir_path.name}"')

	except FileNotFoundError as e:
		logging.error(f'File not found during processing for {md_file.name}: {e}')
	except Exception as e:
		logging.error(f'An unexpected error occurred for {md_file.name}: {e}')
		logging.error(''.join(traceback.format_exc()))

@exception_prompter
def main():
	'''Main script logic.'''
	md_files = sorted([p for p in FILE_PATH.glob('*.md') if p.is_file()])
	show_dirs = [p.name for p in SHOW_PATH.iterdir() if p.is_dir()]

	if not md_files or not show_dirs:
		logging.error('Source files or destination directories not found. Exiting.')
		return

	logging.info(f'Found {len(md_files)} files and {len(show_dirs)} directories.')

	# TODO map from md -> folder skipping things that have tvshow.nfo
	#"""
	### Speeding things along; comment this out
	target_filename = "Lost Village.md"
	# Find the index of the target file
	start_index = next(i for i, path in enumerate(md_files) if path.name == target_filename)
	# Slice the list from that index onwards
	md_files = sorted(md_files[start_index:])
	#"""

	#scorer=fuzz.WRatio            # default
	#scorer=fuzz.ratio             # good for similar lengths
	#scorer=fuzz.partial_ratio     # good for substrings
	#scorer=fuzz.token_sort_ratio  # ignores word order
	#scorer=fuzz.token_set_ratio   # ignores work order & duplicates
	scorer=fuzz.partial_token_sort_ratio

	for md_file in md_files:
		file_stem = LEADING_ARTICLES_RE.sub('', md_file.stem)

		# skip configured shows
		if file_stem in show_dirs and (SHOW_PATH /file_stem / 'tvshow.nfo').is_file():
			continue

		choices = process.extract(file_stem, show_dirs, limit=9, scorer=scorer)
		selection = get_user_choice(file_stem, choices)

		if selection:
			logging.info(f"Processing '{file_stem}' -> '{selection}'...")
			process_selection(md_file, selection)
		else:
			logging.info(f"Skipping '{file_stem}'.")


if __name__ == '__main__':
	try:
		setup_logging()
		main()
		logging.info('Script finished successfully.')
	except Exception as e:
		# This outer catch is for issues during setup or unhandled in main
		logging.critical(f'A critical exception occurred: {e}\n\n{"".join(traceback.format_exc())}')
		exit(1)
