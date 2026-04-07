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
SBOM_PACKAGES_FILE="$BUILD_DIR/sbom-packages.jsonl"
SBOM_FILE="$OUTPUT_DIR/sbom.spdx.json"
SBOM_CSV_FILE="$OUTPUT_DIR/sbom.csv"

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
  if [ -f ${RUNTIME32_INSTALLERS_FILE} ]; then
    cat ${RUNTIME32_INSTALLERS_FILE} | sed 's|C:/msys64||' >>${INSTALLERS_FILE}
  fi
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
    packages=""
    pacman_exit_code=0
    packages=$(pacman -Qo --config <(sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf) "$filename" 2>/dev/null) || pacman_exit_code=$?

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

sbom_collect_packages() {
  >${SBOM_PACKAGES_FILE}

  while IFS=, read -r package_name package_version; do
    pacman_info=""
    pacman_exit_code=0
    pacman_info=$(pacman --config <(sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf) -Qi "$package_name" 2>/dev/null) || pacman_exit_code=$?

    if [ $pacman_exit_code -ne 0 ]; then
      echo "Pacman -Qi failed for $package_name with exit code $pacman_exit_code, skipping."
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

    sbom_append_package "$package_name" "$package_version" "$license" "$description" "$url"

  done <"$MSYS_PACKAGES_FILE"

  if [ -f "$EXTERNAL_SBOM_FILE" ]; then
    while IFS=, read -r package_name package_version license description url; do
      sbom_append_package "$package_name" "$package_version" "$license" "$description" "$url"
    done <"$EXTERNAL_SBOM_FILE"
  fi
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

  while IFS=, read -r package_name package_version; do
    pacman_info=""
    pacman_exit_code=0
    pacman_info=$(pacman --config <(sed 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf) -Qi "$package_name" 2>/dev/null) || pacman_exit_code=$?

    if [ $pacman_exit_code -ne 0 ] || [[ -z "$pacman_info" ]]; then
      continue
    fi

    installed_version=$(echo "$pacman_info" | awk -F ': ' '/^Version/ {print $2}')
    if [[ "$installed_version" != "$package_version" ]]; then
      continue
    fi

    url=$(echo "$pacman_info" | awk -F ': ' '/^URL/ {print $2}')
    license=$(echo "$pacman_info" | awk -F ': ' '/^Licenses/ {print $2}')
    description=$(echo "$pacman_info" | awk -F ': ' '/^Description/ {print $2}')

    echo "$package_name,$package_version,$license,$description,$url" >> "$SBOM_CSV_FILE"
  done < "$MSYS_PACKAGES_FILE"

  if [ -f "$EXTERNAL_SBOM_FILE" ]; then
    cat "$EXTERNAL_SBOM_FILE" >> "$SBOM_CSV_FILE"
  fi

  echo "SBOM CSV generated: $SBOM_CSV_FILE"
}

sbom() {
  sbom_precondition_check
  sbom_scan_installed_files
  sbom_scan_headers
  sbom_create_installer_map
  sbom_create_msys_installed_files
  sbom_create_msys2_package_list
  sbom_collect_packages
  sbom_create_sbom
  sbom_create_csv
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  sbom
fi
