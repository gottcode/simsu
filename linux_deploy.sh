#!/bin/bash

EXE='simsu'
VERSION='1.2.1'
DIR="${EXE}-${VERSION}"

strip $EXE

echo -n 'Copying files... '
rm -rf $DIR
mkdir $DIR
cp simsu $DIR
cp icons/simsu.png $DIR
cp icons/simsu.desktop $DIR
echo 'Done'

echo -n 'Creating tarball... '
tar czf "$DIR.tar.gz" $DIR
echo 'Done'

echo -n 'Cleaning up temporary files... '
rm -rf $DIR
echo 'Done'
