#!/bin/bash

SOURCE_DIR=$1
BINARY_DIR=$2

cd ${SOURCE_DIR}

if [[ ! -f configure_flag ]]; then ./configure --prefix="${BINARY_DIR}" && touch configure_flag; fi

if ! make -q; then make -j 4 && make install; fi

exit 0