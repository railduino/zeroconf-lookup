::
:: Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
::
:: This file is part of Zeroconf-Lookup.
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included in all
:: copies or substantial portions of the Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
:: SOFTWARE.
::
@echo off

IF %1.==cleanup.  GOTO Clean
IF %1.==clean.    GOTO Clean
IF %1.==c.        GOTO Clean
IF %1.==update.   GOTO Update
IF %1.==upd.      GOTO Update
IF %1.==u.        GOTO Update

:: Default is build
go build -v -o zeroconf_lookup.exe
GOTO End

:Clean
del chrome.json
del mozilla.json
del zeroconf_lookup.exe
GOTO End

:Update
echo updating cmp
go get -u github.com/google/go-cmp/cmp
echo updating zeroconf
go get -u github.com/grandcat/zeroconf
echo updating registry
go get -u golang.org/x/sys/windows/registry
GOTO End

:End

