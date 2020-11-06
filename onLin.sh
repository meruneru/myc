#!/bin/bash
CURRENT=$(cd $(dirname $0); pwd)
docker run --rm -it -v $CURRENT:/home/myc -w /home/myc debian $@
