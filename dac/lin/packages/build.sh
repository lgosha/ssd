#!/bin/bash

VERSION=`cat version`
SVN="/usr/local/bin/svn"
PACKAGE="dac"
set -e

RPMBUILDPLACE=/usr/src/rpm/

pushd ../
./makedist
popd

pushd ../
$SVN export . ../$PACKAGE-$VERSION
popd

pushd ../../$PACKAGE-$VERSION/packages

./set_ver $PACKAGE $VERSION
cp $PACKAGE.spec $RPMBUILDPLACE/SPECS
rm -f $PACKAGE.spec

pushd ../
./makedist
popd

cp ../../$PACKAGE*.bz2 $RPMBUILDPLACE/SOURCES

rm -f $PACKAGE.spec

pushd $RPMBUILDPLACE
rpmbuild -ba SPECS/$PACKAGE.spec
popd $RPMBUILDPLACE

popd

rm -f ../../$PACKAGE-$VERSION.tar.bz2
rm -rf ../../$PACKAGE-$VERSION

mv -f $RPMBUILDPLACE/RPMS/i586/$PACKAGE-$VERSION-1.i586.rpm ../../
mv -f $RPMBUILDPLACE/SRPMS/$PACKAGE-$VERSION-1.src.rpm ../../
