#!/bin/bash
# Usage: ./set_version.sh 0.1.2

echo $1 > VERSION
git add .
git commit -a -m "version $1"
git tag -a v$1 -m "version $1"

