#!/bin/sh

killall -9 udevd
killall -9 watchdog.sh
killall -9 wpa_supplicant

modprobe -r libertas_sdio

sdmmc_module.sh unload

# Clear screen now.
echo "1" > /sys/onyx/clear_at_poweroff

poweroff

