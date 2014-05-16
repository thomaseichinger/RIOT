#!/bin/sh

SYSTEM=`uname -s`

if [ "${SYSTEM}" = "FreeBSD" ]; then
    sh ./tapsetup-freebsd.sh $@
elif [ "${SYSTEM}" = "Darwin" ]; then
    sh ./tapsetup-osx.sh $@
else
    sh ./tapsetup-linux.sh $@
fi

exit $?
