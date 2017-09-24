FROM ubuntu

ENV TERM linux

RUN bash -xc 'apt-get update && apt-get install -y gcc gcc-multilib g++ g++-multilib python git curl wget'

RUN bash -xc 'apt-get -y install lib32stdc++-4.8-dev ; apt-get -y install lib32z1 lib32z1-dev ; apt-get -y install libc6-dev-i386 libc6-i386'


WORKDIR /opt/

RUN bash -xc 'mkdir -p /opt/alliedmodders'


## Clone dev repos
## Configure ambuild
RUN bash -xc 'mkdir -p /opt/alliedmodders/ && cd /opt/alliedmodders && git clone --recursive https://github.com/alliedmodders/sourcemod && bash sourcemod/tools/checkout-deps.sh && pushd ambuild && python setup.py install && popd && pushd sourcemod/ && mkdir -p build && python ../configure.py -s tf2 --hl2sdk-root ../../ --mms-path ../../mmsource-1.10/ --no-mysql'

RUN bash -xc 'cd alliedmodders/sourcemod/sourcepawn && mkdir -p build && cd build && python ../configure.py && ambuild'

## Copy runetf sourcepawn files

## Compile runetf plugins

RUN bash -xc 'mkdir -p /opt/build/dist/ && git clone http://github.com/pliesveld/runetf && pushd runetf && git submodule init && git submodule update && popd && && pushd runetf && mkdir -p build && cd build/ && python ../configure -s tf2 && ambuild && cp runetf_ext/runetf.ext.2.tf2/runetf.ext.2.tf2.so /opt/build/dist/'


