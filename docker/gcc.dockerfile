FROM gcc:8

LABEL maintainer="grave.jul@gmail.com"

ARG WORKDIR="/root"

WORKDIR ${WORKDIR}

# Install CMake 3.8
RUN wget -q --no-check-certificate https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz && \
    tar -xzf cmake-3.8.2-Linux-x86_64.tar.gz && \
    mv cmake-3.8.2-Linux-x86_64 /opt/cmake && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Install Boost 1.64
RUN wget -q --no-check-certificate https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.bz2 && \
    tar -xjf boost_1_64_0.tar.bz2 && \
    cd boost_1_64_0 && \
    ./bootstrap.sh --with-libraries=system >/dev/null && \
    ./b2 install variant=release link=static threading=multi >/dev/null

COPY . evoke

RUN cmake --version && \
    cmake -E remove_directory ./evoke/build && \
    cmake -E make_directory ./evoke/build && \
    cmake -E chdir ./evoke/build cmake .. && \
    cmake --build ./evoke/build && \
    cmake -E chdir ./evoke/build make install

ENTRYPOINT [ "evoke" ]
CMD [ "--help" ]
