# ship — release pipeline runner

`ship` runs a release pipeline described in a YAML file, structured like a
GitHub Actions workflow: `runs-on` selects a machine, `container` selects a
job image, *targets* name lists of *jobs*, and steps are shell commands
or legacy built-in actions (signing through
the signing service, release notes, GitHub releases, S3 uploads, the update
catalog and appcast). It knows nothing about a particular project: the
procedure is the pipeline file, and the machine-specific values are a
free-form configuration file the pipeline refers to as `${{ config.* }}`.

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

Runner definitions live in `tools/local/runners.yaml`; job images and build
inputs live in `tools/local/release.yaml`. Machine access
and VM management belong in a private config file, rather than the workflow or
runner. Add this to `ship.yaml`:

```yaml
include: ship.runners.yaml
```

Copy `tools/local/ship.runners.example.yaml` to `ship.runners.yaml` (git-ignored)
and adjust its settings:

```yaml
execution:
  windows:
    ssh: build@windows-builder
    docker: docker.exe
    start: ''
    stop: ''
```

`runners.yaml` defines runner types and reads these settings; `ship.runners.yaml`
contains your private machine details. Existing includes of the older private
filename `ship.environments.yaml` remain valid.

Use an SSH destination or alias with working key authentication, and Docker in
Windows-container mode on the Windows machine. Ship runs PowerShell and copies
files through SSH/SCP; it does not use a hypervisor's guest agent. Optional
`start`/`stop` commands run on the machine running Ship. `start` must return with
SSH ready. `stop` runs after output collection and container cleanup, including
on failure; leave it empty for a shared VM. Start and stop hooks belong to each
job's runner lifetime. The example config contains Incus and Proxmox
profiles. These are ordinary commands in config, not built-in providers.

For Docker on the machine running Ship itself, run Ship on Windows and set
`execution.windows.ssh: ''`, or select the example `local-windows` profile.
Linux/macOS users need an SSH-accessible Windows host. Microsoft documents
[Windows OpenSSH installation](https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_install_firstuse)
and [key authentication](https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_keymanagement).

Config `include` accepts a filename or a list, relative to the containing file.
Includes can be nested; cycles are rejected. The containing file overrides its
includes, then the selected profile is applied. Pipeline `runner-files` loads
runner definitions relative to the pipeline file. Legacy `environment-files`
remains supported for older workflows.

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
  signing_service_url: "${{ config.signing_service_url }}"   # for ship.secret() and `sign`

options:                   # the pipeline's own command line options
  commit: { short: t, help: "Commit or tag to check out" }        # ${{ inputs.commit }}
  ppa: { config: linux.ppa_increment, help: "PPA increment" }     # overrides that config key
  no-sign: { flag: true, config: windows.sign, value: false }     # --no-sign sets it to false

vars:                      # derived values, refreshed as outputs arrive: ${{ vars.name }}
  source: "${{ config.workspace_dir }}/source"
  version: "${{ fromJSON(needs.workspace.outputs.version) }}"
  deploy: "${{ config.workspace_dir }}/deploy"
  release_dir: "${{ vars.deploy }}/${{ vars.version.tag }}"

runners:                   # normally in an external runner-files YAML
  linux-builder:
    type: linux-container-host
    engine: "${{ config.container.engine }}"
    sync: "${{ config.container.sync }}"

targets:                   # what --target selects; --job/--skip-job narrow it
  linux: [check-config, workspace, changelogs, appimage, ppa, github]

jobs:
  check-config:
    steps:
      - uses: check-config # every config key this target reads; missing ones are reported
        with: { required: [signing_service_url, workspace_dir, container.engine, linux.image] }
  appimage:
    needs: [workspace]     # prerequisites are included even outside the selected target
    runs-on: linux-builder
    container:
      image: "${{ config.container.image_repository }}:${{ config.linux.image }}"
      platform: "${{ matrix.platform }}"             # Ship extension
      mounts:                                      # synchronized bind mounts
        "${{ vars.source }}": /workspace/source
        "${{ vars.release_dir }}": /workspace/deploy
      env: { WORKRAVE_ENV: local }
    if: "${{ host.os != 'windows' }}"
    strategy: { matrix: { platform: [linux/amd64, linux/aarch64] } }
    steps:
      - run: /workspace/scripts/ci/build.sh         # a shell command
        env: { CONF_APPIMAGE: "1" }
      - id: build
        run: echo "id=$(date +%Y%m%d)" >> "$SHIP_OUTPUT"
      - run: echo "Built ${{ steps.build.outputs.id }}"
      - run: git push origin main
        dry-run: echo                               # only printed with --dry-run
