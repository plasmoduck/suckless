#!/bin/sh

SCCPREFIX=${SCCPREFIX:-@PREFIX@}
${SCCPREFIX}/bin/cc -E $@
