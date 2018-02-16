#!/bin/bash

# for executing in non travis environment
[ -z "$TRAVIS_OS_NAME" ] && TRAVIS_OS_NAME="$(uname -s | tr '[:upper:]' '[:lower:]')"

PLATFORM=x86
BUILD_TYPE=Debug

# Detect number of processor cores
# default is 4 jobs
if [[ "$TRAVIS_OS_NAME" == 'osx' || "$TRAVIS_OS_NAME" == 'darwin' ]]
then
	JOBS=$(sysctl -n hw.ncpu)
	PLATFORM=osx
elif [[ "$TRAVIS_OS_NAME" == 'linux' ]]
then
	JOBS=$(nproc)
fi
echo "compile jobs: ${JOBS:=4}"

# compile prepare
mkdir build || exit 1
cd build

# Compile hyperion for tags
[ -n "${TRAVIS_TAG:-}" ] && BUILD_TYPE=Release

# Compile hyperion for cron - take default settings

# Compile for PR (no tag and no cron)
[ "${TRAVIS_EVENT_TYPE:-}" != 'cron' -a -z "${TRAVIS_TAG:-}" ] && PLATFORM=${PLATFORM}-dev

cmake -DPLATFORM=$PLATFORM -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=/usr .. || exit 2
if [[ "$TRAVIS_OS_NAME" == 'linux' ]]
then
	# activate dispmanx and osx mocks
	cmake -DENABLE_OSX=ON -DENABLE_DISPMANX=ON .. || exit 5
fi

# Build the package on osx
if [[ "$TRAVIS_OS_NAME" == 'osx' || "$TRAVIS_OS_NAME" == 'darwin' ]]
then
	make -j ${JOBS} || exit 3
fi

# Build the package with docker
if [[ $TRAVIS_OS_NAME == 'linux' ]]
then
	echo "Startup docker"
	docker build -f ./.travis/Dockerfile.ubuntu -t ubuntu:16.04 --cache-from ubuntu:16.04 . || exit 3
fi
