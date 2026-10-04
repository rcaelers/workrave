# ship — release pipeline runner

`ship` runs a release pipeline described in a YAML file, structured like a
GitHub Actions workflow: *environments* (the host, an MSYS2 shell, build
containers — locally or on a remote podman host), *targets* naming lists of
*jobs*, and steps that are shell commands or builtin actions (signing through
the signing service, release notes, GitHub releases, S3 uploads, the update
catalog and appcast). It knows nothing about a particular project: the
procedure is the pipeline file, and the machine-specific values are a
free-form configuration file the pipeline refers to as `{{ config.* }}`.

Workrave's pipeline is `tools/local/release.yaml`, its configuration
`tools/local/ship.yaml` (git-ignored, see `tools/local/ship.example.yaml`),
and `tools/local/ship` builds this crate if needed and runs it with both:

```
tools/local/ship pipeline show --target linux            # what would run
tools/local/ship release --commit <sha> --version v1_12_0_alpha_1 --dry-run
tools/local/ship release --target linux --job ppa --version v1_12_0_alpha_1
tools/local/ship release --target publish-appcast
```

Without the wrapper: `--pipeline`/`$SHIP_PIPELINE` (default `release.yaml` in
the current directory), `--config`/`$SHIP_CONFIG` (default
`~/.config/ship/ship.yaml`), `--profile` to apply a profile of the
configuration. The scripts that run *inside* the build containers and in CI
(`tools/ci/build.sh`, `tools/local/ppa.sh`, ...) are bash, because they must
run uncompiled on every platform and architecture the images cover.

Each target starts with a `check-config` job that lists the configuration
keys it needs and reports the missing ones (also in `pipeline show`). The
engine itself needs only what the pipeline's `settings:` gives it (the
signing service URL) and what each container environment declares
(`engine`, `sync`, `remote-dir`).

## Windows container builds

`tools/local/ship release --target windows` builds both Windows editions in
process-isolated Docker containers: Workrave (GTK) and WorkraveNext (Qt).
GTK uses MSYS2 CLANG64; Qt uses native
llvm-mingw and the cached Conan SDK, without MSYS2. Configure, compile, test,
install, SBOM, portable archive, installer, symbols and artifact recording are
separate steps in `tools/local/release.yaml` and in the build report. Each
edition/configuration keeps one container for the entire job.

Workrave keeps the existing installer identity, including upgrades from 32-bit
1.10 installations. WorkraveNext installs alongside it with separate shortcuts
and an independent uninstaller. Both use the same settings and state, with one
running instance and one shared startup selection. WorkraveNext names the Qt
package, executable and shortcuts; the application itself displays Workrave.

Execution definitions live in `tools/local/environments.yaml`. Machine access
and VM management belong in a private config file, rather than the workflow or
runner. Add this to `ship.yaml`:

```yaml
include: ship.environments.yaml
```

Then create `ship.environments.yaml` (git-ignored):

```yaml
execution:
  windows:
    ssh: build@windows-builder
    docker: docker.exe
    start: ''
    stop: ''
```

Use an SSH destination or alias with working key authentication, and Docker in
Windows-container mode on the Windows machine. Ship runs PowerShell and copies
files through SSH/SCP; it does not use a hypervisor's guest agent. Optional
`start`/`stop` commands run on the machine running Ship. `start` must return with
SSH ready. `stop` runs after output collection and container cleanup, including
on failure; leave it empty for a shared VM. Start and stop hooks belong to each
job's environment lifetime. The example config contains Incus and Proxmox
profiles. These are ordinary commands in config, not built-in providers.

For Docker on the machine running Ship itself, run Ship on Windows and set
`execution.windows.ssh: ''`, or select the example `local-windows` profile.
Linux/macOS users need an SSH-accessible Windows host. Microsoft documents
[Windows OpenSSH installation](https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_install_firstuse)
and [key authentication](https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_keymanagement).

Config `include` accepts a filename or a list, relative to the containing file.
Includes can be nested; cycles are rejected. The containing file overrides its
includes, then the selected profile is applied. Pipeline `environment-files`
loads environment definitions relative to the pipeline file.

Both Windows image definitions live in `workrave-build-containers`. Set
`windows.containers_dir` to that checkout. Ship rebuilds an image only when
its context changes. Source and script inputs are copied into the container
layer; declared output directories and symbols are collected after the job,
also when a step fails. Compiler objects remain local to the job's container.
Cargo and SDK volumes persist between jobs.

The `win-qt-sdk` job builds locked Conan dependencies on Linux in the
`workrave-build:llvm-mingw` image. Set `windows.dependencies_dir`,
`windows.dependencies_image` and `windows.conan_cache` for this job. An
unchanged dependency configuration reuses the exported SDK. Windows initializes
that SDK and its native `windeployqt` in `windows.qt_sdk_cache`.

