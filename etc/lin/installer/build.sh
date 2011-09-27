#!/bin/bash

VERSION=`cat version`
PACKAGE="ssd_conf"
PROJECT=`cat project`

DIR="$PACKAGE.$PROJECT.$VERSION"

mkdir -p "$DIR"
mkdir -p "./$DIR/dac/conf"
mkdir -p "./$DIR/dsc/conf"

cp -f ../dac/conf/$PROJECT/* "$DIR/dac/conf"
cp -f ../dsc/conf/$PROJECT/* "$DIR/dsc/conf"
cp -f ../configure $DIR
cp -f ./version $DIR

tar cf - "$DIR" | bzip2 -f > "$PACKAGE.$PROJECT.$VERSION.tar.bz2"

rm -rf "$DIR"