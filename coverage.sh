#!/bin/bash

evoke -t coverage

lcov --no-external --capture --directory .  --output-file /tmp/evoke.info

lcov --remove /tmp/evoke.info '/usr/include/*' '/usr/lib/*' -o /tmp/evoke_filtered.info
mkdir html
genhtml --ignore-errors source /tmp/evoke_filtered.info --legend --title "`git log | head -n 1`" --output-directory=html

