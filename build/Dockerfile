FROM ubuntu

ENV TERM linux

RUN bash -xc 'apt-get update && apt-get install -y gcc g++ g++-multilib python git'

WORKDIR /opt/

RUN bash -xc 'mkdir -p /opt/build/dist/ && git clone http://github.com/pliesveld/runetf && cd runetf && git submodule init && git submodule update && pushd alliedmodders/ambuild && python setup.py install && popd && mkdir build && cd build/ && python ../configure -s tf2 && ambuild && cp runetf_ext/runetf.ext.2.tf2/runetf.ext.2.tf2.so /opt/build/dist/'


