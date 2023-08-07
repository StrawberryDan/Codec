#!/bin/bash

SOURCE_DIR=$1
BINARY_DIR=$2


if [[ ! -d ${BINARY_DIR} ]]; then
  mkdir -p ${SOURCE_DIR}

  cd ${SOURCE_DIR}
  git clone "https://git.ffmpeg.org/ffmpeg.git" .
  git checkout "release/6.0"

  if [[ ! -f configure_flag ]]; then ./configure --prefix="${BINARY_DIR}" && touch configure_flag; fi

  if ! make -q; then make -j 4 && make install; fi
fi
exit 0