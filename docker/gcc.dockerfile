FROM alpine:latest

LABEL maintainer="grave.jul@gmail.com"

ARG WORKDIR="/root"

WORKDIR ${WORKDIR}

RUN apk update --no-cache && apk add --no-cache \
    boost-dev \
    cmake \
    g++ \
    make

COPY . evoke

RUN cmake -E remove_directory ./evoke/build && \
    cmake -E make_directory ./evoke/build && \
    cmake -E chdir ./evoke/build cmake .. && \
    cmake --build ./evoke/build && \
    cmake -E chdir ./evoke/build make install

ENTRYPOINT [ "evoke" ]
CMD [ "--help" ]
