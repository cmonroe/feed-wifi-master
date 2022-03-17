#!/bin/ash
# This script is used for wrapping atenl daemon to ated

# 0 is normal mode, 1 is used for specific commands
mode="0"
add_quote="0"
cmd="atenl"

for i in "$@"
do
    if [ "$i" = "-c" ]; then
        cmd="${cmd} -c"
        mode="1"
        add_quote="1"
    elif [ "${add_quote}" = "1" ]; then
        cmd="${cmd} \"${i}\""
        add_quote="0"
    else
        if [ ${i} = "ra0" ]; then
            i="phy0"
        elif [ ${i} = "rax0" ]; then
            i="phy1"
        fi
        cmd="${cmd} ${i}"
    fi
done

if [ "$mode" = "0" ]; then
    killall atenl
fi

eval "${cmd}"
