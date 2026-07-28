#!/bin/bash

media_dir="/mnt/Vault/Media/Anime/"
db_dir="/mnt/Vault/obsidian/media/DB/Anime/"

# Get a list of directories in media_dir
find "$media_dir" -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | sort > /tmp/media_folders.txt

# Get a list of markdown file names (without extension) in db_dir
find "$db_dir" -maxdepth 1 -mindepth 1 -type f -name "*.md" -printf '%f\n' | sed 's/\.md$//' | sort > /tmp/db_md_files.txt

# Compare the two lists and show folders present in media_folders but not in db_md_files
echo "Folders without corresponding .md files:"
comm -23 /tmp/media_folders.txt /tmp/db_md_files.txt

# Clean up temporary files
rm /tmp/media_folders.txt /tmp/db_md_files.txt
