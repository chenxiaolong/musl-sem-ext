FROM alpine:latest

RUN apk --no-cache add cmake g++ gcc gtest gtest-dev linux-headers make

ADD . /tmp/src/
RUN tmpdir=$(mktemp -d) \
    && cd "${tmpdir}" \
    && cmake /tmp/src \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && ctest -VV \
    && make install \
    && cp musl-sem-ext-test /usr/local/bin/ \
    && cd / \
    && rm -r "${tmpdir}"