```

Strings use `${{ expression }}` interpolation. A whole expression preserves its
JSON type; text containing expressions remains a string. `if:` accepts a bare
expression, `${{ ... }}`, or a YAML boolean. Empty strings, `false`, zero and
`null` are false; nonempty strings, including `'false'`, are true. Step outputs
are strings: use `fromJSON()` when an output represents a boolean or a matrix.

Supported syntax includes property/index access, single-quoted strings (double
an embedded quote), numbers, booleans, `null`, parentheses, `!`, comparisons,
`&&` and `||`. Logical operators short-circuit and return their selected operand.
String comparisons ignore case. Supported functions are `contains`, `startsWith`,
`endsWith`, `format`, `join`, `fromJSON`, `toJSON`, `success`, `failure`,
`cancelled` and `always`. Unknown functions and malformed expressions are rejected
when loading the workflow, including expressions in unselected jobs.

Contexts include `env`, `inputs`, `matrix`, `steps.<id>.outputs`,
`steps.<id>.outcome`, `steps.<id>.conclusion`, `needs.<job>.outputs` and
`needs.<job>.result`. Ship extensions are `config` (machine configuration),
computed `vars`, `dry_run`, `host.os`, `item` for `foreach`, `ship.exe`,
`ship.workflow_dir` and `ship.targets`. The `inputs` context contains declared
pipeline options plus `--set key=value`; `options` remains an alias. Options with
a `config:` key and `--set config.key=value` override the machine configuration.
Ship's own CLI flags select targets/jobs, files and dry-run behavior; other flags
in `ship release --help` come from the pipeline's `options:`.

Ship-specific expression helpers are `ship.secret(name)` (from the signing
service, masked in logs and empty in a dry run), `ship.exists(path)`,
`ship.glob(pattern)`, `ship.sha256(path)`, `ship.today(format)`, `ship.msys(path)`
(`C:\a` → `/c/a`), `ship.lower(text)`, `ship.replace(text, old, new)` and
`ship.slice(value, start, end)`.

Legacy `{{ ... }}` Jinja templates, helpers and filters remain available for old
workflows. Their text-to-boolean behavior is preserved; new workflows should
use `${{ ... }}` consistently. Workrave's release workflow uses the new syntax.

### Dependencies, conditions and matrices

Selecting a target or `--job` also includes its recursive `needs` dependencies.
`--skip-job` records a skipped result; it does not remove that dependency from
the graph. Only direct dependencies appear in the `needs` context. By default,
a job runs only if all its dependencies succeeded. Failed or skipped dependencies
skip their dependents, while independent jobs can still run.

A step runs only if earlier steps in its job succeeded, unless its condition
contains a status function. Use `failure()` for recovery or `always()` for
cleanup. At job level, `failure()` includes failed ancestors, even when an
intermediate dependency was skipped. To allow intentionally skipped dependencies
while still blocking failures, use `if: ${{ !failure() && !cancelled() }}`.
Recovery does not erase the original failure: the release still exits nonzero.
Ctrl-C stops scheduling new work; container cleanup and output collection still
run. `always()` does not override this interruption policy.

Job `if` runs before matrix expansion and cannot refer to `matrix`. A matrix can
be a mapping of array-valued axes or a whole `${{ fromJSON(...) }}` expression.
Axes can also be expressions, for example:

```yaml
strategy:
  fail-fast: false
  matrix:
    ui: [Gtk3, Qt]
    configuration: ${{ fromJSON(inputs.debug && '["Release","Debug"]' || '["Release"]') }}
