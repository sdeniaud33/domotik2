#!/usr/bin/env bash

export DEBUG=domotik:*
echo "Transforming ..."
tsc
if [ $? -ne 0 ]; then
	echo "Failed :("
	exit -1
fi
echo "Running ..."
ts-node .
