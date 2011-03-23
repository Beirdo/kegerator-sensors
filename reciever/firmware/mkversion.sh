#!/bin/sh

VERSION=`cat -`

cat > version.c <<EOF
#include <stdint.h>
#include "local.h"

uint8_t sensor_fw_version[MAX_BUF_LEN] = "${VERSION}";
EOF
