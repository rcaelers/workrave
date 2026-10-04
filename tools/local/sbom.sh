#!/bin/bash
shopt -s extglob

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  WORKSPACE_DIR=${SOURCES_DIR:-$(pwd)}
  BUILD_DIR=${BUILD_DIR:-${WORKSPACE_DIR}/_build}
  DEPLOY_DIR=${DEPLOY_DIR:-${WORKSPACE_DIR}/_deploy}
  OUTPUT_DIR=${OUTPUT_DIR:-${WORKSPACE_DIR}/_deploy}
  SOURCES_DIR=${SOURCES_DIR:-${WORKSPACE_DIR}}
  SCRIPTS_DIR=${SCRIPTS_DIR:-${SOURCES_DIR}/tools}
fi

# CMake uses drive-letter paths; the release scripts use MSYS paths. Keep
# the install-file lookup in one form when this script is sourced by build.sh.
if command -v cygpath >/dev/null 2>&1; then
  BUILD_DIR=$(cygpath -m "$BUILD_DIR")
  OUTPUT_DIR=$(cygpath -m "$OUTPUT_DIR")
fi

INSTALLERS_FILE="$BUILD_DIR/installers.txt"
RUNTIME_INSTALLERS_FILE="$BUILD_DIR/runtime_installers.txt"
RUNTIME32_INSTALLERS_FILE="$BUILD_DIR/.32/runtime32_installers.txt"
MSYS_INSTALLERS_FILE="$BUILD_DIR/msys_installers.txt"
MSYS_PACKAGES_FILE="$BUILD_DIR/msys_packages.txt"
EXTERNAL_SBOM_FILE="$BUILD_DIR/external-sbom.csv"
PACMAN_CONFIG="$BUILD_DIR/_sbom_pacman.conf"
SBOM_PKG_INFO_FILE="$BUILD_DIR/_sbom_pkginfo.tsv"

>${MSYS_INSTALLERS_FILE}

# Continue with the rest of the script
sbom_precondition_check() {
  if [ ! -d "$BUILD_DIR/.cmake/api/v1/reply" ]; then
    echo "CMake File API data not found."
    exit 1
  fi
}

sbom_scan_installed_files() {
  local dir_files
  dir_files=$(find "$BUILD_DIR/.cmake/api/v1/reply" -name "directory-*.json")
  if [ -z "$dir_files" ]; then
    echo "No directory JSON files found."
    exit 1
  fi

  # Single jq call across all directory files (instead of one per file)
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
  ' $dir_files >${INSTALLERS_FILE}

  sed 's|C:/msys64||' ${RUNTIME_INSTALLERS_FILE} >>${INSTALLERS_FILE}
  if [ -f ${RUNTIME32_INSTALLERS_FILE} ]; then
    sed 's|C:/msys64||' ${RUNTIME32_INSTALLERS_FILE} >>${INSTALLERS_FILE}
  fi
}

function sbom_scan_headers() {
  ninja -C $BUILD_DIR -t deps >$BUILD_DIR/deps.txt

  EXCLUDE_DIR="C:/msys64/clang64/include/c++|.*/$BUILD_DIR/_deps"
  CURRENT_DIR="$(pwd)"
  CURRENT_DIR="C:${CURRENT_DIR#/c}"

  sort -u $BUILD_DIR/deps.txt -o $BUILD_DIR/deps.txt
  grep -oP '(?<=\s\s\s\s)[^\s]+\.(h|hh|hpp|hxx)\b' $BUILD_DIR/deps.txt | grep -Ev "^($EXCLUDE_DIR|$CURRENT_DIR)" >$BUILD_DIR/deps-unique.txt

  # Single awk pass: deduplicate by directory, strip C:/msys64 prefix
  awk -F/ '{
    dir = ""
    for (i = 1; i < NF; i++) dir = dir (i > 1 ? "/" : "") $i
    if (!(dir in seen)) {
      seen[dir] = 1
      sub(/^C:\/msys64/, "")
      print
    }
  }' $BUILD_DIR/deps-unique.txt >$MSYS_INSTALLERS_FILE
}

declare -A installer_map

sbom_create_installer_map() {
  while IFS=, read -r source destination; do
    # Strip trailing whitespace, slashes, /. without spawning sed
    source="${source%%+([[:space:]])}" ; source="${source%%/}" ; source="${source%%/.}"
    destination="${destination%%+([[:space:]])}" ; destination="${destination%%/}" ; destination="${destination%%/.}"
    name="${source##*/}"
    if [[ $source == *"/clang64/"* || $source == *"/mingw32/"* ]]; then
      installer_map["$destination/$name"]="$source"
    fi
  done <${INSTALLERS_FILE}
}

