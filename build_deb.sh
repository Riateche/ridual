#!/bin/bash

REPO_PATH=.
BUILD_PATH=../build
PROG_NAME=ridual
VERSION=`cat $REPO_PATH/VERSION`

if [[ $1 == ppa ]]
then
  echo "Mode: uploading to PPA"
else
  echo "Model: local build"
fi

rm -rf ${BUILD_PATH}/$VERSION
mkdir ${BUILD_PATH}/$VERSION
mkdir ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION
cp -R ${REPO_PATH}/* ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION/
cd ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION

echo "${PROG_NAME} ($VERSION-1) precise; urgency=low

  * No information.

 -- Pavel Strakhov <ri@idzaaus.org>  Fri, 06 Jul 2012 23:39:11 +0400

" >> debian/changelog

tar -pzcf ../${PROG_NAME}_$VERSION.orig.tar.gz *

## generate .dsc and .changes files
debuild -S

if [[ $1 == ppa ]]
then
  ## Send package to PPA
  dput ppa:strahovp/ridual ../*.changes
else
  ## Build package locally (it will appear in '/var/cache/pbuilder/result')
  sudo pbuilder build ../*.dsc
fi





#DISTRIBUTIONS="lucid natty oneiric precise quantal"
#ARCHITECTURES="i386 amd64"
#DISTRIBUTIONS="natty"
#ARCHITECTURES="amd64"
#for distribution in $DISTRIBUTIONS; do
#for architecture in $ARCHITECTURES; do
#done
#done

