#!/bin/bash

if [ "${1:0:1}" = '-' ]; then
	set -- ./orangebox/srcds_run "$@"
fi

set -Eeuox pipefail

originalArgOne="$1"
shouldInitialize=

if [ "$originalArgOne" = './orangebox/srcds_run' ]; then
	env
	pwd
fi

if [[ ! -d orangebox ]]; then
	tar -zxvf /usr/src/steamcmd_linux.tar.gz
	cp /usr/src/tf2_ds.txt /opt/hlserver/
	update.sh

	pushd orangebox/tf/	
	
	tar -zxvf /usr/src/mmsource-*.tar.gz
	tar -zxvf /usr/src/sourcemod-*.tar.gz
	# tar -zxvf /usr/src/runetfmod-*.tar.gz

	popd
fi


# configure runetf

# configure cfg/server.cfg

# healthcheck


exec "$@"

