#! /bin/bash

jlinkexe \
    -device EFM32GG11B820F2048 \
    -if SWD \
    -speed 2700 \
    -commanderscript flash.jlink