#!/bin/bash

echo $1 > VERSION
git add .
git commit -a -m "version $1"
git tag -a $1 -m "version $1"

