zeroconf-lookup
===============

Browser WebExtension for finding local web servers using Zeroconf (which
is called Bonjour in the OSX / macOS world).

\[Hint: this is still work-in-progress and growing fast. Please be
patient if it does not immediately work out of the box.\]

WebExtensions and Zeroconf
--------------------------

In recent Chrome and Firefox browsers (as well as in Safari and others)
the Zeroconf discovery has been removed for security reasons. It seems
that the only viable alternative is using WebExtensions and
NativeMessaging. They work almost identical in both browsers, with
Chrome leading the way and Firefox following suit.

NativeMessaging requires an independent host application. This
application needs to be installed independently from the browser by
means of the operating system. The browser will nevertheless invoke and
terminate this application as needed.

Purpose of this repository
--------------------------

This repository aims to provide various methods for getting back
Zeroconf into the browser(s). It contains both the standalone host
application and the browser extension. The initial implementation
features a host program written in C on Linux and an extension running
under Firefox. Since it uses NativeMessaging, it needs Firefox Version
50.0 or newer, including Firefox Quantum.

The C host application
----------------------

### Installation

As of today the host application has only been implemented in C.
Eventually Go and Python might be interesting. It uses Avahi on Linux
(tested with Ubuntu 16.4, CentOS 7.4 and on my Raspberry Pi 3). On OSX
and Windows there are plans to use mDNSResponder.

1.  Download or clone this repository
2.  Change dir into **Host-C** and run **make**
3.  For user installation run **make local** (does not require
    root privileges)
4.  For system wide installation run **sudo make system** (requires
    root privileges)

The steps 3 and 4 also install the manifest used by Firefox (and also
the one for Chrome) in the standard location.

If Avahi ist available, the answers usually come in immediately, because
the **avahi‑daemon** maintains current state. Otherwise the program
waits for 3 seconds and returns all collected answers. So far, I have
not been able to use connection-based messaging, only connectionless. A
possible hint can be found under
<https://discourse.mozilla.org/t/connection-based-native-messaging-doesnt-work-in-popups/17185>

The C program sends Log output to **/tmp/zeroconf\_lookup.log**. By
means of a config file it is possible to also send **DEBUG** messages.
See below for configuring. Since this logfile will be overwritten by
every invocation, it will not grow.

### Configuration

Firefox always calls the application without command line parameters.
There are variables that can be changed, and **zeroconf\_lookup** checks
for them in several standard locations. The first one found is used, so
locations cannot be mixed.

The variables and their default value are:

*\[to be added\]*

Installation – Firefox WebExtension
-----------------------------------

The extension is available for production from
<https://addons.mozilla.org/en-US/firefox/addon/railduino-zeroconf-lookup/>
and has been approved by the Firefox add-ons Team. It can also be
installed in debugging mode from the Git repository. See
<https://developer.mozilla.org/en-US/Add-ons/WebExtensions/WebExtensions_and_the_Add-on_ID#Basic_workflow_with_no_add-on_ID>
for an explanation how to install. The Firefox directory within the
local copy of the repository is the location that needs to be loaded.
The manifest there includes the path that has been setup with the **make
local / make system** command for the host application above.

Contributing to this repository
-------------------------------

Contributions and comments are welcome.


