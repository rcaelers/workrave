#!/usr/bin/env -S npx ts-node

import oclif from '@oclif/core';

oclif
  .run(void 0, import.meta.url)
  .then(oclif.flush)
  .catch(oclif.Errors.handle);

