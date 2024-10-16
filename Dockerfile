FROM gcc:14.2.0

# Copy repo into container
COPY . /Linux-Wav-Player-Suite

# Install dependencies
RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash
RUN apt update -qq && apt install -y \
    git \
    git-lfs \
    valgrind \
    clang-format \
    clang-tidy

# Install for compatibility with WSL audio devices
RUN apt install -y \
    libsdl2-dev \
    pulseaudio

# Build procps
RUN apt install -y -qq autopoint autoconf automake libtool-bin gettext libncursesw5-dev dejagnu libnuma-dev libsystemd-dev
WORKDIR /Linux-Wav-Player-Suite/dependencies/procps
RUN ./autogen.sh
RUN ./configure
RUN make
RUN make check
WORKDIR /

# Build and install alsa-lib
# TODO use certs
RUN wget --no-check-certificate https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.9.tar.bz2
RUN tar -xvjf alsa-lib-1.2.9.tar.bz2
WORKDIR /alsa-lib-1.2.9
RUN ./configure --enable-shared=no --enable-static=yes
RUN make install

# Create default user
# Allows for creation of a user that matches the owner of the repo as it is in the host
ARG NEWUSER="dummy"
RUN useradd  --create-home --shell /bin/bash --gid root --groups sudo ${NEWUSER}
RUN chown --recursive ${NEWUSER} /Linux-Wav-Player-Suite
USER ${NEWUSER}
WORKDIR /Linux-Wav-Player-Suite
