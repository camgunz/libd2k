#!/bin/sh

set -e

# export CBASE_INCLUDE_DIR=${HOME}/local/include
# export CBASE_LIB_DIR=${HOME}/local/lib
export MPDECIMAL_INCLUDE_DIR=${HOME}/local/include
export MPDECIMAL_LIB_DIR=${HOME}/local/lib
export CBASE_PREFIX=${HOME}/local

BUILD_DIR='cbuild'
BUILD_TYPE='Debug'
# BUILD_TYPE='Release'

mkdir -p ${BUILD_DIR}

pushd ${BUILD_DIR} > /dev/null

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DBIN_DIR=~/local/bin \
      -DLIB_DIR=~/local/lib \
      -DINCLUDE_DIR=~/local/include \
      -DSHARE_DIR=~/local/share \
      ..

make

popd > /dev/null
