#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  WORKSPACE_DIR=$(pwd)
  BUILD_DIR=${WORKSPACE_DIR}/_build
  DEPLOY_DIR=${WORKSPACE_DIR}/_deploy
  OUTPUT_DIR=${WORKSPACE_DIR}/_deploy
fi

INSTALLERS_FILE="$BUILD_DIR/installers.txt"
RUNTIME_INSTALLERS_FILE="$BUILD_DIR/runtime_installers.txt"
RUNTIME32_INSTALLERS_FILE="$BUILD_DIR/.32/runtime32_installers.txt"
MSYS_INSTALLERS_FILE="$BUILD_DIR/msys_installers.txt"
MSYS_PACKAGES_FILE="$BUILD_DIR/msys_packages.txt"
EXTERNAL_SBOM_FILE="$BUILD_DIR/external-sbom.csv"
SBOM_FILE="$OUTPUT_DIR/sbom.csv"

>${MSYS_INSTALLERS_FILE}

# Function to run a command safely, capture stdout, and restore 'set -e' state
run_command_with_restore() {
  # Save the current state of 'set -e'
  if [[ $- == *e* ]]; then
    errexit_was_set=true
  else
    errexit_was_set=false
  fi

  # Temporarily disable 'set -e'
  set +e

  # Run the command, capturing stdout
  output=$("$@" 2>/dev/null)
  exit_code=$?

  # Restore the original state of 'set -e'
  if [ "$errexit_was_set" = true ]; then
    set -e
  else
    set +e
  fi

  # Return the command’s exit code and captured output
  echo "$output"
  return 0
}

# Continue with the rest of the script
sbom_precondition_check() {
  if [ ! -d "$BUILD_DIR/.cmake/api/v1/reply" ]; then
    echo "CMake File API data not found."
    exit 1
  fi
}

sbom_scan_installed_files() {
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
  done >${INSTALLERS_FILE}

  cat ${RUNTIME_INSTALLERS_FILE} | sed 's|C:/msys64||' >>${INSTALLERS_FILE}
  cat ${RUNTIME32_INSTALLERS_FILE} | sed 's|C:/msys64||' >>${INSTALLERS_FILE}
}

function sbom_scan_headers() {
  ninja -C $BUILD_DIR -t deps >$BUILD_DIR/deps.txt

  EXCLUDE_DIR="C:/msys64/clang64/include/c++|.*/$BUILD_DIR/_deps"
  CURRENT_DIR="$(pwd)"
  CURRENT_DIR="C:${CURRENT_DIR#/c}"

  processed_dirs=()

  sort -u $BUILD_DIR/deps.txt -o $BUILD_DIR/deps.txt
  grep -oP '(?<=\s\s\s\s)[^\s]+\.(h|hh|hpp|hxx)\b' $BUILD_DIR/deps.txt | grep -Ev "^($EXCLUDE_DIR|$CURRENT_DIR)" >$BUILD_DIR/deps-unique.txt

  cat $BUILD_DIR/deps-unique.txt | while read -r line; do
    include=true
    file_dir=$(dirname "$line")

    if [[ " ${processed_dirs[@]} " =~ " ${file_dir} " ]]; then
      include=false
    fi

    if $include; then
      echo "$line" | sed 's|C:/msys64||'
      processed_dirs+=("$file_dir")
    fi
  done >$MSYS_INSTALLERS_FILE
}

declare -A installer_map

sbom_create_installer_map() {
  while IFS=, read -r source destination; do
    source=$(echo "$source" | sed 's/[[:space:]]*$//;s:/*$::;s:/\.$::')
    destination=$(echo "$destination" | sed 's/[[:space:]]*$//;s:/*$::;s:/\.$::')
    name=$(basename "$source")
    if [[ $source == *"/clang64/"* || $source == *"/mingw32/"* ]]; then
      installer_map["$destination/$name"]="$source"
    fi
  done <${INSTALLERS_FILE}
}

sbom_create_msys_installed_files() {
  find "$OUTPUT_DIR" -type f | while read -r file; do
    relative_path="${file#$OUTPUT_DIR/}"
    found=false
    removed_path=""

    while [ -n "$relative_path" ]; do
      if [[ -n "${installer_map[$relative_path]}" ]]; then
        echo "Matched: $file (Source: ${installer_map[$relative_path]} + ${removed_path})"
        echo "${installer_map[$relative_path]}${removed_path}" >>${MSYS_INSTALLERS_FILE}
        found=true
        break
      fi
      base_name="${relative_path##*/}" # Get the last part of the relative path
      if [ -n "$removed_path" ]; then
        removed_path="/$base_name$removed_path"
      else
        removed_path="/$base_name"
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
}

sbom_create_msys2_package_list() {
  >${MSYS_PACKAGES_FILE}
  while IFS= read -r filename; do
    packages=$(run_command_with_restore pacman -Qo --config <(sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf) "$filename")
    pacman_exit_code=$?

    if [ "$pacman_exit_code" -ne 0 ]; then
      echo "Pacman failed for $filename, skipping."
      continue
    fi

    echo "$packages" | while IFS= read -r package_info; do
      echo "$filename belongs to $package_info"
      package=$(echo "$package_info" | awk '{ printf "%s,%s\n", $5,$6 }')
      echo "$package" >>${MSYS_PACKAGES_FILE}
    done
  done <${MSYS_INSTALLERS_FILE}

  sort -u ${MSYS_PACKAGES_FILE} -o ${MSYS_PACKAGES_FILE}
}

sbom_create_sbom() {
  echo "Package Name,Version,License,Description,URL" >$SBOM_FILE

  while IFS=, read -r package_name package_version; do
    pacman_info=$(run_command_with_restore pacman --config <(sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf) -Qi "$package_name")
    pacman_exit_code=$?

    if [ $pacman_exit_code -ne 0 ]; then
      echo "Pacman -Qi failed with exit code $exit_code, but script continues."
      continue
    fi

    if [[ -z "$pacman_info" ]]; then
      echo "Package $package_name not found, skipping..."
      continue
    fi

    installed_version=$(echo "$pacman_info" | awk -F ': ' '/^Version/ {print $2}')

    if [[ "$installed_version" != "$package_version" ]]; then
      echo "Version mismatch for $package_name (expected: $package_version, found: $installed_version), skipping..."
      continue
    fi

    # Extract details
    url=$(echo "$pacman_info" | awk -F ': ' '/^URL/ {print $2}')
    license=$(echo "$pacman_info" | awk -F ': ' '/^Licenses/ {print $2}')
    description=$(echo "$pacman_info" | awk -F ': ' '/^Description/ {print $2}')

    # Add package info to SBOM
    echo "$package_name,$package_version,$license,$description,$url" >>$SBOM_FILE

  done <"$MSYS_PACKAGES_FILE"

  cat $EXTERNAL_SBOM_FILE >>$SBOM_FILE
  echo "SBOM generated: $SBOM_FILE"
}

sbom() {
  sbom_precondition_check
  sbom_scan_installed_files
  sbom_scan_headers
  sbom_create_installer_map
  sbom_create_msys_installed_files
  sbom_create_msys2_package_list
  sbom_create_sbom
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  sbom
fi
