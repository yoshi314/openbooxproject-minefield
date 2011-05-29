#!/bin/sh
if [ $# -ne 2 ]; then
    echo "Usage: `basename $0` <version file> <version number>"
    exit 1
fi

GIT_COMMIT=`cat $1 | grep GIT_COMMIT | awk -F"=" '{print $2}'`
DONOR_VERSION=`cat $1 | grep VERSION | awk -F"\"" '{print $2}' | awk -F" " '{print $1}'`
DONOR_BUILDDATE=`cat $1 | grep VERSION | awk -F"\"" '{print $2}' | awk -F" " '{print $2}'`

echo "export GIT_COMMIT=$GIT_COMMIT"
echo
echo "export OBX_VERSION=\"$2\""
echo "export OBX_BUILDDATE=\"`date +"%Y%m%d"`\""
echo "export DONOR_VERSION=\"$DONOR_VERSION\""
echo "export DONOR_BUILDDATE=\"$DONOR_BUILDDATE\""
echo
echo "export VERSION=\"$DONOR_VERSION-$OBX_VERSION $OBX_BUILDDATE\""
