#!/bin/bash

IMAGE=$1

openocd -f "${RIOTBOARD}/${BOARD}/dist/${BOARD}_jtag.cfg" \
    -f "target/stm32f1x.cfg" \
    -c "init" \
    -c "targets" \
    -c "reset halt" \
    -c "reset init" \
    -c "flash write_image erase ${IMAGE}" \
    -c "verify_image ${IMAGE}" \
    -c "reset run"\
    -c "shutdown"
