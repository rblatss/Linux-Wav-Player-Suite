FROM gcc:4.9
COPY . .

# Build and install alsa-lib
# TODO use certs
RUN wget --no-check-certificate https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.9.tar.bz2
RUN tar -xvjf alsa-lib-1.2.9.tar.bz2
WORKDIR alsa-lib-1.2.9/
RUN ./configure --enable-shared=no --enable-static=yes
RUN make install
WORKDIR ..
