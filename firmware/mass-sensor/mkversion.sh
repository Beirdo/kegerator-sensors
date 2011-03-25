#!/bin/sh

VERSION=`cat -`

cat > version.c <<EOF
#include "local.h"
#include <stdint.h>

uint8_t sensor_fw_version[MAX_BUF_LEN] = "${VERSION}";
EOF
