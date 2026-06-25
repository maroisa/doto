# Doto

Doto is a tool to help managing your home dotfiles

## How to Build

1. Clone this repository
```bash
git clone https://github.com/maroisa/doto.git
```

2. Install required dependencies

**Ubuntu / Debian / Mint / Pop!_OS**
```bash
sudo apt install build-essential libncurses-dev libgit2-dev libglib2.0-dev
```
**Fedora / RHEL / CentOS**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install ncurses-devel libgit2-devel glib2-devel
```

**Arch Linux / Manjaro / EndeavourOS / CachyOS**
```bash
sudo pacman -S base-devel ncurses libgit2 glib2
```

**Void Linux**
```bash
sudo xbps-install -S base-devel ncurses-devel libgit2-devel glib-devel
```

3. Build
```bash
# to build locally
make

# to build and run
make run

# install to $HOME/.local/bin
make install
```

## How to uninstall

```bash
make uninstall
```
