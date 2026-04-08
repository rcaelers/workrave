#!/bin/bash
shopt -s extglob

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
SBOM_PACKAGES_FILE="$BUILD_DIR/sbom-packages.jsonl"
SBOM_FILE="$OUTPUT_DIR/sbom.spdx.json"
SBOM_CSV_FILE="$OUTPUT_DIR/sbom.csv"
PACMAN_CONFIG="$BUILD_DIR/_sbom_pacman.conf"
SBOM_PKG_INFO_FILE="$BUILD_DIR/_sbom_pkginfo.tsv"

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
  return $exit_code
}

normalize_whitespace() {
  printf '%s' "$1" | tr '\r\n\t' '   ' | sed 's/[[:space:]]\+/ /g; s/^ //; s/ $//'
}

spdx_safe_id() {
  printf '%s' "$1" | sed 's/[^A-Za-z0-9.-]/-/g; s/-\{2,\}/-/g; s/^-//; s/-$//'
}

spdx_package_id() {
  printf 'SPDXRef-Package-%s' "$(spdx_safe_id "$1-$2")"
}

spdx_normalize_license() {
  local raw normalized comment

  raw=$(normalize_whitespace "$1")
  comment=""

  if [[ -z "$raw" ]]; then
    printf 'NOASSERTION\t\n'
    return 0
  fi

  normalized="$raw"
  normalized="${normalized//documentation:spdx:/}"
  normalized="${normalized//spdx:/}"
  normalized=$(printf '%s' "$normalized" | sed 's#/# OR #g; s/[[:space:]]\+/ /g; s/^ //; s/ $//')

  case "$normalized" in
  GPL-2.0)
    normalized="GPL-2.0-only"
    ;;
  GPL-3.0)
    normalized="GPL-3.0-only"
    ;;
  LGPL-2.1)
    normalized="LGPL-2.1-only"
    ;;
  LGPL-3.0)
    normalized="LGPL-3.0-only"
    ;;
  esac

  if [[ "$raw" == "Unknown" || "$raw" == "custom" || "$raw" == "LGPL" || "$raw" == custom:* ]]; then
    printf 'NOASSERTION\t%s\n' "$raw"
    return 0
  fi

  if [[ "$normalized" =~ (^|[[:space:]])(custom|Unknown)([[:space:]]|$) ]]; then
    printf 'NOASSERTION\t%s\n' "$raw"
    return 0
  fi

  if [[ "$normalized" =~ [:,] ]]; then
    printf 'NOASSERTION\t%s\n' "$raw"
    return 0
  fi

  if [[ ! "$normalized" =~ ^[A-Za-z0-9.+()-]+([[:space:]]+(AND|OR|WITH)[[:space:]]+[A-Za-z0-9.+()-]+)*$ ]]; then
    printf 'NOASSERTION\t%s\n' "$raw"
    return 0
  fi

  if [[ "$normalized" != "$raw" ]]; then
    comment="$raw"
  fi

  printf '%s\t%s\n' "$normalized" "$comment"
}

sbom_append_package() {
  local name version license description url license_declared license_comment spdx_id

  name=$(normalize_whitespace "$1")
  version=$(normalize_whitespace "$2")
  license=$(normalize_whitespace "$3")
  description=$(normalize_whitespace "$4")
  url=$(normalize_whitespace "$5")

  if [[ -z "$name" || -z "$version" ]]; then
    return 0
  fi

  IFS=$'\t' read -r license_declared license_comment < <(spdx_normalize_license "$license")
  spdx_id=$(spdx_package_id "$name" "$version")

  jq -cn \
    --arg spdxid "$spdx_id" \
    --arg name "$name" \
    --arg version "$version" \
    --arg description "$description" \
    --arg homepage "$url" \
    --arg licenseDeclared "$license_declared" \
    --arg licenseComment "$license_comment" \
    '
      {
        SPDXID: $spdxid,
        name: $name,
        versionInfo: $version,
        downloadLocation: "NOASSERTION",
        filesAnalyzed: false,
        licenseConcluded: "NOASSERTION",
        licenseDeclared: $licenseDeclared
      }
      + (if $description != "" then {summary: $description} else {} end)
      + (if $homepage != "" then {homepage: $homepage} else {} end)
      + (if $licenseComment != "" then {licenseComments: $licenseComment} else {} end)
    ' >>"$SBOM_PACKAGES_FILE"

  printf '\n' >>"$SBOM_PACKAGES_FILE"
}

