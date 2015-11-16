#!/bin/bash -x

docker run --name workrave-build -tid -v $PWD/:/build/source rcaelers/workrave-master-build bash
docker exec -ti workrave-build /build/source/build/docker/build.sh windows Gtk+2 RelWithDebInfo
