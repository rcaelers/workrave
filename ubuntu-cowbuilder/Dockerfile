FROM ubuntu:groovy

LABEL maintainer "Rob Caelers <rob.caelers@gmail.com>"

RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get update -y && \
  apt-get install -y debian-archive-keyring cowbuilder pbuilder && \
  apt-get clean

COPY pbuilderrc /root/.pbuilderrc

RUN for dist in hirsute groovy focal bionic xenial unstable testing stable; do \
       DIST=$dist cowbuilder --create --distribution $dist --basepath /var/cache/pbuilder/base-$dist.cow; \
    done

CMD ["/bin/bash"]
