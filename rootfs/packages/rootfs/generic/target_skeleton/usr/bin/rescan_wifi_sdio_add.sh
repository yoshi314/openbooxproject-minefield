#!/bin/sh
echo "1" > /sys/class/mmc_host/mmc0/rescan
sleep 1
