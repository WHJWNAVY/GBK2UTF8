#! /bin/sh
# cd to the script directory:
cd "${0%/*}" || { echo "Couldn't cd to ${0%/*}!"; exit 1; }
CLANG_FORMAT="clang-format"
if ! [ -x "$(command -v ${CLANG_FORMAT})" ]; then
  echo "{$CLANG_FORMAT} not found. skipping..."
  exit 1
fi
find -iname "*.[ch]" | xargs -i ${CLANG_FORMAT} -i --style=file {}
