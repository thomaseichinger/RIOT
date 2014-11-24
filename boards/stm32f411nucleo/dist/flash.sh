#!/bin/sh

openocd -f "${RIOTBOARD}/${BOARD}/dist/openocd.cfg" \
    -c "init" \
    -c "targets" \
    -c "reset" \
    -c "halt" \
    -c "flash protect 0 0 7 off" \
    -c "flash erase_address 0x08000000 0x20000" \
    -c "flash write_image erase $1 0 ihex" \
    -c "verify_image $1" \
    -c "reset run"\
    -c "shutdown"
