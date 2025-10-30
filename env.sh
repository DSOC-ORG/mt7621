#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PATH="$PATH:$SCRIPT_DIR:$SCRIPT_DIR/buildroot/output/host/usr/bin"
