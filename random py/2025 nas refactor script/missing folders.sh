#!/bin/bash

media_dir="/mnt/Vault/Media/Anime/"
db_dir="/mnt/Vault/obsidian/media/DB/Anime/"

# Get a list of markdown file names (without extension) in db_dir
find "$db_dir" -maxdepth 1 -mindepth 1 -type f -name "*.md" -printf '%f\n' | sed 's/\.md$//' | sort > /tmp/db_md_files.txt

# Get a list of directories in media_dir
find "$media_dir" -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | sort > /tmp/media_folders.txt

# Compare the two lists and show markdown files present in db_md_files but not in media_folders
echo "MD files without corresponding folders:"
comm -23 /tmp/db_md_files.txt /tmp/media_folders.txt

# Clean up temporary files
rm /tmp/db_md_files.txt /tmp/media_folders.txt
