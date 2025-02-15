#!/bin/bash

### Script to help moving .cpp/.h files with git mv.
move_files() {
  local filename="$1"
  local folder="$2"

  for ext in h cpp; do
    source_file="$filename.$ext"
    destination_file="$folder/$filename.$ext"

    if [ ! -f "$source_file" ]; then
      echo "Warning: File '$source_file' does not exist. Skipping."
    else
      git mv "$source_file" "$destination_file"
      if [ $? -eq 0 ]; then
        echo "Moved '$source_file' to '$destination_file'"
      else
        echo "Error: Failed to move '$source_file'."
        exit 1
      fi
    fi
  done
}

# Check for options
while [ $# -gt 0 ]; do
  case "$1" in
    -a | --all )
      all_flag=true
      shift
      ;;
    -h | --help )
      echo "Usage: $0 [-a|--all] <filename1> <filename2> ... <folder>"
      echo "   -a, --all: Move all .h and .cpp files in the current directory."
      exit 0
      ;;
    * ) # No more options
      break
      ;;
  esac
done

# The last argument is the folder
folder="${@: -1}"

# Check if the folder exists
if [ ! -d "$folder" ]; then
  echo "Error: Folder '$folder' does not exist."
  exit 1
fi

if [ "$all_flag" = true ]; then
  # Move all .h and .cpp files in the current directory
  for file in *.h; do
    filename="${file%.*}" # Extract filename without extension
    move_files "$filename" "$folder"
  done
else
  # Move specified files
  filenames=("${@:1:$(($#-1))}")
  for filename in "${filenames[@]}"; do
    move_files "$filename" "$folder"
  done
fi

echo "Done."