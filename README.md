# Linux-Wav-Programs-Suite
Description
-----------

An exercise in C++ to implement the functionality of a media player (play, stop, plot, ff, rewind etc.) into the terminal. Written in the spirit of the unix hallmark: Write programs that do one thing, and do it well.

The audio control is handled by the [ALSA library](https://www.alsa-project.org/wiki/Main_Page#:~:text=The%20Advanced%20Linux%20Sound%20Architecture%20(ALSA)%20provides%20audio). And the functionality to interact with processes comes from [`procps`](https://gitlab.com/procps-ng/procps).

Currently, this "suite of programs" is only composed of *Play, Stop* and *Pause*. I do not have plans to complete *Plot, ff* and *Rewind* at this time.

Work in progress:
- The dev container isn't quite right. But, i've been trying to get it working using WSL.
- The apps do janky interprocess communication - play writes its PID to a temp file, and stop / pause read the PID from the file. I'd like to do something more linux-y. I added the `procps` repo as a submodule and i'm hoping to find something i can either integrate or steal.
- Audio isn't playing quite right. Is this WSL or a problem with the `play` app?

Setup
------------
From your Linux environment:
1. Install [Docker Engine](https://docs.docker.com/engine/install/),
1. TODO Install the [VSCode Docker extension](https://code.visualstudio.com/docs/containers/overview#_installation),
1. TODO Configure Docker for [rootless mode](https://docs.docker.com/engine/security/rootless/)<sup>1</sup>,
1. TODO Install the [VSCode Dev Containers extension](https://code.visualstudio.com/docs/devcontainers/containers#_installation), and [create a devcontainers.json file](https://code.visualstudio.com/docs/devcontainers/containers#_create-a-devcontainerjson-file),
1. Clone this repository and open a workspace into the root level directory of the cloned repository,
1. Build the Docker image:
    ```
    docker pull gcc
    docker build . -t gcc-alsa
    ```
1. TODO Connect to an instance of this repository's Docker image<sup>2</sup>:
    ```
    docker run -it -e "PULSE_SERVER=${PULSE_SERVER}" -v /mnt/    wslg/:/mnt/wslg/ gcc-alsa:latest  # see note 3
    ```
<sup>1</sup>For rootless mode in WSL, you may need to manually configure DNS:

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

<sup>2</sup>Depending on your Docker installation, you might need to prefix these commands with `sudo`.

<sup>3</sup>The -e / -v options are passed in to make the WSL audio available to the Docker container (credit for this goes to Jeremiah on [stackoverflow](https://stackoverflow.com/a/68316880/4455974)). If using real Linux, you might just need to use `--device /dev/snd` (my [source](https://github.com/mviereck/x11docker/wiki/Container-sound:-ALSA-or-Pulseaudio)). 


Build and run
-----

From inside the container, at the root of the repository directory: 
```
make
```
Then, try 'em out!
```
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

And, to cleanup the Docker stuff if need be:
```
docker rm <container ID>    # remove container
docker rmi gcc-alsa         # remove image
```
