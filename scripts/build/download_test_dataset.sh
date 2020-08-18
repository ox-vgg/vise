#!/bin/bash

## check dependency location provided by user
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 [FOLDER TO STORE TEST DATASET]" >&2
  exit 1
fi
if ! [ -e "$1" ]; then
  echo "$1 not found" >&2
  exit 1
fi
if ! [ -d "$1" ]; then
  echo "$1 not a directory" >&2
  exit 1
fi

DATADIR=$1
if ! [ -f "${DATADIR}/15th-Century-Illustrations.zip" ]; then
    wget -O $DATADIR/15th-Century-Illustrations.zip https://www.robots.ox.ac.uk/~vgg/software/vise/_internal/test/asset/15th-Century-Illustrations.zip
fi
if ! [ -d "${DATADIR}/15th-Century-Illustrations" ]; then
    unzip $DATADIR/15th-Century-Illustrations.zip -d ${DATADIR}/
fi

if ! [ -f "${DATADIR}/Oxford-Buildings.zip" ]; then
    wget -O $DATADIR/Oxford-Buildings.zip https://www.robots.ox.ac.uk/~vgg/software/vise/_internal/test/asset/Oxford-Buildings.zip
fi
if ! [ -d "${DATADIR}/Oxford-Buildings" ]; then
    unzip $DATADIR/Oxford-Buildings.zip -d ${DATADIR}/
fi
