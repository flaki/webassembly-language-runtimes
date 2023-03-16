FROM suborbital/e2core:v0.6.0-rc3 as e2core
FROM debian:bullseye as builder
RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y wget curl cmake git zip xz-utils

ENV WASI_VERSION=19
ENV WASI_VERSION_FULL=${WASI_VERSION}.0
ENV WASI_SDK_PATH=/wasi-sdk-${WASI_VERSION_FULL}
ENV CC="${WASI_SDK_PATH}/bin/clang --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot"
RUN wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_VERSION}/wasi-sdk-${WASI_VERSION_FULL}-linux.tar.gz \
	&& tar xvf wasi-sdk-${WASI_VERSION_FULL}-linux.tar.gz

ENV WIZER_VERSION=v1.6.1-beta.4
ENV PATH=${PATH}:/wizer-${WIZER_VERSION}-x86_64-linux
RUN wget https://github.com/bytecodealliance/wizer/releases/download/${WIZER_VERSION}/wizer-${WIZER_VERSION}-x86_64-linux.tar.xz \
	&& tar xvf wizer-${WIZER_VERSION}-x86_64-linux.tar.xz

RUN git clone -b fly-v1 https://github.com/flaki/webassembly-language-runtimes \
	&& cd /webassembly-language-runtimes/experiments/se2-bindings/wasm-wrapper-c \
	&& ./build-wasm.sh \
	&& wizer -f _start \
		--mapdir /plugin::../py-fetch \
		--mapdir /usr::target/wasm32-wasi/deps/usr \
		--allow-wasi \
		--wasm-bulk-memory=true \
		target/wasm32-wasi/wasm-wrapper-c.wasm -o /tmp/py.wasm

#COPY template-tenant.json .
#RUN WASMHASH=$(sha256sum -b /tmp/py.wasm | cut -c-64) \
#    && cat template-tenant.json | sed -e "s/<WASMHASH>/$WASMHASH/" > /tmp/tenant.json \
#    && cd /tmp \
#    && zip /modules.wasm.zip tenant.json py.wasm

FROM nginx

COPY --from=e2core /usr/local/bin/e2core /usr/local/bin/
COPY --from=builder /tmp/py.wasm /

COPY www/* /usr/share/nginx/html/
COPY nginx-server.conf /etc/nginx/conf.d/webserver.conf

COPY entrypoint.sh /
CMD /entrypoint.sh
