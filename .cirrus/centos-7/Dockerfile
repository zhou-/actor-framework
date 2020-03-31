FROM centos:7

RUN yum -y install \
    epel-release \
    centos-release-scl

RUN yum -y install \
    devtoolset-7 \
    git \
    cmake3 \
    make \
    openssl \
    openssl-devel \
    && yum clean all \
    && rm -rf /var/cache/yum

RUN echo 'unset BASH_ENV PROMPT_COMMAND ENV' > /usr/bin/cirrus-ci-env && \
    echo 'source /opt/rh/devtoolset-7/enable' >> /usr/bin/cirrus-ci-env

ENV BASH_ENV="/usr/bin/cirrus-ci-env" \
    ENV="/usr/bin/cirrus-ci-env" \
    PROMPT_COMMAND=". /usr/bin/cirrus-ci-env"
