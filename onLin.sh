#!/bin/bash

docker run --rm -it -v ~/works/github/myc/:/home/myc -w /home/myc debian $@