The Linux job exposes cache lookup, Conan configuration, dependency compilation,
the Qt smoke build, PDB validation, SDK export, archive caching and staging as
separate steps. Package relocation lives in `workrave-dependencies/conan/export-sdk.py`,
with regular CMake files under `conan/sdk`. The cache key includes dependency
inputs, this release workflow and the compiler/Conan versions. A cache hit skips
all preparation steps and stages the existing archive.

To create or restore both dependency volumes without building Workrave:

```sh
tools/local/ship release --target windows-dependencies
```

This target needs `scripts_dir`, `workspace_dir`, Linux container settings and
the Windows connection settings. It needs no release checkout, version,
signing service or upload configuration. Missing volumes are created
automatically. The locked dependencies, Qt smoke build and matching PDBs are
checked before the SDK is exported. Repeated runs reuse both caches.

The Linux volume defaults to `workrave-conan`; the Windows volume defaults to
`workrave-qt-sdk`. Separate recovery caches can be selected with:

```sh
tools/local/ship release --target windows-dependencies \
  --set config.windows.conan_cache=workrave-conan-recovery \
  --set config.windows.qt_sdk_cache=workrave-qt-sdk-recovery
```

Keep the Conan checkout, committed `windows.lock` and pinned compiler image to
rebuild the same dependency versions. Source checksums are verified and the
lockfile is not updated. This does not promise byte-identical binaries.

## SBOM generation

`ship sbom` is the shared SPDX/CSV writer. `tools/local/sbom.sh` discovers
MSYS2 packages; the Conan build supplies its SDK manifest. Both feed the same
writer alongside CMake's FetchContent inventory. The duplicate Python Workrave
build, remote runner and SBOM helpers have been removed. The SDK exporter and
symbol processing remain specialized helpers.

For a standalone Windows build with SBOM enabled, build Ship first and set
`SHIP_EXECUTABLE` if it is not on PATH. Both editions generate the inventory
explicitly after installing:

```sh
cmake --install <build-directory> --component SBOM
```

The release workflow includes this as a named step before packaging.

## The pipeline file

```yaml
settings:
  signing_service_url: "{{ config.signing_service_url }}"   # for secret() and `sign`

options:                   # the pipeline's own command line options
  commit: { short: t, help: "Commit or tag to check out" }        # {{ options.commit }}
  ppa: { config: linux.ppa_increment, help: "PPA increment" }     # overrides that config key
  no-sign: { flag: true, config: windows.sign, value: false }     # --no-sign sets it to false

vars:                      # derived values, computed once per job: {{ vars.name }}
  source: "{{ config.workspace_dir }}/source"
  release_dir: "{{ vars.deploy }}/{{ version.tag }}"

environments:              # where a job's `run:` steps execute
  ubuntu:
    type: container        # host | container | msys2
    engine: "{{ config.container.engine }}"
    sync: "{{ config.container.sync }}"   # mirror mounts to a remote podman host
    image: "{{ config.container.image_repository }}:{{ config.linux.image }}"
    mounts: { "{{ vars.source }}": /workspace/source }
    env: { WORKRAVE_ENV: local }

targets:                   # what --target selects; --job/--skip-job narrow it
  linux: [check-config, workspace, changelogs, appimage, ppa, github]

jobs:
  check-config:
    steps:
      - uses: check-config # every config key this target reads; missing ones are reported
        with: { required: [signing_service_url, workspace_dir, container.engine, linux.image] }
  appimage:
    needs: [workspace]     # ordering; jobs outside the target are ignored
    runs-in: ubuntu
    if: "{{ host.os != 'windows' }}"
    strategy: { matrix: { platform: [linux/amd64, linux/aarch64] } }
    steps:
      - run: /workspace/scripts/ci/build.sh         # a shell command
        platform: "{{ matrix.platform }}"
        env: { CONF_APPIMAGE: "1" }
        mounts: { "{{ vars.release_dir }}": /workspace/deploy/ }
        options: [--cap-add, SYS_ADMIN]
      - uses: newsgen                               # a builtin action
        foreach: "{{ config.linux.ppa_series }}"    # once per {{ item }}
        with: { template: debian-changelog, series: "{{ item }}", output: ... }
      - run: echo "build.id=$(date +%Y%m%d)" >> "$SHIP_OUTPUT"
        outputs: [build.id]                         # {{ build.id }} from here on
      - run: git push origin main
        dry-run: echo                               # only printed with --dry-run
```

