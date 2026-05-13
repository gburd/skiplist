#!/bin/sh
# autogen.sh -- run autoreconf to generate the ./configure script
# from configure.ac.  Required only for users building from a git
# checkout; release tarballs ship the pre-built configure script.

set -eu

if ! command -v autoreconf >/dev/null 2>&1; then
    echo "error: autoreconf not found; install autoconf" >&2
    exit 1
fi

autoreconf --install --force --warnings=all
echo "autogen.sh: configure script ready -- run ./configure now"
