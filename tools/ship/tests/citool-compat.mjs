#!/usr/bin/env node

// Black-box compatibility checks for the Citool -> Ship migration.
//
// Prerequisites:
//   (cd tools/citool && npm ci && npm run build)
//   node tools/ship/tests/citool-compat.mjs
//
// The test server implements only the S3 operations used by the tools.  It
// deliberately records objects, rather than relying on an S3 implementation,
// so the test remains hermetic and can show the final storage state directly.

import assert from 'node:assert/strict';
import { execFileSync, spawnSync } from 'node:child_process';
import { mkdtemp, mkdir, readFile, rm, writeFile } from 'node:fs/promises';
import http from 'node:http';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const shipDir = path.resolve(scriptDir, '..');
const repoDir = path.resolve(shipDir, '..', '..');
const citoolDir = path.join(repoDir, 'tools', 'citool');
const citoolBin = process.env.CITOOL_BIN ?? path.join(citoolDir, 'dist', 'citool.js');
const shipBin = process.env.SHIP_BIN ?? path.join(shipDir, 'target', 'debug', 'ship');

function runOrThrow(label, command, args, options = {}) {
  const result = spawnSync(command, args, {
    cwd: options.cwd ?? repoDir,
    encoding: 'utf8',
    env: {
      ...process.env,
      SNAPSHOTS_SECRET_ACCESS_KEY: 'test-secret',
      TZ: 'UTC',
      ...options.env,
    },
  });
  assert.equal(
    result.status,
    0,
    `${label} failed:\nstdout:\n${result.stdout}\nstderr:\n${result.stderr}`,
  );
  return result;
}

function runCitool(args, options) {
  return runOrThrow('citool', process.execPath, [citoolBin, ...args], options);
}

function runShip(args, options) {
  return runOrThrow('ship', shipBin, args, options);
}

function escapeXml(value) {
  return value
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&apos;');
}

class MockS3 {
  constructor() {
    this.objects = new Map();
    this.requests = [];
    this.server = http.createServer((request, response) => this.handle(request, response));
  }

  put(key, body, contentType = 'application/octet-stream') {
    this.objects.set(key, { body: Buffer.from(body), contentType });
  }

  text(key) {
    const object = this.objects.get(key);
    assert.ok(object, `missing mock-S3 object ${key}`);
    return object.body.toString('utf8');
  }

  async start() {
    await new Promise((resolve) => this.server.listen(0, '127.0.0.1', resolve));
    const address = this.server.address();
    assert.ok(address && typeof address !== 'string');
    this.endpoint = `http://127.0.0.1:${address.port}`;
    return this;
  }

  async stop() {
    await new Promise((resolve, reject) => this.server.close((error) => (error ? reject(error) : resolve())));
  }

  snapshot() {
    return Object.fromEntries(
      [...this.objects.entries()]
        .filter(([key]) => !/^v1\.12\/\d{8}-\d{6}-catalog\.json$/.test(key))
        .sort(([a], [b]) => a.localeCompare(b))
        .map(([key, object]) => [key, object.body.toString('utf8')]),
    );
  }

  list(prefix) {
    const contents = [...this.objects.keys()]
      .filter((key) => key.startsWith(prefix))
      .sort()
      .map((key) => `<Contents><Key>${escapeXml(key)}</Key><LastModified>2026-01-01T00:00:00.000Z</LastModified><ETag>\"etag\"</ETag><Size>${this.objects.get(key).body.length}</Size><StorageClass>STANDARD</StorageClass></Contents>`)
      .join('');
    return `<?xml version=\"1.0\" encoding=\"UTF-8\"?><ListBucketResult xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\"><Name>snapshots</Name><Prefix>${escapeXml(prefix)}</Prefix><Marker></Marker><IsTruncated>false</IsTruncated>${contents}</ListBucketResult>`;
  }

  async handle(request, response) {
    const url = new URL(request.url, 'http://mock-s3.invalid');
    const [bucket, ...keyParts] = url.pathname.split('/').filter(Boolean);
    const key = keyParts.map(decodeURIComponent).join('/');
    this.requests.push({ method: request.method, key, query: url.searchParams.toString() });

    if (bucket !== 'snapshots') {
      response.writeHead(404).end();
      return;
    }

    if (request.method === 'GET' && !key) {
      response.writeHead(200, { 'content-type': 'application/xml' }).end(this.list(url.searchParams.get('prefix') ?? ''));
      return;
    }

    if (request.method === 'HEAD') {
      response.writeHead(this.objects.has(key) ? 200 : 404).end();
      return;
    }

    if (request.method === 'GET') {
      const object = this.objects.get(key);
      if (!object) {
        response.writeHead(404).end('NoSuchKey');
        return;
      }
      response.writeHead(200, { 'content-type': object.contentType }).end(object.body);
      return;
    }

    if (request.method === 'PUT') {
      const chunks = [];
      for await (const chunk of request) chunks.push(chunk);
      this.put(key, Buffer.concat(chunks), request.headers['content-type'] ?? 'application/octet-stream');
      response.writeHead(200, { etag: '\"etag\"' }).end();
      return;
    }

    if (request.method === 'DELETE') {
      this.objects.delete(key);
      response.writeHead(204).end();
      return;
    }

    response.writeHead(405).end();
  }
}

