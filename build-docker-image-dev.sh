#!/bin/sh
PROJECT_NAME=au_merging
DOCKER_TAG="latest"

docker build --no-cache -t $PROJECT_NAME:$DOCKER_TAG -f Dockerfile.dev .