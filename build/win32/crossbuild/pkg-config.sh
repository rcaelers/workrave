#!/bin/sh

unset PKG_CONFIG_DIR
unset PKG_CONFIG_PATH
export PKG_CONFIG_LIBDIR=${SYSROOT}/lib/pkgconfig:${SYSROOT}/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=${SYSROOT}

exec pkg-config --define-variable=prefix=/ "$@"
