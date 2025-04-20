#!/usr/bin/env bash
set -eu -o pipefail

ROOT_DIR=$(pwd)
declare -A COMPILERS=( [gcc]="gcc g++" [clang]="clang clang++" )
SANITIZERS=( "" ASAN UBSAN TSAN LSAN MSAN "ASAN+UBSAN" )

MAX_JOBS=16

run_config(){
  local compiler=$1 sanitizer=$2
  local CC CXX build_dir config_name
  read -r CC CXX <<< "${COMPILERS[$compiler]}"

  config_name=$compiler
  if [[ -n $sanitizer ]]; then
    local sanitized=${sanitizer//+/_}
    sanitized=${sanitized,,}
    config_name+="-$sanitized"
  else
    config_name+="-nosan"
  fi
  build_dir="$ROOT_DIR/build-$config_name"

  echo "=== [$config_name] start ==="
  rm -rf "$build_dir" && mkdir -p "$build_dir"

  declare -A FLAGS=(
    [AA_ENABLE_WARNINGS]=ON
    [AA_ENABLE_ASAN]=OFF
    [AA_ENABLE_UBSAN]=OFF
    [AA_ENABLE_TSAN]=OFF
    [AA_ENABLE_LSAN]=OFF
    [AA_ENABLE_MSAN]=OFF
  )
  if [[ -n $sanitizer ]]; then
    IFS='+' read -ra parts <<< "$sanitizer"
    for s in "${parts[@]}"; do
      FLAGS[AA_ENABLE_${s}]=ON
    done
  fi

  cmake_args=( -S "$ROOT_DIR" -B "$build_dir"
               -DCMAKE_BUILD_TYPE=Debug
               -DCMAKE_C_COMPILER="$CC"
               -DCMAKE_CXX_COMPILER="$CXX" )
  for key in "${!FLAGS[@]}"; do
    cmake_args+=( "-D${key}=${FLAGS[$key]}" )
  done

  cmake "${cmake_args[@]}" \
    && cmake --build "$build_dir" \
    && ctest --test-dir "$build_dir" --verbose \
    && echo "=== [$config_name] OK ===" \
    || echo "!!! [$config_name] FAILED" >&2
}

jobs_running(){
  jobs -rp | wc -l
}

for compiler in "${!COMPILERS[@]}"; do
  for sanitizer in "${SANITIZERS[@]}"; do
    run_config "$compiler" "$sanitizer" &

    while (( $(jobs_running) >= MAX_JOBS )); do
      sleep 0.5
    done
  done
done

wait
echo "All builds done."

