# zeroconf-lookup

Browser WebExtension for finding local web servers using Zeroconf
(which is called Bonjour in the OS X / macOS world).

Hint: this is still work-in-progress and growing fast.
Please be patient if it does not immediately work out of the box.

## WebExtensions and Zeroconf

In recent Chrome and Firefox browsers (as well as in Safari and others)
the Zeroconf discovery has been removed for security reasons. It seems
that the only viable alternative is using WebExtensions and
NativeMessaging. They work almost identical in both browsers, with
Chrome leading the way and Firefox following suit.

NativeMessaging requires an independent host application. This
application needs to be installed independently from the browser by
means of the operating system. The browser will nevertheless invoke and
terminate this application as needed.

## Purpose of this repository

This repository aims to provide various methods for getting back
Zeroconf into the browser(s). It contains both the standalone host
application (in various flavors) and the source code of the browser extensions.

So far, I have not been able to use connection-based messaging, only connectionless.
A possible hint can be found under
https://discourse.mozilla.org/t/connection-based-native-messaging-doesnt-work-in-popups/17185

By means of a config file it is possible to send **LOG** and **DEBUG** messages.
See below for configuring. Since this logfile will be overwritten by every invocation, it will not grow.

### Configuration

**to be added - call `zeroconf_lookup -h` or check the source code - sorry I'm still working on it**


# Installation

For installing the *native host* program, see the INSTALL.md file in this directory.

## Installation – Firefox and Chrome / Chromium WebExtension

To install the *browser extension* search for **zeroconf** in the appropriate app stores.

For Firefox It can also be installed in debugging mode from the Git repository.
See https://developer.mozilla.org/en-US/Add-ons/WebExtensions/WebExtensions_and_the_Add-on_ID#Basic_workflow_with_no_add-on_ID
for an explanation how to install. The Firefox directory within the
local copy of the repository is the location that needs to be loaded.

The same is possible for Chrome / Chromium
See https://developer.chrome.com/extensions/getstarted#unpacked

# Contributing to this repository

Contributions and comments are welcome.


