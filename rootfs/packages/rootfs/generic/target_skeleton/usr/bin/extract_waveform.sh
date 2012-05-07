#!/bin/sh
if [ -f /root/Settings/wave_tmp ]; then
    rm /root/Settings/wave_tmp
fi
    
bs_sfmrw read 2182 40000 /root/Settings/wave_tmp
md5sum /root/Settings/wave_tmp > /root/Settings/wave_md5

if [ $SCREEN_WIDTH = 600 ]; then
    bs60_init
elif [ $SCREEN_WIDTH = 824 ]; then
    bs97_init
fi

if [ -f /root/Settings/wave_tmp ]; then
    rm /root/Settings/wave_tmp
fi

