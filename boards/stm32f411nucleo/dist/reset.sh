#!/bin/sh

openocd -f "${RIOTBOARD}/${BOARD}/dist/openocd.cfg" \
    -c "init" \
    -c "reset run" \
    -c "shutdown"
