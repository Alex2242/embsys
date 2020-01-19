# Embsys project

## build

```bash
./build.sh [toolchain path]
```
_note_: arm-buildroot-linux-uclibcgnueabihf-gcc requires libmpfr.so.4 that may not be
provided in recent distros (eg Arch), run `sudo ln -s /lib/libmpfr.so /lib/libmpfr.so.4`
to fix this.
