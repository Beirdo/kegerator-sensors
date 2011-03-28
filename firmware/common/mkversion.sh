#!/bin/sh

VERSION=`git describe --always --dirty`

cat > version.c <<EOF
#include "local.h"
#include <stdint.h>

uint8_t sensor_fw_version[MAX_BUF_LEN] = "${VERSION}";
EOF
