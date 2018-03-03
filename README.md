# zeroconf-lookup

Browser WebExtension for finding local web servers using Zeroconf
(which is called Bonjour in the OSX / macOS world).

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
application (in various flavors) and the browser extension.

So far, I have not been able to use connection-based messaging, only connectionless.
A possible hint can be found under
https://discourse.mozilla.org/t/connection-based-native-messaging-doesnt-work-in-popups/17185

The programs under OS X / macOS and Linux  send Log output to **/tmp/zeroconf\_lookup.log**.
By means of a config file it is possible to also send **DEBUG** messages.
See below for configuring. Since this logfile will be overwritten by every invocation, it will not grow.

### Configuration

Firefox and Chrome / Chromium always call the application without command line parameters.
There are variables that can be changed, and **zeroconf\_lookup** checks
for them in several standard locations. The first one found is used, so
locations cannot be mixed.

The variables and their default value are:

*\[to be added\]*


# Installation

For installing the *native host* program, see the INSTALL.md file in this directory.

## Installation – Firefox WebExtension

The extension is available for production from
https://addons.mozilla.org/en-US/firefox/addon/railduino-zeroconf-lookup/
and has been approved by the Firefox add-ons Team.

It can also be installed in debugging mode from the Git repository.
See https://developer.mozilla.org/en-US/Add-ons/WebExtensions/WebExtensions_and_the_Add-on_ID#Basic_workflow_with_no_add-on_ID
for an explanation how to install. The Firefox directory within the
local copy of the repository is the location that needs to be loaded.

# Contributing to this repository

Contributions and comments are welcome.


