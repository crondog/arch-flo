#!/bin/sh

if [ -e /lib/firmware/$FIRMWARE ]
then
    echo 1 > /sys/$DEVPATH/loading
    cat /lib/firmware/$FIRMWARE > /sys/$DEVPATH/data
    echo 0 > /sys/$DEVPATH/loading
else
    echo -1 > /sys/$DEVPATH/loading
fi
