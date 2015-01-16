#!/bin/bash

IMAGE=${1}
TARGET=${2}

openocd -f "board/atmel_samr21_xplained_pro.cfg" \
    -c "init" \
    -c "targets" \
    -c "reset halt" \
    -c "reset init" \
    -c "flash write_image erase ${IMAGE}" \
    -c "verify_image ${IMAGE}" \
    -c "reset run" \
    -c "shutdown" \
    ${TARGET}
