#!/usr/bin/env npx ts-node

import oclif from '@oclif/core';

import * as sourceMapSupport from 'source-map-support';
sourceMapSupport.install();

oclif
  .run(void 0, import.meta.url)
  // .then(oclif.flush)
  .catch(oclif.Errors.handle);

