zeroconf-lookup
===============

Browser WebExtension for finding local web servers using Zeroconf

WebExtensions and Zeroconf
--------------------------

In contemporary Chrome and Firefox browsers (as well as in Safari and
others) the Zeroconf discovery has been removed for security reasons. It
seems the only viable alternative approach is using WebExtensions and
NativeMessaging. They work quite similarly in both browsers.

NativeMessaging requires an independent host application. This app needs
to be installed independently from the browser by means of the operating
system. The browser will nevertheless invoke and terminate the
application.

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
and another one in Python. Apple OS X (or macOS), Android, and Windows
are next on the list, as well as versions for Chrome and maybe more.
Another topic is test driven development, which is not yet included.

Installation – host application
-------------------------------

As of today the host application has only been written in C. It is
completely self contained, i.e. it does not rely on any third party
library. Users need to compile and install it from hand.

1.  Download or clone this repository
2.  Change dir into **Host-C-Onetime** and run ***make***
3.  For user installation run ***make user*** (does not require
    root privileges)
4.  For system wide installation run ***make root*** (requires
    root privileges)

The steps 3 and 4 also install the manifest used by Firefox (and also
the one for Chrome) in the standard location.


