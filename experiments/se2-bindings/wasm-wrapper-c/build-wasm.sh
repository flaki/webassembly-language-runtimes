if [[ ! -v WASI_SDK_PATH ]]; then
    echo "WASI_SDK_PATH is required. Download wasi-sdk and set the variable"
    exit 1
fi

export TARGET_DIR=target/wasm32-wasi
if [[ "$1" == "--clean" ]]; then
    rm -rf ${TARGET_DIR}
fi

mkdir -p ${TARGET_DIR}/deps 2>/dev/null
export DEPS_CHECKSUM="3d058be4c6e74d40c76307ac634f0930  -"
if [ "${DEPS_CHECKSUM}" == "$(find target/wasm32-wasi/deps | md5sum)" ]; then
    echo "Dependencies already downloaded. Reusing..."
else
    curl -sL https://github.com/assambar/webassembly-language-runtimes/releases/download/python%2F3.11.1%2B20230223-8a6223c/libpython-3.11.1.tar.gz | tar xzv -C ${TARGET_DIR}/deps
    if [ "${DEPS_CHECKSUM}" != "$(find target/wasm32-wasi/deps | md5sum)" ]; then
        echo "Dependency checksum mismatch. Run \`find target/wasm32-wasi/deps | md5sum\` and update DEPS_CHECKSUM"
        exit 1
    fi
fi

export INCLUDE_DIRS="$(realpath ${TARGET_DIR}/deps/include/python3.11)"

export LIBS_DIRS="-L/$(realpath ${TARGET_DIR}/deps/lib/wasm32-wasi)"
export WASI_LIBS="-lwasi-emulated-getpid -lwasi-emulated-signal -lwasi-emulated-process-clocks"
export LINK_FLAGS="${LIBS_DIRS} -lpython3.11 ${WASI_LIBS}"

export CMAKE_EXTRA_ARGS="-DWASI_SDK_PREFIX=${WASI_SDK_PATH} -DCMAKE_TOOLCHAIN_FILE=${WASI_SDK_PATH}/share/cmake/wasi-sdk.cmake"

cmake -B${TARGET_DIR} ${CMAKE_EXTRA_ARGS} . || exit 1
cmake --build ${TARGET_DIR} --verbose || exit 1

echo "Module built in $(realpath ${TARGET_DIR}/wasm-wrapper-c.wasm)"
