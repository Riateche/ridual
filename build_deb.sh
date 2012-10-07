#!/bin/bash
# Usage:
# ./build_deb.sh          - only prepare build directory
# ./build_deb.sh local    - build deb using pbuilder
# ./build_deb.sh ppa      - send build task to ppa


REPO_PATH=.
BUILD_PATH=/tmp/ridual_build_deb
PROG_NAME=ridual
VERSION=`cat $REPO_PATH/VERSION`

if [[ $1 == ppa ]]
then
  echo "Mode: upload to PPA"
  DISTRIBUTIONS="lucid"
elif [[ $1 == local ]]
then
  echo "Mode: local build"
  DISTRIBUTIONS="precise"
else
  echo "Mode: copy only" 
  DISTRIBUTIONS="precise"
fi

rm -rf ${BUILD_PATH}/$VERSION
mkdir ${BUILD_PATH}/$VERSION
mkdir ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION
cp -R ${REPO_PATH}/* ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION/
cd ${BUILD_PATH}/$VERSION/${PROG_NAME}_$VERSION

for distribution in $DISTRIBUTIONS; do
  echo "${PROG_NAME} ($VERSION-1) $distribution; urgency=low

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
  elif [[ $1 == local ]]
  then
    ## Build package locally (it will appear in '/var/cache/pbuilder/result')
    sudo pbuilder build ../*.dsc
  fi

done




#DISTRIBUTIONS="lucid natty oneiric precise quantal"
#ARCHITECTURES="i386 amd64"
#DISTRIBUTIONS="natty"
#ARCHITECTURES="amd64"
#for distribution in $DISTRIBUTIONS; do
#for architecture in $ARCHITECTURES; do
#done
#done

