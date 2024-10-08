# Linux-Wav-Programs-Suite
Description
-----------

An exercise in C++ to implement the functionality of a media player (play, stop, plot, ff, rewind etc.) into the terminal. Written in the spirit of the unix hallmark: Write programs that do one thing, and do it well.

The audio control is handled by the [ALSA library](https://www.alsa-project.org/wiki/Main_Page#:~:text=The%20Advanced%20Linux%20Sound%20Architecture%20(ALSA)%20provides%20audio). And the functionality to interact with processes comes from [`procps`](https://gitlab.com/procps-ng/procps).

Currently, this "suite of programs" is only composed of *Play, Stop* and *Pause*. I do not have plans to complete *Plot, ff* and *Rewind* at this time.

Work in progress: I created a Dockerfile so that this project can be built and demonstrated anywhere. But, I used WSL to write these instructions, so there are some notes specific for running the Docker container in WSL.

Setup
------------
From your Linux environment:
1. Clone this repository and open a workspace into the root level directory of the cloned repository,
1. Install [Docker Engine](https://docs.docker.com/engine/install/),
1. Install the [VSCode Docker extension](https://code.visualstudio.com/docs/containers/overview#_installation),
1. Configure Docker for [rootless mode](https://docs.docker.com/engine/security/rootless/)<sup>1</sup>,
1. Install the [VSCode Dev Containers extension](https://code.visualstudio.com/docs/devcontainers/containers#_installation), and [create a devcontainers.json file](https://code.visualstudio.com/docs/devcontainers/containers#_create-a-devcontainerjson-file),
1. TODO Connect to an instance of this repository's Docker image:
    ```
    docker pull gcc
    docker build . -t gcc-alsa
    docker run -it -e "PULSE_SERVER=${PULSE_SERVER}" -v /mnt/    wslg/:/mnt/wslg/ gcc-alsa:latest
    ```
Depending on your Docker installation, you might need to prefix these commands with `sudo`.

The -e / -v options are passed in for WSL (credit for this goes to Jeremiah on [stackoverflow](https://stackoverflow.com/questions/68310978/playing-sound-in-docker-container-on-wsl-in-windows-11)). If not using WSL, you probably need to pass in `--device /dev/snd` (my [source](https://github.com/mviereck/x11docker/wiki/Container-sound:-ALSA-or-Pulseaudio)). 

<sup>1</sup>If you're using WSL (like me), you may need to manually configure DNS:

1. Add the following to `/etc/wsl.conf`:
    ```
    [network]
    generateResolvConf = false
    ```
1. Reboot.
1. Create /etc/resolv.conf and add the following:
    ```
    nameserver 8.8.8.8
    ```


Build and run
-----

From inside the container: 
```
make
build/play samples/violins.wav
build/pause  # pause
build/pause  # resume
build/stop
```
The output binaries can also be copied from the container to the host with the following:
```
docker ps -as  # prints a list of container IDs
docker cp <container ID>:/build/* .
```

Cleanup:
```
docker rm <container ID>    # remove container
docker rmi gcc-alsa         # remove image
```
