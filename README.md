# Embsys project [![Build Status](https://travis-ci.org/Alex2242/embsys.svg?branch=master)](https://travis-ci.org/Alex2242/embsys)

## build

The project can be build for the native architure with

```bash
./build.sh
```

It can also be cross-compiled for arm-buildroot-linux-uclibcgnueabihf-gcc
by specifying the location of the targeted architure toolchain.

```bash
./build.sh -tp <toolchain-path>
```

_See `build -h` for more information_ 

_note_: arm-buildroot-linux-uclibcgnueabihf-gcc requires libmpfr.so.4 that may not be
provided in recent distros (eg Arch), run `sudo ln -s /lib/libmpfr.so /lib/libmpfr.so.4`
to fix this.

## Release

Binaries can be found in the [release section](https://github.com/Alex2242/embsys/releases) for both x86 and arm.

## Usage

### Server

The server needs to be started on the machine which has a camera.
The options available for the server are described in `server -h`:

A simple usage example would be:

```bash
Usage: server (-c | -s) [OPTION]...
        server -s [OPTION]...
                start the server and wait for connections from client
        server -c [OPTION]...
                Capture an image on the server

Options:
        -p | --port  PORT             Port for clients to connect [21245]
        -d | --device NAME            Video device name [/dev/video0]
        -m | --mmap                   Use memory mapped buffers [default]
        -r | --read                   Use read() calls
        -u | --userptr                Use application allocated buffers
        -p | --port PORT              The port of the server [21245]
        -o | --output FILENAME        Set JPEG output filename [capture.jpg]
        -q | --quality JPEG_QUALITY   Set JPEG quality (0-100) [70]
        -W | --width WIDTH            Set image width [640]
        -H | --height HEIGHT          Set image height [480]
        -S | --syslog                 Use system logger instead of stdout for logging
        -h | --help                   Print this message
```

Which starts a the server and awaits connections from clients.

### Client

The client needs to be able to access the server using ipv4 on the designated port.
The options available for the server are described in `client -h`:

```bash
Usage: 
        client -a SERVER_ADDR [OPTION]...
                connect the server, handshake and disconnect
Options:
        -p | --port PORT              The port of the server [21245]
        -o | --output FILENAME        Set JPEG output filename [capture.jpg]
        -q | --quality JPEG_QUALITY   Set JPEG quality (0-100) [70]
        -W | --width WIDTH            Set image width [640]
        -H | --height HEIGHT          Set image height [480]
        -c | --capture                Capture an image on the server
        -s | --shutdown               Shutdown server after transaction
        -S | --syslog                 Use system logger instead of stdout for logging
        -h | --help                   Print this message
```

## Authors

- Alexandre Degurse
- Julien Khalile
- Henry Stoven
- Driss Tayebi