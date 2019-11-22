# Base image
FROM ubuntu:16.04

################## METADATA ######################
LABEL base.image="ubuntu:16.04"
LABEL version="1"
LABEL software.version="v1.14.0"

################## MAINTAINER ######################
MAINTAINER biolxy <biolxy@aliyun.com>


RUN mv /etc/apt/sources.list /etc/apt/sources.list.bkp && \
    bash -c 'echo -e "deb mirror://mirrors.ubuntu.com/mirrors.txt xenial main restricted universe multiverse\n\
deb mirror://mirrors.ubuntu.com/mirrors.txt xenial-updates main restricted universe multiverse\n\
deb mirror://mirrors.ubuntu.com/mirrors.txt xenial-backports main restricted universe multiverse\n\
deb mirror://mirrors.ubuntu.com/mirrors.txt xenial-security main restricted universe multiverse\n\n" > /etc/apt/sources.list' && \
    cat /etc/apt/sources.list.bkp >> /etc/apt/sources.list && \
    cat /etc/apt/sources.list
RUN  sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
# RUN sed -i s@/archive.ubuntu.com/@/mirrors.tuna.tsinghua.edu.cn/@g /etc/apt/sources.list
# RUN sed -i 's/deb src/# deb src/g' /etc/apt/sources.list

RUN apt-get clean all && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y  \
        autotools-dev   \
        automake        \
        cmake           \
        curl            \
        grep            \
        sed             \
        dpkg            \
        fuse            \
        git             \
        wget            \
        zip             \
        openjdk-8-jre   \
        build-essential \
        pkg-config      \
        python3          \
        python3-dev      \
        python3-pip      \
        bzip2           \
        ca-certificates \
        libglib2.0-0    \
        libxext6        \
        libsm6          \
        libxrender1     \
        git             \
        mercurial       \
        subversion      \
        sudo            \
        zlib1g-dev &&   \
        apt-get clean && \
        apt-get purge && \
        rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
        && apt-get clean && apt-get purge


RUN wget http://opengene.org/MutScan/mutscan
RUN chmod a+x mutscan
RUN mv mutscan /usr/local/bin/
ENV PATH=/usr/local/bin/:$PATH

RUN mkdir /data /config

# Add user biodocker with password biodocker
RUN groupadd fuse && \
    useradd --create-home --shell /bin/bash --user-group --uid 1000 --groups sudo,fuse biodocker && \
    echo `echo "biodocker\nbiodocker\n" | passwd biodocker` && \
    chown biodocker:biodocker /data && \
    chown biodocker:biodocker /config

WORKDIR /data
VOLUME ["/data", "/config"]

# Overwrite this with 'CMD []' in a dependent Dockerfile

ENTRYPOINT ["mutscan"]
CMD ["-h"]

