CC = gcc
CFLAGS = -Wall -g $(shell pkg-config --libs --cflags ncursesw glib-2.0 gio-2.0 libgit2) -DAPP_VERSION=\"$(DOTO_VERSION)\"
DOTO_EXEC = doto
DOTO_SOURCE = main.c doto.c
DOTO_VERSION = "v0.0.0"

.PHONY: all build clean

all: build

run: $(DOTO_SOURCE)
	@$(MAKE) build
	@echo ""
	@./doto

build: $(DOTO_SOURCE)
	@echo "Building the source..."
	$(CC) $(CFLAGS) -o $(DOTO_EXEC) $(DOTO_SOURCE)

install: $(DOTO_SOURCE)
	@$(MAKE) build
	@echo "\nInstalling in $(HOME)/.local/bin ..."
	@mv ./doto $(HOME)/.local/bin/doto
	@echo "\nSuccess!"

uninstall:
	@echo "Removing $(PWD)/doto"
	@echo "Removing $(HOME)/.local/bin/doto"
	@rm -f $(DOTO_EXEC)
	@rm -f $(HOME)/.local/bin/doto
	@echo "\nSuccess!"
