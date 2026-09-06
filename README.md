# Kitchen Display

A Raspberry Pi kiosk application (C++/LVGL) that renders a Trello-backed kitchen
board on an attached display.

## Building

The application is cross-compiled for aarch64. In a KubeCoder environment the
whole build is one verb:

```bash
kc project build
```

That runs the two passes of `scripts/dockcross/crossbuild.sh` in order — the
same two the Jenkinsfile runs — inside the `arm64-cross` tool container:

1. `crossbuild.sh prerequisites` — a **native x86_64** ICU host build, with
   `CC`/`CXX` overridden to the container's native compilers. Its output is what
   the cross ICU build consumes through `--with-cross-build`.
2. `crossbuild.sh` — libbacktrace, OpenSSL, zlib, cURL, ICU, LVGL and the
   application itself, all cross-compiled.

The vendored libraries are git submodules under `lib/`, which `crossbuild.sh`
initialises itself; nothing has to be done to a fresh clone first.
`tools/windows_simulator` is deliberately left uninitialised.

Output lands in `build/` (the libraries) and `bin/` (the `main` binary plus the
shared libraries it needs). The build is incremental — a library whose
`build/lib/<name>` already exists is skipped — so only the first run from a
clean tree is slow. `scripts/clean.sh` resets the tree completely.

`kc project lint` validates `docs/architecture/*.yaml` against the architecture
validation service, as `Jenkinsfile.architecture` does. There is no test suite.

The binary runs only on the physical display host; Jenkins deploys `bin/` there
over rsync and restarts the `kitchendisplay` service. `scripts/run.sh` and
`scripts/debug.sh` are for use on that host, not in a development environment.

## Installation

Splash screen.

```bash
sudo apt -y install rpd-plym-splash
```

Copy `support/splash.png` to `/usr/share/plymouth/themes/pix/`.

Enable the splash screen.

```bash
sudo raspi-config
# System Options | Splash Screen
```

If the splash screen doesn't show.

```bash
sudo update-initramfs -u
```
