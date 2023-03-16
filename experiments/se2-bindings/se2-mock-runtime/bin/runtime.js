#!/usr/bin/env node

const yargs = require("yargs");
const { WASI } = require('wasi');
const path = require('path');
const fs = require('fs');

const { WasmMemoryManager } = require('../mem-utils');
const { get } = require('../sync-request');

log = function() {
    var prefix = "\x1b[33m[runtime.js]\x1b[0m |";
    return Function.prototype.bind.call(console.log, console, prefix);
}();

// Parse args
const options = yargs
    .usage("Usage: -wrapper WASM_MODULE_PATH -plugin PYTHON_PLUGIN_ROOT")
    .option("wrapper", { describe: "Path to the Wasm module to run", type: "string", demandOption: true })
    .option("plugin-root", { describe: "Path to root folder with plugin.py", type: "string", demandOption: true })
    .option("python-usr-root", { describe: "Path to folder containing python's stdlib. Inside this folder we expect to see usr/local/lib...", type: "string", demandOption: true })
    .option("payload", { describe: "Override payload passed into the Wasm module", type: "string" })
    .argv;

const wasi = new WASI({
    args: [options.wrapper],
    env: {
        'PYTHONPATH': '/plugin'
    },
    preopens: {
        '/plugin': options.pluginRoot,
        '/usr': path.join(options.pythonUsrRoot, 'usr')
    },
});

memMgr = null;

const importObject = {
    "wasi_snapshot_preview1": {
        // sock_accept is not available, but is part of the wasi_snapshot_preview1 specification
        sock_accept(fd, flags) {
            return fd;
        },
        ...wasi.wasiImport
    },
    env: {
        _ffi_data: null,
        return_result(result, result_len, ident) {
            const buf = memMgr.wrapBuf(result, result_len);
            const strResult = memMgr.strFromBuf(buf);
            log(`Returned result "${strResult}" with ident ${ident}`);
        },
        return_error(code, msg, msg_len, ident) {
            const buf = memMgr.wrapBuf(msg, msg_len);
            const strMsg = memMgr.strFromBuf(buf);
            log(`Returned error {code:${code}, message="${strMsg}"} with ident ${ident}`);
        },
        log_msg(msg, msg_len, loglevel, ident) {
            const buf = memMgr.wrapBuf(msg, msg_len);
            const strMsg = memMgr.strFromBuf(buf);
            log(`id=${ident} | log_msg( INFO ): `+strMsg)
        },
        fetch_url(method, url, url_len, body, body_len, ident) {
            const strUrl = memMgr.strFromBuf(memMgr.wrapBuf(url, url_len));
            log(`id=${ident} | fetch_url( GET ${strUrl} )`)

            // TODO: method/request body support is unimplemented

            // Sync request to the URl, will return a buffer
            const httpResponse = get(strUrl);

            //const response = new TextEncoder().encode(httpResponseString);
            const response = httpResponse;
            const responseSize = response.byteLength;

            log(`HTTP GET ${strUrl} returned ${httpResponse.byteLength} bytes of data`)

            this._ffi_data = response
            return responseSize
        },
        get_ffi_result(memptr, ident) {
            log(`id=${ident} | get_ffi_result( ${memptr} )`)

            const result = this._ffi_data
            const resultSize = result.length

            const buf = memMgr.wrapBuf(memptr, resultSize);
            buf.setBytes(result);

            log(`Copied ${result.length} bytes to ffi_result buffer`)
        },
    },
};

(async () => {
    log(`Loading module ${options.wrapper} ...`);
    const wasmBuffer = fs.readFileSync(options.wrapper);
    wasmModule = await WebAssembly.instantiate(wasmBuffer, importObject);
    memMgr = new WasmMemoryManager(wasmModule.instance.exports);

    if (Object.hasOwn(wasmModule.instance.exports, '_start'))
        wasi.start(wasmModule.instance);
    else if (Object.hasOwn(wasmModule.instance.exports, '_initialize'))
        wasi.initialize(wasmModule.instance);
    else
        throw Error('WASM module should export either _start or _initialize');

    log('Started wasm module');

    const strBuf = memMgr.bufFromString(options.payload ?? "Flaki");

    const ident = 12345;
    log(`Calling wasmModule.run_e(${strBuf.getPtr()}, ${strBuf.getSize()}, ${ident})...`);
    wasmModule.instance.exports.run_e(strBuf.getPtr(), strBuf.getSize(), ident);

    log('wasmModule.run_e returned');
    memMgr.deallocateBuf(strBuf);

})().catch(e => {
    log("Error: ", e);
});
