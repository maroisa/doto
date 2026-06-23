# Doto

Doto is a tool to help managing your home dotfiles

## How to Build

1. Clone this repository
```bash
git clone https://github.com/maroisa/doto.git
```

2. Install required dependencies
```bash
# Debian
sudo apt update && sudo apt install build-essential libncurses-dev

# Arch Linux
sudo pacman -Syu base-devel ncurses

# Void Linux
sudo xbps-install -Sy base-devel ncurses-devel

# Fedora
sudo dnf group install "Development Tools" ncurses-devel
```

> [!NOTE]
> Majority of distribution already has `ncurses`.

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
