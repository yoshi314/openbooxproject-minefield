#!/bin/sh
echo "0" > /sys/class/mmc_host/mmc0/rescan
sleep 1