```

Matrix `exclude` removes matching combinations; `include` adds compatible values
or appends new combinations. `fail-fast` defaults to true and stops the remaining
matrix entries after a failure. Jobs and matrix entries currently run serially;
`max-parallel` accepts a positive upper bound but does not enable concurrency.
Legacy job `always: true` only affects target selection; use a status function
in `if:` to control failure behavior.

### Step and job outputs

Give a step an `id` and write `name=value` lines to `$SHIP_OUTPUT` (PowerShell:
`$env:SHIP_OUTPUT`). Subsequent steps in that job read
`${{ steps.<id>.outputs.<name> }}`. Outputs are strings, so compare a boolean
flag with `'true'` or parse it with `fromJSON()`. An output is optional unless the workflow
itself checks for it. IDs must be unique within a job.

To pass a value between jobs, declare a job output explicitly:

```yaml
jobs:
  prepare:
    outputs:
      version: "${{ steps.version.outputs.value }}"
    steps:
      - id: version
        run: echo "value=1.12" >> "$SHIP_OUTPUT"
  build:
    needs: prepare
    steps:
      - run: echo "Building ${{ needs.prepare.outputs.version }}"
```

Only direct dependencies appear in `needs`; step outputs never leak into
another job or matrix entry. Matrix job outputs merge in completion order,
so use distinct output names when each matrix entry must be retained.

Legacy step `outputs: [version.tag, ...]` remains supported during migration.
It retains the global dotted names, boolean conversion and strict declared
output checks. It cannot be combined with `id`. Steps with `id` also cannot
use Ship's `foreach`: use separate steps or a matrix for scoped outputs.

### Environment files and run defaults

Ship provides fresh files for each shell step:

| Primary variable | Alias | Effect |
| --- | --- | --- |
| `SHIP_OUTPUT` | `GITHUB_OUTPUT` | Set outputs of the current step. |
| `SHIP_ENV` | `GITHUB_ENV` | Set environment variables for subsequent steps in this job. |
| `SHIP_PATH` | `GITHUB_PATH` | Prepend directories to PATH for subsequent steps in this job, one per line. |

Each alias points to the same file as its Ship name. Environment changes do
not affect the writing step or other jobs. Step `env` overrides job values;
job values override workflow values. Environment-file updates replace the
inherited values for later steps, which may still override them using `env`.
The `env` template context reflects these values. PATH entries are applied
on the execution machine, including inside remote containers.

Output and environment files accept `name=value` and multiline values:

```sh
{
    echo 'notes<<END_NOTES'
    printf '%s\n' "$notes"
    echo 'END_NOTES'
} >> "$SHIP_OUTPUT"
```

Choose a delimiter that does not occur on a line by itself in the value.
Values preserve whitespace and remain strings, including empty values.
Use UTF-8 when writing files; with Windows PowerShell 5.1 use
`Out-File -Encoding utf8 -Append`. `SHIP_ENV` cannot replace reserved
`SHIP_*`, `GITHUB_*` or `RUNNER_*` variables.

Run settings can be given at workflow, job or step level:

```yaml
defaults:
  run:
    shell: bash
    working-directory: /workspace/source
