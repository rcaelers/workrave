#!/bin/bash

WORKSPACE_DIR=$(pwd)
BUILD_DIR=${WORKSPACE_DIR}/_build
DEPLOY_DIR=${WORKSPACE_DIR}/_deploy
DEPLOY32_DIR=${WORKSPACE_DIR}/_build/.32/_deploy

INSTALLERS_FILE="$BUILD_DIR/installers.txt"
RUNTIME_INSTALLERS_FILE="$BUILD_DIR/runtime_installers.txt"
RUNTIME32_INSTALLERS_FILE="$BUILD32_DIR/.32/runtime_installers.txt"
MSYS_INSTALLERS_FILE="$BUILD_DIR/msys_installers.txt"
SBOM_FILE="$BUILD_DIR/sbom.txt"

if [ ! -d "$BUILD_DIR/.cmake/api/v1/reply" ]; then
  echo "CMake File API data not found."
  exit 1
fi

DIRECTORY_FILES=$(find "$BUILD_DIR/.cmake/api/v1/reply" -name "directory-*.json")
if [ -z "$DIRECTORY_FILES" ]; then
  echo "No directory JSON files found."
  exit 1
fi

for dir_file in $DIRECTORY_FILES; do
  jq -r '
    .installers[]? |
    select(.type == "file" or .type == "directory") |
    .destination as $dest |
    .paths[]? |
    if type == "string" then
      select(startswith("C:/msys64")) |
      .[9:] as $source_trimmed |
      "\($source_trimmed),\($dest)"
    else
      .from as $source |
      select($source | startswith("C:/msys64")) |
      "\($source[9:]),\($dest | sub("/[^/]+/?$"; ""))"
    end
  ' "$dir_file"
done > ${INSTALLERS_FILE}

cat ${RUNTIME_INSTALLERS_FILE} | sed 's|C:/msys64||'>> ${INSTALLERS_FILE}
cat ${RUNTIME_INSTALLERS_FILE} | sed 's|C:/msys64||'>> ${INSTALLERS_FILE}

declare -A installer_map
while IFS=, read -r source destination; do
  source=$(echo "$source" | sed 's/[[:space:]]*$//;s:/*$::;s:/\.$::')
  destination=$(echo "$destination" | sed 's/[[:space:]]*$//;s:/*$::;s:/\.$::')
  name=$(basename "$source")
  installer_map["$destination/$name"]="$source"
done < ${INSTALLERS_FILE}

>${MSYS_INSTALLERS_FILE}

find "$DEPLOY_DIR" "$DEPLOY32_DIR" -type f | while read -r file; do
  relative_path="${file#$DEPLOY_DIR/}"
  found=false

  while [ -n "$relative_path" ]; do
    if [[ -n "${installer_map[$relative_path]}" ]]; then
      echo "Matched: $file (Source: ${installer_map[$relative_path]})"
      echo "${installer_map[$relative_path]}" >> ${MSYS_INSTALLERS_FILE}
      found=true
      break
    fi
    if [[ "$relative_path" == "${relative_path%/*}" ]]; then
      break
    fi
    relative_path="${relative_path%/*}"
  done

  if [ "$found" = false ]; then
    echo "Not Found: $file"
  fi
done

sort -u -o ${MSYS_INSTALLERS_FILE} ${MSYS_INSTALLERS_FILE}

> ${SBOM_FILE}
while IFS= read -r filename; do
    packages=$(pacman -Qo "$filename" | awk '{print $5,$6}')
    package_count=$(echo "$packages" | wc -l)
    if [ "$package_count" -eq 0 ]; then
        echo "$filename belongs to Unknown"
    elif [ "$package_count" -eq 1 ]; then
        echo "$filename belongs to $packages"
        echo "$packages" >> ${SBOM_FILE}
    else
        echo "Error: $filename is found in multiple packages: $packages"
    fi
done < ${MSYS_INSTALLERS_FILE}
