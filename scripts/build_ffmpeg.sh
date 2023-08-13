#!/bin/bash

SOURCE_DIR=$1
BINARY_DIR=$2


if [[ ! -d ${BINARY_DIR} ]]; then
  mkdir -p ${SOURCE_DIR}

  cd ${SOURCE_DIR} || exit
  git clone "https://git.ffmpeg.org/ffmpeg.git" .
  git checkout "release/6.0"

  ./configure --prefix="${BINARY_DIR}" --disable-programs --disable-doc --enable-hardcoded-tables

  if ! make -q; then CXXFLAGS="-march=native -O3" make -j 4 && make install; fi
fi
exit 0