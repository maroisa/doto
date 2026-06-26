# Doto

Doto is a tool to help managing your home dotfiles

## Usage

**Init**
```bash
# For the first time, run dotfiles repository initialization.
doto init

# If you already have dotfiles repository, pass your repository as argument
doto init https://github.com/USER/DOTFILES_REPO

# use --force if directory already exists to overwrite
```

**Add to Doto's Directory**
```bash
doto add my_file1 my_file2 ...
```

**Go to Doto's Directory**
```bash
doto cd

# Then operates your git ...
```

## How to Install
1. Get build artifact from [Github Actions](https://github.com/maroisa/doto/actions)
2. Extract it
3. Install required dependecies


**Ubuntu / Debian / Mint / Pop!_OS**
```bash
sudo apt install build-essential libncurses libgit2
```

**Fedora / RHEL / CentOS**
```bash
sudo dnf install ncurses libgit2
```

**Arch Linux / Manjaro / EndeavourOS / CachyOS**
```bash
sudo pacman -S ncurses libgit2 
```

**Void Linux**
```bash
sudo xbps-install -S ncurses libgit2
```

4. Run `doto` or move it to `$HOME/.local/bin`


## How to Build

1. Clone this repository
```bash
git clone https://github.com/maroisa/doto.git
```

2. Install required dependencies

**Ubuntu / Debian / Mint / Pop!_OS**
```bash
sudo apt install build-essential libncurses-dev libgit2-dev
```
**Fedora / RHEL / CentOS**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install ncurses-devel libgit2-devel 
```

**Arch Linux / Manjaro / EndeavourOS / CachyOS**
```bash
sudo pacman -S base-devel ncurses libgit2 
```

**Void Linux**
```bash
sudo xbps-install -S base-devel ncurses-devel libgit2-devel 
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
