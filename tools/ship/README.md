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
(Host and msys2 steps only.)

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

A container job prepares its mounts once: with a remote podman
(`podman system connection`), the mounted directories are mirrored there with
rsync before the first step and synced back after the last, also on failure;
git-ignored paths are not uploaded, so the remote build tree persists between
runs.

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
