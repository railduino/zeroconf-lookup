zeroconf-lookup
===============

Browser WebExtension for finding local web servers using Zeroconf (which
is called Bonjour in the OSX / macOS world).

WebExtensions and Zeroconf
--------------------------

In contemporary Chrome and Firefox browsers (as well as in Safari and
others) the Zeroconf discovery has been removed for security reasons. It
seems that the only viable alternative is using WebExtensions and
NativeMessaging. They work quite similarly in both browsers, with Chrome
leading the way and Firefox following suit.

NativeMessaging requires an independent host application. This
application needs to be installed independently from the browser by
means of the operating system. The browser will nevertheless invoke and
terminate this application.

Purpose of this repository
--------------------------

This repository aims to provide various methods for getting back
Zeroconf into the browser. It contains both the standalone host
application and the browser extension. The first implementation features
a host program written in C on Linux and an extension running under
Firefox. Since it uses NativeMessaging, it needs Firefox Version 50.0 or
newer, including Firefox Quantum.

TODO
----

There are beginnings and fragments of a host application written in Go
and another one in Python. Maybe another one using NodeJS. Apple OSX (or
macOS), Android, and Windows are next on the list, as well as versions
for Chrome and maybe more. Another topic is test driven development and
host application packaging, which is not yet included.

The C host application
----------------------

### Installation

As of today the host application has only been written in C. It tries to
use Avahi, but will work standalone if it can’t find ***avahi-browse***.
It is completely self contained, i.e. it does not rely on any third
party library. Users need to compile and install it manually.

1.  Download or clone this repository
2.  Change dir into **Host-C** and run ***make***
3.  For user installation run ***make user*** (does not require
    root privileges)
4.  For system wide installation run ***make root*** (requires
    root privileges)

The steps 3 and 4 also install the manifest used by Firefox (and also
the one for Chrome) in the standard location.

If Avahi ist available, the answers usually return immediately, because
the ***avahi‑daemon*** maintains current state. Otherwise the program
waits for 3 seconds and returns all collected answers. So far, I have
not been able to use Connection-based messaging. A possible hint can be
found under
<https://discourse.mozilla.org/t/connection-based-native-messaging-doesnt-work-in-popups/17185>

The C program sends Log output to the ***syslog(3)*** ***USER***
facility, usually only with ***INFO***, ***WARNING***, ***ERROR*** or
***FATAL*** severity. By means of a config file it is possible to send
also ***DEBUG*** messages. See below for configuring.

### Configuration

Firefox always calls the application without command line parameters.
There are three variables that can be changed, and
***zeroconf\_lookup*** checks for them in several standard locations.
The first one found is used, so locations cannot be mixed.

The three variables and their default value are:

> **timeout=3**The timeout if the built-in mDNS-SD discovery is used,
> i.e. Avahi is not found or not used. The number must be in the range
> from 1 to 9.

> **avahi=1**Use Avahi if available and. Disable Avahi browsing by
> setting to 0. A sub-process is forked and **avahi-browse** ist called
> with the following command line:\
> ***avahi-browse --resolve --parsable --no-db-lookup --terminate
> \_http.\_tcp 2&gt;/dev/null***

> **debug=0**Debugging is enabled setting this value to 1.

An example configuration file is included in the repository called
***zeroconf\_lookup.conf.example***. It enabled debugging. The following
locations are checked in descending order:

1.  An environment variable called ***ZEROCONF\_LOOKUP***\
    Example: ***ZEROCONF\_LOOKUP=”avahi=0;debug=1”***
2.  A file called ***zeroconf\_lookup.conf*** in the calling user’s
    ***HOME*** directory.
3.  A file called ***/etc/zeroconf\_lookup.conf***
4.  A file called ***zeroconf\_lookup.conf*** in the directory where the
    executable is found.

Installation – Firefox WebExtension
-----------------------------------

The extension is available for production from
<https://addons.mozilla.org/en-US/firefox/addon/railduino-zeroconf-lookup/>
and has been approved by the Firefox add-ons Team. It can also been
installed in debugging mode from the Git repository. See
<https://developer.mozilla.org/en-US/Add-ons/WebExtensions/WebExtensions_and_the_Add-on_ID#Basic_workflow_with_no_add-on_ID>
for an explanation how to install. The Firefox directory within the
local copy of the repository is the location that needs to be loaded.
The manifest there includes the path that has been setup with the
**make** command for the host application above.

Contributing to this repository
-------------------------------

Contributions and comments are welcome.


