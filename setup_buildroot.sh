#!/bin/bash

BUILDROOT_REF=2015.02
BUILDROOT_SOURCE=https://github.com/buildroot/buildroot

if [ ! -d "buildroot" ]; then
git clone $BUILDROOT_SOURCE buildroot
fi
pushd buildroot > /dev/null
git checkout $BUILDROOT_REF
git reset --hard HEAD
git apply ../buildroot_patches/*.patch
popd > /dev/null