Every string is a Jinja template (minijinja), rendered when the step runs.
Context: `config` (the machine configuration, with the overrides from
options that have a `config:` key and from `--set config.key=value`),
`options` (the other pipeline options as given, plus `--set key=value`
pairs), `dry_run`,
`host.os`, `ship.exe`, `env.*`, `vars.*`, `matrix.*`, `item`, plus whatever
earlier steps set (see below). `ship` itself only has `--target`, `--job`,
`--skip-job`, `--dry-run`, `--set` and the file-selecting options; everything
else in `ship release --help` comes from the pipeline's `options:`. `vars` and outputs that render to
`true`/`false` become booleans. Functions: `secret(name)` (from the signing service, masked in logs,
empty in a dry run), `exists(path)`, `glob(pattern)`, `today(format)`.
Filter: `msys` (`C:\a` → `/c/a`). `if:` is false for an empty, `false`, `0`
or `none` result.

A `run:` step can set values for later steps and jobs, like `$GITHUB_OUTPUT`:
it declares them in `outputs:` and writes `name=value` lines to the file in
`$SHIP_OUTPUT`. Dotted names nest, so the `workspace` job's
`echo "version.tag=$TAG" >> "$SHIP_OUTPUT"` is `{{ version.tag }}`
everywhere after it; `true`/`false` become booleans. A declared output that
is not written, or a written one that is not declared, is an error.
This works in host, MSYS2, Linux container and Windows container steps. Linux
container outputs are collected through the container engine, including when
it runs on a remote host.

Builtin actions: `check-config`, `newsgen`, `sign` (cosign, ed25519,
authenticode, catalog), `github-release`, `s3-upload`, `catalog`, `appcast`,
`check-no-homebrew-links` — see `src/cmd/release/actions/` for their
parameters. Actions take everything they need as parameters (repository,
token, bucket, ...); they do not read the configuration. Their side effects are printed instead of performed in a dry
run; `run:` steps execute unless marked `dry-run: echo`.

Ctrl-C stops the running command (its whole process group, so a container's
podman client stops the container too), removes the container if one is
still there, syncs mirrored directories back, and exits with an error; a
second Ctrl-C aborts immediately.

At the end of `ship release`, a build report on stderr lists each job and
step's status and elapsed wall-clock time, followed by the total time.
Matrix jobs and `foreach` iterations appear separately, including the matrix
or item values in their labels; conditional skips and empty loops are marked
`skipped`, with no execution time. The report
also appears after a failure or a graceful Ctrl-C, covering the work reached
before stopping. Job rows are subtotals: each includes its steps plus a job
overhead row for container preparation, syncing results back and bookkeeping.
A pipeline overhead row accounts for time outside the jobs. Add the step
and overhead rows to get the overall total; do not also add the job subtotals.
All durations retain millisecond precision, so rounding may cause a small
difference when adding the displayed values.
Dry runs produce a report marked `(dry run)`: shell steps with `dry-run: echo`
are marked `echoed`, and action timings reflect their dry-run behavior.
`pipeline show` does not produce a timing report.

A container job prepares its mounts once: with a remote podman
(`podman system connection`), the mounted directories are mirrored there with
rsync before the first step and synced back after the last, also on failure;
git-ignored paths are not uploaded, so the remote build tree persists between
runs.

The `ppa` and `deb` jobs iterate over `linux.ppa_series` in the pipeline, so
each Ubuntu series gets separate source- and binary-package timing entries.
Their `ppa.sh` and `pbuild.sh` helpers build only the series named by `DIST`;
packaging, signing and the rootless pbuilder setup stay in shell code.

### Workrave AppImage builds

The Workrave release pipeline builds separate `x86_64` and `aarch64`
AppImages. On an amd64 container host, the ARM64 build uses the amd64 image
`ubuntu-cross-aarch64` from `workrave-build-containers`; override its tag
with `linux.cross_image` if needed. The image tag stays the same as its
Ubuntu base advances. ARM64 hosts continue using the regular native image
for the ARM64 build.

The cross image supplies `CONF_TOOLCHAIN_FILE` and `CONF_TARGET_ARCH` to
`tools/ci/build.sh`. Compilation, RPC generation and AppImage compression
run natively; QEMU is limited to helpers that inspect target libraries.
Packaging extracts its tools without FUSE, so no extra container
capabilities are needed. The source revision must contain the matching
CMake support as well as the updated scripts.

AppImage build/output directories are separated by architecture, and the
deployed names are `workrave-linux-x86_64-<version>.AppImage` and
`workrave-linux-aarch64-<version>.AppImage`.

## Other commands

| command | what it does |
|---|---|
| `ship pipeline show` | Print the jobs and every rendered command line of a target without running anything. |
| `ship sign cosign\|ed25519\|authenticode\|catalog` | Sign through the signing service. Also the CMake signing hook (`WITH_SIGN_TOOL`). |
| `ship secret <name>` | Fetch a secret from the signing service (debugging). |
| `ship newsgen`, `ship catalog`, `ship appcast` | Release notes, artifact catalog and appcast generation (also used by CI). |

## Development

```
cargo test
cargo clippy
```