sbom_workrave_version() {
  local version

  version=$(sed -n 's/^[[:space:]]*set[[:space:]]*(WORKRAVE_VERSION[[:space:]]*"\([^"]*\)".*/\1/p' "${WORKSPACE_DIR:-$SOURCES_DIR}/CMakeLists.txt" | head -n 1)
  if [[ -n "$version" ]]; then
    printf '%s' "$version"
  else
    printf 'UNKNOWN'
  fi
}

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
  xargs -r -a "${MSYS_INSTALLERS_FILE}" \
    pacman -Qo --config "$PACMAN_CONFIG" 2>/dev/null | \
    awk '{ print $5","$6 }' | \
    sort -u > "${MSYS_PACKAGES_FILE}"

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

sbom_collect_packages() {
  local _merged="$BUILD_DIR/_sbom_merged.tsv"

  # Merge MSYS2 package info + external SBOM into a single TSV (6th field = source)
  awk -F'\t' '{print $0"\tmsys2"}' "${SBOM_PKG_INFO_FILE}" > "$_merged"
  if [ -f "$EXTERNAL_SBOM_FILE" ]; then
    awk -F, '{OFS="\t"; print $1,$2,$3,$4,$5,"external"}' "$EXTERNAL_SBOM_FILE" >> "$_merged"
  fi

  # Single awk call generates all JSONL (replaces ~60+ individual jq invocations)
  awk -F'\t' '
    function json_escape(s) {
      gsub(/\\/, "\\\\", s)
      gsub(/"/, "\\\"", s)
      gsub(/\t/, "\\t", s)
      return s
    }
    function spdx_safe_id(s) {
      gsub(/[^A-Za-z0-9.-]/, "-", s)
      gsub(/-{2,}/, "-", s)
      sub(/^-/, "", s)
      sub(/-$/, "", s)
      return s
    }
    function normalize_license(raw,   norm, comment, result) {
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", raw)
      if (raw == "") return "NOASSERTION\t"
      norm = raw
      gsub(/documentation:spdx:/, "", norm)
      gsub(/spdx:/, "", norm)
      gsub(/\//, " OR ", norm)
      gsub(/[[:space:]]+/, " ", norm)
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", norm)
      if (norm == "GPL-2.0") norm = "GPL-2.0-only"
      if (norm == "GPL-3.0") norm = "GPL-3.0-only"
      if (norm == "LGPL-2.1") norm = "LGPL-2.1-only"
      if (norm == "LGPL-3.0") norm = "LGPL-3.0-only"
      if (raw == "Unknown" || raw == "custom" || raw == "LGPL" || raw ~ /^custom:/) return "NOASSERTION\t" raw
      if (norm ~ /(^|[[:space:]])(custom|Unknown)([[:space:]]|$)/) return "NOASSERTION\t" raw
      if (norm ~ /[:,]/) return "NOASSERTION\t" raw
      if (norm !~ /^[A-Za-z0-9.+()-]+([[:space:]]+(AND|OR|WITH)[[:space:]]+[A-Za-z0-9.+()-]+)*$/) return "NOASSERTION\t" raw
      comment = ""
      if (norm != raw) comment = raw
      return norm "\t" comment
    }
    {
      name = $1; version = $2; license = $3; desc = $4; url = $5; source = $6
      if (name == "" || version == "") next

      if (source == "msys2")
        dl = "https://packages.msys2.org/packages/" name
      else if (url != "")
        dl = url
      else
        dl = "NOASSERTION"

      spdxid = "SPDXRef-Package-" spdx_safe_id(name "-" version)
      lic = normalize_license(license)
      split(lic, lp, "\t")
      ld = lp[1]; lc = lp[2]

      printf "{\"SPDXID\":\"%s\",\"name\":\"%s\",\"versionInfo\":\"%s\",\"downloadLocation\":\"%s\",\"filesAnalyzed\":false,\"licenseConcluded\":\"NOASSERTION\",\"licenseDeclared\":\"%s\"", json_escape(spdxid), json_escape(name), json_escape(version), json_escape(dl), json_escape(ld)
      if (desc != "") printf ",\"summary\":\"%s\"", json_escape(desc)
      if (url != "") printf ",\"homepage\":\"%s\"", json_escape(url)
      if (lc != "") printf ",\"licenseComments\":\"%s\"", json_escape(lc)
      printf "}\n"
    }
  ' "$_merged" > "$SBOM_PACKAGES_FILE"

  rm -f "$_merged"
}

sbom_create_sbom() {
  local created_at workrave_version document_namespace

  created_at=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
  workrave_version=$(sbom_workrave_version)
  document_namespace="https://workrave.org/spdxdocs/workrave-${workrave_version}-$(date -u +"%Y%m%dT%H%M%SZ")"

  jq -s \
    --arg created "$created_at" \
    --arg namespace "$document_namespace" \
    --arg workraveVersion "$workrave_version" \
    '
      def rootPackage:
        {
          SPDXID: "SPDXRef-Package-Workrave",
          name: "Workrave",
          versionInfo: $workraveVersion,
          supplier: "Organization: Workrave",
          downloadLocation: "NOASSERTION",
          filesAnalyzed: false,
          homepage: "https://workrave.org",
          licenseConcluded: "NOASSERTION",
          licenseDeclared: "NOASSERTION",
          summary: "Break reminder and RSI prevention tool",
          primaryPackagePurpose: "APPLICATION"
        };
      def dependsOn($target):
        {
          spdxElementId: "SPDXRef-Package-Workrave",
          relationshipType: "DEPENDS_ON",
          relatedSpdxElement: $target
        };
      (unique_by(.SPDXID)) as $packages
      | {
          spdxVersion: "SPDX-2.3",
          dataLicense: "CC0-1.0",
          SPDXID: "SPDXRef-DOCUMENT",
          name: "workrave-sbom",
          documentNamespace: $namespace,
          creationInfo: {
            created: $created,
            creators: [
              "Tool: workrave tools/local/sbom.sh"
            ]
          },
          documentDescribes: ["SPDXRef-Package-Workrave"],
          packages: ([rootPackage] + $packages),
          relationships: (
            [
              {
                spdxElementId: "SPDXRef-DOCUMENT",
                relationshipType: "DESCRIBES",
                relatedSpdxElement: "SPDXRef-Package-Workrave"
              }
            ]
            + ($packages | map(dependsOn(.SPDXID)))
          )
        }
    ' "$SBOM_PACKAGES_FILE" >"$SBOM_FILE"

  echo "SBOM generated: $SBOM_FILE"
}

sbom_create_csv() {
  echo "Package Name,Version,License,Description,URL" > "$SBOM_CSV_FILE"

  # Read from cached package info (no additional pacman calls)
  awk -F'\t' '{ print $1","$2","$3","$4","$5 }' "${SBOM_PKG_INFO_FILE}" >> "$SBOM_CSV_FILE"

  if [ -f "$EXTERNAL_SBOM_FILE" ]; then
    cat "$EXTERNAL_SBOM_FILE" >> "$SBOM_CSV_FILE"
  fi

  echo "SBOM CSV generated: $SBOM_CSV_FILE"
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
  sbom_collect_packages
  echo "  sbom_collect_packages: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_sbom
  echo "  sbom_create_sbom: $(( SECONDS - _t0 ))s"

  _t0=$SECONDS
  sbom_create_csv
  echo "  sbom_create_csv: $(( SECONDS - _t0 ))s"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  sbom
fi
