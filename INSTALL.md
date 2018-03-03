# Installing zeroconf\_lookup

**zeroconf\_lookup** is a browser extension for **Firefox** and **Chrome** / **Chromium**.

It consists of two parts:

1. The so called *native host* needs to be installed by the user with operating system specific procedures. It consists of just one executable and an optional config file.
2. The *browser extension* itself is available through the appropriate app store. It locates the *native host* by means of a *browser manifest*.

This file provides instructions for installing the *native host* executable including the *browser manifest* in various environments.

The first step is cloning (or updating) this **GitHub** repository:

`git clone https://github.com/railduino/zeroconf-lookup.git`

In the unlikely event that you don't have `git` installed, you can of course download an archive and unpack it.

Anyway, `cd` into the `zeroconf_lookup` directory. Well, and from here on it gets operating system specific.

# Installing on Windows

**coming soon**

# Installing on OS X / macOS

Hint: I am an *Apple Newbie* and my only device is a *Mac Mini (early 2011)* running *El Capitan* (I cannot upgrade to macOS right now).

The common setup is to install the executable (`zeroconf_lookup`) either at `/usr/local/bin` or any location writable for the running user.
In any case the *browser manifest* must be installed in a well-known location with a reference to this location. This is done by calling `zeroconf_lookup -i`

There are two flavors of the executable - one written in C and one in Go. Maybe you want to check out both and keep the one that you like more :-)

## Installing the **C** version via CMake and XCode

**coming soon**

## Installing the **C** via CMake

**coming soon**

## Installing the **C** via Homebrew

**coming soon (still learning how to brew)**

## Installing the **C** via Makefile only

**coming soon**

# Installing on Linux

**coming soon**

