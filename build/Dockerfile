FROM ubuntu

ENV TERM linux

RUN bash -xc 'apt-get update && apt-get install -y gcc gcc-multilib g++ g++-multilib python git'

RUN bash -xc 'apt-get -y install lib32stdc++-4.8-dev ; apt-get -y install lib32z1 lib32z1-dev ; apt-get -y install libc6-dev-i386 libc6-i386'


RUN bash -c 'mkdir -p /opt/runetf/alliedmodders/ && cd /opt/runetf/alliedmodders && git clone --recursive https://github.com/alliedmodders/sourcemod && bash sourcemod/tools/checkout-deps.sh'

WORKDIR /opt/

RUN bash -xc 'mkdir -p /opt/build/dist/ && git clone http://github.com/pliesveld/runetf && cd runetf && git submodule init && git submodule update && pushd alliedmodders/ambuild && python setup.py install && popd && mkdir -p build && cd build/ && python ../configure -s tf2 && ambuild && cp runetf_ext/runetf.ext.2.tf2/runetf.ext.2.tf2.so /opt/build/dist/'

RUN 



