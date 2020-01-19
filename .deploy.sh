#!/bin/bash

set -o errexit

cd build/x86
mkdir embsys-x86-$TRAVIS_TAG
cp bin/* lib/* embsys-x86-$TRAVIS_TAG

tar -czf embsys-x86-$TRAVIS_TAG.tar.gz embsys-x86-$TRAVIS_TAG