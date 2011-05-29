#!/bin/sh

root_files="web_browser.db notes_template notes"

if [ -f /root/Settings/explorer ]; then
    . /root/Settings/explorer
fi

sleep 1

if [ ! -f "$EXPLORER" ]; then
    EXPLORER=explorer
    OPTION=""
fi

while true
do
    system_manager -qws -shell="$EXPLORER $OPTION"

    # system_manager crashes
    killall -q -9 system_manager
    killall -q -9 $EXPLORER
    killall -q -9 web_browser
    killall -q -9 wpa_supplicant
    killall -q -9 udhcpc

    # check if /root is full    
    for i in $root_files
    do
        remain_cap=`df | grep mtdblock5 | awk '{print $4}'`
        if [ $remain_cap -ge 1024 ]; then
            break
        fi

        rm -rf /root/$i
    done
done