sbom_create_msys_installed_files() {
  local file relative_path found removed_path base_name
  >"${MSYS_INSTALLERS_FILE}.tmp"

  while IFS= read -r file; do
    relative_path="${file#$OUTPUT_DIR/}"
    found=false
    removed_path=""

    while [[ -n "$relative_path" ]]; do
      if [[ -n "${installer_map[$relative_path]+_}" ]]; then
        echo "${installer_map[$relative_path]}${removed_path}" >>"${MSYS_INSTALLERS_FILE}.tmp"
        found=true
        break
      fi
      base_name="${relative_path##*/}"
      if [[ -n "$removed_path" ]]; then
        removed_path="/$base_name$removed_path"
      else
        removed_path="/$base_name"
      fi

      if [[ "$relative_path" == "${relative_path%/*}" ]]; then
        break
      fi
      relative_path="${relative_path%/*}"
    done

    if [[ "$found" == false ]]; then
      echo "Not Found: $file" >&2
    fi
  done < <(find "$OUTPUT_DIR" -type f)

  sort -u "${MSYS_INSTALLERS_FILE}.tmp" "${MSYS_INSTALLERS_FILE}" -o "${MSYS_INSTALLERS_FILE}"
  rm -f "${MSYS_INSTALLERS_FILE}.tmp"
}

sbom_create_msys2_package_list() {
  >${MSYS_PACKAGES_FILE}

  local file_count
  file_count=$(wc -l < "${MSYS_INSTALLERS_FILE}")
  echo "Looking up package ownership for $file_count files..."

  # Batch all file ownership lookups into a single xargs+pacman call
  # instead of invoking pacman -Qo per file
  # Output format: "/path/to/file is owned by package_name version"
  local ownership_file="$BUILD_DIR/_sbom_ownership.txt" status
  # Project and fetched dependency headers do not belong to MSYS packages.
  # pacman still prints the owned files, but returns 1 for unowned files
  # (which xargs reports as 123). Preserve that useful output with pipefail.
  if xargs -r -d '\n' -a "${MSYS_INSTALLERS_FILE}" \
      pacman -Qo --config "$PACMAN_CONFIG" > "$ownership_file" 2>/dev/null; then
    status=0
  else
    status=$?
  fi
  if [[ $status != 0 && $status != 123 ]]; then
    return "$status"
  fi
  awk '{ print $5","$6 }' "$ownership_file" | sort -u > "${MSYS_PACKAGES_FILE}"
  if [[ ! -s "$MSYS_PACKAGES_FILE" ]]; then
    echo "No MSYS package ownership information found" >&2
    return 1
  fi

  echo "Found $(wc -l < "${MSYS_PACKAGES_FILE}") unique packages."
}

# Query all package metadata in a single pacman -Qi call and cache as TSV
sbom_cache_package_info() {
  local pkg_list
  pkg_list=$(cut -d, -f1 < "${MSYS_PACKAGES_FILE}" | tr '\n' ' ')

  echo "Querying metadata for $(wc -w <<< "$pkg_list") packages..."

  # Single batch pacman -Qi call, parsed into TSV: name\tversion\tlicense\tdescription\turl
  pacman -Qi --config "$PACMAN_CONFIG" $pkg_list 2>/dev/null | awk '
    /^Name[[:space:]]/ { sub(/^[^:]*:[[:space:]]*/, ""); name=$0 }
    /^Version[[:space:]]/ { sub(/^[^:]*:[[:space:]]*/, ""); version=$0 }
    /^Description[[:space:]]/ { sub(/^[^:]*:[[:space:]]*/, ""); desc=$0 }
    /^URL[[:space:]]/ { sub(/^[^:]*:[[:space:]]*/, ""); url=$0 }
    /^Licenses[[:space:]]/ { sub(/^[^:]*:[[:space:]]*/, ""); license=$0 }
    /^$/ { if(name) print name"\t"version"\t"license"\t"desc"\t"url; name="" }
    END { if(name) print name"\t"version"\t"license"\t"desc"\t"url }
  ' > "${SBOM_PKG_INFO_FILE}"

  echo "Cached metadata for $(wc -l < "${SBOM_PKG_INFO_FILE}") packages."
}

# MSYS2 owns package discovery; Ship owns SPDX/CSV formatting for all backends.
sbom_create_sbom() {
  if [ ! -x "${SHIP:-}" ]; then
    source "$SCRIPTS_DIR/ci/ship.sh"
    build_ship
  fi
  "$SHIP" sbom --msys "$SBOM_PKG_INFO_FILE" --external "$EXTERNAL_SBOM_FILE" \
      --source "$SOURCES_DIR" --output "$OUTPUT_DIR"
}

sbom() {
  local _t0 _t1
  # Generate pacman config with disabled signatures (once, cached)
  sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf > "$PACMAN_CONFIG"

  sbom_precondition_check

  _t0=$SECONDS
  sbom_scan_installed_files
  echo "  sbom_scan_installed_files: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_scan_headers
  echo "  sbom_scan_headers: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_installer_map
  echo "  sbom_create_installer_map: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_msys_installed_files
  echo "  sbom_create_msys_installed_files: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_msys2_package_list
  echo "  sbom_create_msys2_package_list: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_cache_package_info
  echo "  sbom_cache_package_info: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_sbom || return
  echo "  sbom_create_sbom: $(( SECONDS - _t0 ))s"

}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  sbom
fi