const fixtureNews = `releases:
  - version: 1.2.3
    date: 2026-01-15T12:34:56Z
    short: Short intro.
    more: More details.
    changes:
      - Fix #123 and improve wrapping behavior with enough words for text output.
      - Second change
  - version: 1.2.2
    date: 2025-12-01T08:00:00Z
    changes:
      - Older change
`;

async function buildShip() {
  runOrThrow('cargo build', 'cargo', ['build', '--quiet', '--manifest-path', path.join(shipDir, 'Cargo.toml')]);
}

async function assertNewsgenParity(tmp) {
  const input = path.join(tmp, 'changes.yaml');
  await writeFile(input, fixtureNews);
  for (const template of ['github', 'news', 'blog', 'debian-changelog']) {
    const citoolOutput = path.join(tmp, `citool-${template}`);
    const shipOutput = path.join(tmp, `ship-${template}`);
    const args = [
      'newsgen', '--input', input, '--template', template, '--single', '--release', '1.2.3',
      '--ubuntu', 'jammy', '--increment', '2',
    ];
    runCitool([...args, '--output', citoolOutput]);
    runShip([...args, '--output', shipOutput]);
    assert.equal(
      await readFile(shipOutput, 'utf8'),
      await readFile(citoolOutput, 'utf8'),
      `newsgen ${template} output differs`,
    );
  }
}

async function readOptional(filename) {
  try {
    return await readFile(filename, 'utf8');
  } catch (error) {
    if (error.code === 'ENOENT') return undefined;
    throw error;
  }
}

async function assertNewsgenInvalidDateParity(tmp) {
  const input = path.join(tmp, 'invalid-date.yaml');
  await writeFile(input, fixtureNews.replace('2026-01-15T12:34:56Z', 'not-a-date'));
  const citoolOutput = path.join(tmp, 'citool-invalid-date');
  const shipOutput = path.join(tmp, 'ship-invalid-date');
  const args = ['newsgen', '--input', input, '--template', 'blog', '--single', '--release', '1.2.3'];
  runCitool([...args, '--output', citoolOutput]);
  runShip([...args, '--output', shipOutput]);
  assert.equal(
    await readOptional(shipOutput),
    await readOptional(citoolOutput),
    'newsgen invalid-date behavior differs',
  );
}

async function runAppcast(tool, tmp) {
  const storage = await new MockS3().start();
  try {
    storage.put('v1.12/catalog.json', JSON.stringify({
      builds: [{
        id: 'build-1', tag: 'v1_2_3', increment: '0', channel: 'stable',
        date: '2026-01-15T12:34:56Z', artifacts: [{
          platform: 'windows', kind: 'installer', configuration: 'release',
          url: 'v1.12/workrave.exe', filename: 'workrave.exe', ed25519: 'signature', size: 42,
        }],
      }],
    }), 'application/json');
    const input = path.join(tmp, 'changes.yaml');
    await writeFile(input, fixtureNews);
    const output = path.join(tmp, `${tool}-appcast.xml`);
    const run = tool === 'citool' ? runCitool : runShip;
    run([
      'appcast', '--endpoint', storage.endpoint, '--secret', 'test-secret', '--branch', 'v1.12',
      '--file', '--name', output, '--input', input,
    ]);
    return await readFile(output, 'utf8');
  } finally {
    await storage.stop();
  }
}

async function assertAppcastParity(tmp) {
  assert.equal(await runAppcast('ship', tmp), await runAppcast('citool', tmp), 'appcast output differs');
}

async function runAppcastToStorage(tool, dry) {
  const storage = await new MockS3().start();
  try {
    storage.put('v1.12/catalog.json', JSON.stringify({
      builds: [{
        tag: 'v1_2_3', increment: '0', channel: 'stable', date: '2026-01-15T12:34:56Z',
        artifacts: [{
          platform: 'windows', kind: 'installer', configuration: 'release', url: 'v1.12/workrave.exe',
          filename: 'workrave.exe', ed25519: 'signature', size: 42,
        }],
      }],
    }), 'application/json');
    const run = tool === 'citool' ? runCitool : runShip;
    run([
      'appcast', '--endpoint', storage.endpoint, '--secret', 'test-secret', '--branch', 'v1.12',
      '--name', dry ? 'dry.xml' : 'stored.xml', ...(dry ? ['--dry'] : []),
    ]);
    return storage.snapshot();
  } finally {
    await storage.stop();
  }
}

