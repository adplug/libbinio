#!/bin/sh
#
# Shell script to update the "latest documentation" tree from libbinio CVS
# repository.

CVSROOT=":pserver:anonymous@cvs1:/cvsroot/libbinio"
TARGET=latest

export CVSROOT

cd ${TARGET}
cvs update libbinio.texi fdl.texi
makeinfo --html libbinio.texi
