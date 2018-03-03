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

# A note on trouble shooting in general

After compiling the `zeroconf_lookup` executable (no matter where and how) it is good practice to check that it works.
The first test therefore is running `zeroconf_lookup -h` in the current directory (usually prefixed with `./` under Linux or OS X / macOS).
It will print a usage information and exit.
If that works, try running `zeroconf_lookup -r` which will collect data the same way as in production mode,
but write the *JSON* data on standard output in human readable form (i.e. without the binary length upfront).
This **must** work - otherwise it is useless to continue. Please use the **GitHub** *Issue Tracker* and I will try to help.

Then you can install the executable and the *browser manifest* files as described below. Again check that the setup is working.

# Installing on Windows

The Windows version is a **Go** program, so you should have `go` installed. The minimum version is **1.6** since we are using *context*.
If you don't want to install **Go** or are not able to do so, you can find a compiled `zeroconf_lookup.exe` executable in the `Windows_Go` subdirectory.
It was compiled with the standard **Go** compiler version **1.9.3** using the included `make` batch file.

If you haven't installed **Go** yet, you can find instructions for installing it at https://golang.org/doc/install
Don't forget to add the **GOPATH** and **GOROOT** environment variables and add `go` to your **PATH**

There is a `make.bat` file in the `Windows_Go` subdirectory, so please change into this subdirectory.
`zeroconf_lookup` depends on some external libraries, so please run `make update` to install or update them first. It may take a while.
Then run `make` to build the program. Remember, this is not a *Makefile* but just the `make.bat` batch file.

The resulting executable is self contained and there is just one step left:
creating the Windows Registry keys for finding the *browser manifest* files.
To this end, run `zeroconf_lookup.exe -i` which will create the appropriate manifest files and registry keys.
After doing that, you may not move the executable anywhere else (or if you do, run the command again).
Running `zeroconf_lookup.exe -u` will remove the registry keys again.

When running the `zeroconf_lookup.exe` executable fot the first time on my *Windows 10* computer,
I had to allow the networking in the **Windows Defender Firewall** - you may also have to register with your firewall.

**That's it - you should be all set up now and the extension should work.**

Hint: A NullSoft or WiX installer are probably not worth the while, we have just one executable and two manifest files
(which can reside anywhere) plus two registry keys created by the said executable.

# Installing on OS X / macOS

Hint: I am an *Apple Newbie* and my only device is a *Mac Mini (early 2011)* running *El Capitan* (I cannot upgrade to macOS right now).

The common setup is to install the executable (`zeroconf_lookup`) either at `/usr/local/bin` or any location writable by the current user.
In any case the *browser manifest* must be installed in a well-known location with a reference to the executable.
This is done by calling `zeroconf_lookup -i`

There are two flavors of the executable - one written in C and one in Go.
You may want to check out both and keep the one you like more :-)
The advantage of the **C** version is that returns immediately.
The **Go** version collects data and waits at least one second before returning.

## Installing the **C** version via plain Makefile

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

Essentially, this is just the same procedure as before, but is preferrable
if you have **CMake** (duh!) and the **mDNSResponder** libraries and headers are not in obvious locations.
The `CMakeLists.txt` and `cmake/Modules` will search for the files and link appropriately.

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

Note: I saw this line in the Homebrew *Acceptable Formulae* docs: "We frown on authors submitting their own work unless it is very popular."
I guess this means I am not allowed to submit my own work here. Perhaps I need to wait until it gets popular :-)

## Installing the **Go** version

**coming soon (the code is already in `Apple_Go` - apart from the manifest locations, it is identical to the *Windows* version)**

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

**That's it - you should be all set up now and the extension should work.**

# Installing on Linux

As with OS X / macOS, there are two versions of the program, one written in **C** and one in **Go**.

## Installing the **C** version

All Linux systems I have access to are running the **Avahi** daemon for service discovery.
This includes **Ubuntu** (*16.04 LTS*), **CentOS** (*7.4*) and a **Raspberry Pi 3** (*Raspbian Stretch*).
Others should compile and work, too.

There is a **GNU** *AutoTools* compatible build pipeline with `./configure`, `make` and `sudo make install`.
Only **compatible** - it requires that the `avahi-client` and `avahi-common` libraries are in the linker path.

** more to follow - including the generation of *DEB* and *RPM* packages through the *Ruby* based *FPM* program.**