```

A job's `defaults.run` overrides workflow defaults. A step's `shell` and
`working-directory` override both. Legacy `cwd` is still accepted.
Supported shells are `bash`, `sh`, `powershell` and `pwsh`; Linux containers
currently support `bash` and `sh`. The selected shell must be installed.
Explicit Bash uses error exit and `pipefail`; `sh` uses error exit.
PowerShell uses stop-on-error and propagates the last native exit code.
The environment's `init` script must match the shell used by its steps.

Command files work with host, MSYS2, Linux container and Windows container
steps. Linux command files are collected through the container engine, so
remote engines do not need controller temporary directories mounted.

### Runners and job containers

`runs-on` accepts a single label from `runners` or an external `runner-files`
file. `host` means the machine running Ship and is the default. Ship binds
labels to your configured machines; it does not provision GitHub-hosted runners
or implement label arrays/groups.

Runner types are `host`, `linux-container-host` (Docker or Podman, including a
remote Podman connection), and `windows-container-host` (local Windows Docker or
SSH). Container-host runners require a job `container`. The Windows runner owns
`ssh`, `engine`, `start` and `stop`; Linux runners own `engine`, `sync` and
`remote-dir`. Existing machine configuration and profile keys are unchanged.

A job `container` accepts an image string or a mapping with `image`, `env`,
`volumes` and `options`. Options are a shell-style argument string, split into
arguments without executing a shell. Lifecycle flags and entrypoint overrides
are rejected. Volumes are passed to the engine unchanged: bind sources are paths
on the container host. Ship's `mounts` mapping instead synchronizes controller
paths when remote Podman needs it. `platform` is also a Ship extension.

Each job/matrix entry gets one container. Files installed or generated by a
step remain available to later steps; each step starts a fresh shell. Use
`SHIP_ENV`/`SHIP_PATH` for shell environment changes. Linux and Windows containers
are removed after the job, including failures and graceful interruption. Linux
mounts are synced back and Windows `collect` outputs are copied back on failure.

Windows containers extend GitHub's Linux job-container model. Their optional
`image-context`, `copy`, `collect` and `init` fields retain Ship's native Windows
build support. Image, volumes and transfers belong to the job, while VM access
stays in the runner configuration. Put shell defaults in `defaults.run`.

Legacy `runs-in`/`environments` still work, including the old Linux container per
step behavior. They cannot be combined with `runs-on` or `container`. Step-level
mounts, platform and engine options cannot be used with a shared job container.
Legacy built-in `uses` steps execute on the controller and are rejected inside
new job containers; call `ship action` through `run` there.

### Portable action commands

All built-in operations are available as `ship action <name>` independently of
Ship workflows. Parameters use the same schema as the old `with` mapping:
`--params file.yaml`, `--params -` for stdin, or `--params-env VARIABLE` for a
YAML/JSON value in the environment. Install Ship on the execution machine (or
in its image) before using the command in a GitHub Actions `run` step.

For example, the same step can be used in Ship or GitHub Actions:

```yaml
- name: Generate release notes
  run: ship action newsgen --params-env SHIP_ACTION_PARAMS
  env:
    SHIP_ACTION_PARAMS: |
      input: "changes.yaml"
      template: "github"
      output: "release-notes.md"
      release: ${{ toJSON(inputs.version) }}
      single: true
```

`toJSON` quotes dynamic parameter values so embedded quotes or newlines remain
data. Parameters are not interpolated again by the action command. `--params-env`
also keeps credentials out of command-line arguments. Signing accepts `--url`
or `SIGNING_SERVICE_URL`. Other operations receive every service parameter
explicitly and do not load the machine config; `check-config` is the exception
and accepts `--config`/`--profile`.

Commands fail with a nonzero exit status on errors. `--dry-run` still generates
local release notes and runs checks, but describes signing/publishing instead
of performing them. Ordinary `run` steps do not inherit this flag automatically;
the release workflow passes it explicitly. Unlike the older standalone CI
commands that retain historical error handling, `ship action` never suppresses
an operation's error.

Workrave's release uses these commands for release notes, signing, publishing,
appcasts, catalogs and Homebrew linkage checks. `uses: check-config` remains an
early controller check, including during `pipeline show`; the equivalent
standalone command is available for other workflows. All old built-in `uses`
forms remain supported outside new job containers.

### Compatibility scope

Ship follows GitHub Actions semantics for the expression subset above, scoped
outputs, dependency conditions, matrices, supported run settings and command
files. `SHIP_*` names are primary; `GITHUB_*` names are aliases for command files.

Ship-specific target selection, runner bindings, machine configuration,
computed variables, helpers, `foreach`, native Windows containers and transfer
fields remain extensions. Scheduling is serial. Marketplace actions, event
triggers, service containers, runner label arrays/groups, object-filter syntax,
collection identity comparisons and `hashFiles` are not supported. Unsupported
YAML fields and expression syntax produce errors rather than being silently
ignored. This is a compatible subset, not a full GitHub Actions runner.

See GitHub's [expressions reference](https://docs.github.com/en/actions/reference/workflows-and-actions/expressions),
[job dependency documentation](https://docs.github.com/en/actions/how-tos/write-workflows/choose-what-workflows-do/use-jobs),
[job containers](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#jobsjob_idcontainer) and [environment-file documentation](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-commands#environment-files)
for the shared behavior and file formats.

Builtin actions: `check-config`, `newsgen`, `sign` (cosign, ed25519,
authenticode, catalog), `github-release`, `s3-upload`, `catalog`, `appcast`,
`check-no-homebrew-links` — see `src/services/actions/` for their
parameters. Actions take everything they need as parameters (repository,
token, bucket, ...); only `check-config` reads the configuration. Their side effects are printed instead of performed in a dry
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
