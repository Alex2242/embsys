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

Binaries can be found in the [release section](https://github.com/Alex2242/embsys/releases)
for both x86 and arm.


## Usage

### Server

The server needs to be started on the machine which has a camera.
The options available for the server are described in `server -h`.

A simple usage example would be:

```bash
./server -s -p [port]
```

Which starts a the server and awaits connections from clients.

### Client

The client needs to be able to access the server using ipv4 on the designated port.
The options available for the server are described in `client -h`.

A simple usage example would be:

```bash
./client -c -o [output.jpg] -a [server-ipv4-addr] -p [server-port]
```

The client requests an image from the server and saves it locally.

## Authors

Alexandre Degurse
Julien Khalile
Henry Stoven
Driss Tayebi