async function assertAppcastStorageParity() {
  for (const dry of [false, true]) {
    assert.deepEqual(
      await runAppcastToStorage('ship', dry),
      await runAppcastToStorage('citool', dry),
      `appcast ${dry ? 'dry-run' : 'S3 write'} state differs`,
    );
  }
}

function git(dir, ...args) {
  return execFileSync('git', args, { cwd: dir, encoding: 'utf8', env: { ...process.env, TZ: 'UTC' } }).trim();
}

async function createGitFixture(tmp) {
  const workspace = path.join(tmp, 'workspace');
  await mkdir(workspace);
  git(workspace, 'init', '--quiet');
  git(workspace, 'config', 'user.name', 'Contract Test');
  git(workspace, 'config', 'user.email', 'contract@example.com');
  await writeFile(path.join(workspace, 'history.txt'), 'first\n');
  git(workspace, 'add', 'history.txt');
  git(workspace, 'commit', '--quiet', '-m', 'First commit');
  const previous = git(workspace, 'rev-parse', 'HEAD');
  await writeFile(path.join(workspace, 'history.txt'), 'second\n');
  git(workspace, 'add', 'history.txt');
  git(workspace, 'commit', '--quiet', '-m', 'Second commit');
  return { workspace, previous, head: git(workspace, 'rev-parse', 'HEAD') };
}

function catalogFixture(history) {
  return {
    existing: {
      builds: [
        {
          id: 'new', tag: 'v1_2_3', increment: '0', hash: history.head,
          date: '2026-01-02T00:00:00Z', channel: 'nightly', notes: 'Existing notes',
          artifacts: [{
            url: '/workspace/source/_deploy//snapshots/v1.12/a.exe',
            filename: '/workspace/source/_deploy//snapshots/v1.12/a.exe',
            platform: 'win32', arch: 'ia32', lastmod: '2026-01-01T00:00:00Z', size: 1,
          }],
        },
        {
          id: 'old', tag: 'v1_2_2', increment: '0', hash: history.previous,
          date: '2026-01-01T00:00:00Z', channel: 'stable', artifacts: [{
            url: 'snapshots/v1.12/old.exe', filename: 'snapshots/v1.12/old.exe',
            platform: 'windows', arch: 'x86', lastmod: '2026-01-01T00:00:00Z', size: 2,
          }],
        },
      ],
    },
    part: {
      builds: [{
        id: 'new', tag: 'v1_2_3', increment: '0', hash: history.head,
        date: '2026-01-02T00:00:00Z', channel: 'nightly', notes: '',
        artifacts: [
          {
            url: 'snapshots/v1.12/a.exe', filename: 'snapshots/v1.12/a.exe',
            platform: 'win32', arch: 'ia32', lastmod: '2026-01-02T00:00:00Z', size: 3,
          },
          {
            url: 'snapshots/v1.12/missing.exe', filename: 'snapshots/v1.12/missing.exe',
            platform: 'win32', arch: 'ia32', lastmod: '2026-01-02T00:00:00Z', size: 4,
          },
        ],
      }],
    },
  };
}

async function runCatalog(tool, tmp) {
  const history = await createGitFixture(tmp);
  const fixture = catalogFixture(history);
  const storage = await new MockS3().start();
  try {
    storage.put('v1.12/catalog.json', JSON.stringify(fixture.existing), 'application/json');
    storage.put('v1.12/job-catalog-contract.json', JSON.stringify(fixture.part), 'application/json');
    storage.put('v1.12/a.exe', 'a');
    storage.put('v1.12/old.exe', 'old');
    const run = tool === 'citool' ? runCitool : runShip;
    run([
      'catalog', '--endpoint', storage.endpoint, '--secret', 'test-secret', '--branch', 'v1.12',
      '--workspace', history.workspace,
    ]);
    return storage.snapshot();
  } finally {
    await storage.stop();
  }
}

async function assertCatalogParity(tmp) {
  const citoolState = await runCatalog('citool', path.join(tmp, 'citool-catalog'));
  const shipState = await runCatalog('ship', path.join(tmp, 'ship-catalog'));
  assert.deepEqual(shipState, citoolState, 'catalog mock-S3 state differs');
}

async function main() {
  await buildShip();
  const tmp = await mkdtemp(path.join(os.tmpdir(), 'ship-citool-compat-'));
  try {
    await assertNewsgenParity(tmp);
    await assertNewsgenInvalidDateParity(tmp);
    await assertAppcastParity(tmp);
    await assertAppcastStorageParity();
    await mkdir(path.join(tmp, 'citool-catalog'));
    await mkdir(path.join(tmp, 'ship-catalog'));
    await assertCatalogParity(tmp);
    process.stderr.write('Citool/Ship compatibility checks passed.\n');
  } finally {
    await rm(tmp, { recursive: true, force: true });
  }
}

await main();
