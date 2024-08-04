# Installation

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
