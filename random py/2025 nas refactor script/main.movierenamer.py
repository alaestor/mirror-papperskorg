import re
import sys
from pathlib import Path

# Using an older import style for IMDbPY compatibility
from imdb import IMDb

from jellybuilder_common import find_relative_files

def sanitize_filename(title: str) -> str:
	'''Removes characters invalid in most OS filenames.'''
	# Remove invalid characters
	sanitized = re.sub(r'[<>:"/\\|?*]', '', title)
	# Replace colons used in subtitles, often separated by a space
	sanitized = sanitized.replace(': ', ' - ')
	return sanitized.strip()


def main():
	'''Main execution function.'''
	try:
		ia = IMDb()
		ipath = Path('/mnt/Vault/Media/Movies/')
		t = [ipath / p for p in find_relative_files(ipath, '*.mkv', '*.mp4', '*.avi')]

		if not t:
			print('No movie files found to process.')
			return

		ntn = '/mnt/Vault/Media/Movies/Kill Bill/(2003) Kill Bill Volume 1.mp4'
		index = t.index(Path(ntn))
		movie_files = t[index:]

		#movie_files = t

	except Exception as e:
		print(f'Error during initialization: {e}', file=sys.stderr)
		return

	for file_path in movie_files:
		print(f"\nProcessing: '{file_path}'\n\t{file_path.stem}\n")

		try:
			# Get search query from user, default to file's stem
			default_query = file_path.stem.replace('.', ' ')
			prompt = f"  Enter search query, '0' to skip, or press Enter to use the file name\n\n\t: "
			query = input(prompt) or default_query
			if query == '0':
				continue

			results = ia.search_movie(query, results='movies')
			if not results:
				print(f"  No results found for '{query}'. Skipping.")
				continue

			# Display up to 9 results
			print(f"  Found {len(results)} results for '{query}':")
			display_count = min(len(results), 9)
			for i, movie in enumerate(results[:display_count], start=1):
				title = movie.get('long imdb canonical title', 'N/A')
				kind = movie.get('kind', '')
				#year = movie.get('year', 'N/A')
				print(f'\t{i}: {title} {kind}')

			# Get user's choice
			selected_movie = None
			while not selected_movie:
				choice_prompt = (
					f'  Choose 1-{display_count}, or enter /tt<id> : '
				)
				choice = input(choice_prompt).strip().lower()

				if choice.startswith('/tt'):
					try:
						imdb_id = choice.replace('/tt', '').strip()
						print('  Fetching by ID...')
						selected_movie = ia.get_movie(imdb_id)
					except Exception:
						print('  Invalid IMDb ID or network error. Please try again.')
					continue

				try:
					choice_idx = int(choice) - 1
					if 0 <= choice_idx < display_count:
						movie_summary = results[choice_idx]
						print('  Fetching details...')
						selected_movie = ia.get_movie(movie_summary.movieID)
					else:
						print(f'  Invalid number. Please enter 1-{display_count}.')
				except ValueError:
					print('  Invalid input. Please enter a number or an IMDb ID.')

			# Rename the file
			if selected_movie:
				title = sanitize_filename(selected_movie.get('title'))
				year = selected_movie.get('year')
				imdb_id = selected_movie.movieID
				extension = file_path.suffix

				new_name = f'{title} ({year}) [imdbid-tt{imdb_id}]{extension}'
				new_path = file_path.with_name(new_name)
				if new_path.is_file():
					raise RuntimeError(f'path already exists: {new_path}')

				print(f"  Renaming '{file_path.name}'\n -> '{new_path.name}'")
				file_path.rename(new_path)

		except Exception as e:
			print(f'An unexpected error occurred: {e}', file=sys.stderr)
			continue


if __name__ == '__main__':
	main()
