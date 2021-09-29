#!/bin/bash
CURRENT=$(cd $(dirname $0); pwd)
docker run --rm -it  --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" -v $CURRENT:/home/myc -w /home/myc compilerbook $@
