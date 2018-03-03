# Installing zeroconf\_lookup

**zeroconf\_lookup** is a browser extension for **Firefox** and **Chrome** / **Chromium**.

Hint: The **Chrome** extension has not been checked into the app store, this is next on the agenda .....

It consists of two parts:

1. The so called *native host* needs to be installed by the user with operating system specific procedures. It consists of just one executable and an optional config file.
2. The *browser extension* itself is available through the appropriate app store. It locates the *native host* by means of a *browser manifest*.

This file provides instructions for installing the *native host* executable including the *browser manifest* in various environments.

The first step is cloning (or updating) this **GitHub** repository, unless you are using *Homebrew*, *RPM* or the like (then just see below).

`git clone https://github.com/railduino/zeroconf-lookup.git`

In the unlikely event that you don't have `git` installed, you can of course download an archive and unpack it.

Anyway, `cd` into the `zeroconf_lookup` directory. Well, and from here on it gets operating system specific.

# Installing on Windows

The Windows version is a **Go** program, so you should have `Go` installed. The minimum version is **1.6** since we are using *context*.
If you don't want to install **Go** or are not able to do so, you can find a compiled `zeroconf_lookup` executable in the `Windows_Go` subdirectory.
It was compiled with the standard **Go** compiler version **1.10** using the included `make` batch file.

If you haven't installed **Go** yet, you can find instructions for installing it at https://golang.org/doc/install
Don't forget to add the **GOPATH** and **GOROOT** environment variables and add `go` to your **PATH**

There is a `make.bat` file in the `Windows_Go` subdirectory, so please change into this subdirectory.
`zeroconf_lookup` depends on some external libraries, so please run `make update` to install or update them first. It may take a while.
Then run `make` to build the program. Remember, this is not a *Makefile* but just the `make.bat` batch file.

# Installing on OS X / macOS

Hint: I am an *Apple Newbie* and my only device is a *Mac Mini (early 2011)* running *El Capitan* (I cannot upgrade to macOS right now).

The common setup is to install the executable (`zeroconf_lookup`) either at `/usr/local/bin` or any location writable by the running user.
In any case the *browser manifest* must be installed in a well-known location with a reference to the executable. This is done by calling `zeroconf_lookup -i`

There are two flavors of the executable - one written in C and one in Go.
You may want to check out both and keep the one you like more :-)
The advantage of the **C** version is that returns immediately.
The **Go** version collects data and waits at least one second before returning.

## Installing the **C** version via Makefile only

This is the most basic variant, you will only need `tar`, `cc` and `make`.

1. Change into the *Apple_C* subdirectory:
   * `cd Apple_C`
2. Extract the *tar* archive, replacing the current version number:
   * `tar xvzf zeroconf_lookup-2.2.0.tgz`
3. Change into the extracted subdirectory (again, replace version if necessary):
   * `cd zeroconf_lookup-2.2.0`
4. Compile the executable (a simple `make` should do the trick):
   * `make`
5. Assuming there are no errors (and we are **very** strict), you can install the executable):
   * `make install` or `sudo make install` (which ever you need)

You should have an executable now and can continue with installing the *browser manifest*

## Installing the **C** version via CMake

Essentially, this is just the same procedure as before, but is preferrable if you have **CMake** (duh!) and the **mDNSResponder** libraries and headers are not in obvious locations.
The `CMakeLists.txt` and `cmake/Modules` setup will search for the files and link appropriately.

1. Change into the *Apple_C* subdirectory:
   * `cd Apple_C`
2. Run the *CMake* setup and install the executable:
   * `make install` or `sudo make install` (which ever you need)

You should have an executable now and can continue with installing the *browser manifest*

## Installing the **C** version via CMake and XCode

This flavor is probably only interesting if you want to make changes or if you are used to do all your programming using *XCode*.

1. Change into the *Apple_C* subdirectory:
   * `cd Apple_C`
2. Run the *CMake* setup generating *XCode* project files:
   * `make xcode`
3. Call *XCode* in the usual way (assuming you know what this means :-) and switch to the `_build` subdirectory.
   * `???`

You should be able to generate an executable now and can continue with installing the *browser manifest*

## Installing the **C** version via Homebrew

**coming soon (still learning how to brew)**

## Installing the **Go** version

**coming soon (the code is already in `Apple_Go`)**

## Installing the *browser manifest* (for all flavors and programming languages)

Assuming the `zeroconf_lookup` executable is in your **PATH** now, you can install the *browser manifest* with just calling `zeroconf_lookup -i`.

You may do this as a non-root user, in which case the manifests will be written to the following locations (replace `Admin` with your name):

* `/Users/Admin/Library/Application Support/Mozilla/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`
* `/Users/Admin/Library/Application Support/Google/Chrome/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`
* `/Users/Admin/Library/Application Support/Chromium/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`

You may also run this as root, in which case the locations will be:

* `/Library/Application Support/Mozilla/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`
* `/Library/Google/Chrome/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`
* `/Library/Application Support/Chromium/NativeMessagingHosts/com.railduino.zeroconf_lookup.json`

Please use only one method (you can remove the manifests with `[sudo] zeroconf_lookup -u`)

**That's it - you should be all set up now and the extension should work. If not, see below.**

## Trouble shooting

**coming soon**

# Installing on Linux

**coming soon**

