FROM gcc:latest
COPY . .

# Install stuff
RUN apt install git
RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash
RUN apt install -y git-lfs
RUN apt install -y autopoint
RUN apt-get update -qq && apt-get install -y -qq autopoint autoconf automake libtool-bin gettext libncursesw5-dev dejagnu libnuma-dev libsystemd-dev

# For WSL
RUN apt-get install -y libsdl2-dev
RUN apt-get install -y pulseaudio

# Build procps
WORKDIR dependencies/procps
RUN "./autogen.sh"
RUN "./configure"
RUN make
RUN make check
#RUN make clean
WORKDIR ../..

# Build and install alsa-lib
# TODO use certs
RUN wget --no-check-certificate https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.9.tar.bz2
RUN tar -xvjf alsa-lib-1.2.9.tar.bz2
WORKDIR alsa-lib-1.2.9/
RUN ./configure --enable-shared=no --enable-static=yes
RUN make install
WORKDIR ..
