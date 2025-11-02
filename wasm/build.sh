#!/usr/bin/env bash
set -euo pipefail

em++ -O3 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='createValidatorModule' \
  -s ENVIRONMENT=web \
  -s DISABLE_EXCEPTION_CATCHING=0 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s INITIAL_MEMORY=268435456 \
  -s MAXIMUM_MEMORY=2147483648 \
  -s STACK_SIZE=2097152 \
  -s EXPORTED_FUNCTIONS='["_validate_json","_free_buffer","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["cwrap","UTF8ToString","lengthBytesUTF8","stringToUTF8"]' \
  -I ../validator/inc \
  ../validator/src/mapping.cc \
  ../validator/src/validator.cc \
  ../validator/src/rules.cc \
  -o validator.js

mkdir -p ../public/wasm
mv -f validator.js   ../public/wasm/validator.js
mv -f validator.wasm ../public/wasm/validator.wasm
echo "Built to public/wasm/"
