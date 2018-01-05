zeroconf-lookup
===============

Browser WebExtension for finding local webservers using Zeroconf

WebExtensions and Zeroconf
--------------------------

In contemporary Chrome and Firefox browsers (as well as in Safari and
others) the Zeroconf discovery has been removed for security reasons. It
seems the only viable alternative approach is using WebExtensions and
NativeMessaging. They work quite similarly in both browsers.

NativeMessaging requires an independent host application. This app needs
to be installed independently from the browser by means of the operating
system. The browser will nevertheless invoke and terminate the app.


