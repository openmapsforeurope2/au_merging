#!/bin/sh
DOCKER_NAME=au_merging_base
DOCKER_TAG="latest"

docker build --no-cache -t $DOCKER_NAME:$DOCKER_TAG -f Dockerfile.base ./..