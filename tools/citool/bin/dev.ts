#!/usr/bin/env -S npx ts-node

import oclif from '@oclif/core';
import path from 'node:path';
import url from 'node:url';
import { register } from 'ts-node';

const project = path.join(path.dirname(url.fileURLToPath(import.meta.url)), '..', 'tsconfig.json');

// In dev mode -> use ts-node and dev plugins
process.env.NODE_ENV = 'development'

register({ project })

// In dev mode, always show stack traces
oclif.settings.debug = true;

// Start the CLI
oclif
  .run(void 0, import.meta.url)
  .then(oclif.flush)
  .catch(oclif.Errors.handle);
