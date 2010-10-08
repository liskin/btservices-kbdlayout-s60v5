# Introduction

Once upon a time, there was a Symbian^1 phone, let me call it Tube, and it was
sad because it did not have a keyboard. It decided to turn on its BlueTooth
receiver and look for a keyboard. Tube was lucky this day and it found an
awesome foldable keyboard, and the two of them became friends. Later that day,
the keyboard found out that there's something weird about Tube — it interprets
the key presses in a strange way, some keys produce different characters than
what's written on them. They were worried that they might be unable to fix
this, as Tube had no idea about its source code and the keyboard was pretty
sure it wasn't its fault. Then they called me…

# What really happened?

Tube is a nickname of Nokia 5800 ExpressMusic, the first Symbian phone with
touchscreen, running Symbian S60v5. Using [the driver for S60v3.2][su8], it
supports any BlueTooth keyboard that supports the HID profile. However, the
layout is fixed and cannot be changed (at least on my 5800 the application
crashes whenever I enter settings). This is an attempt to overcome this
limitation.

# What is this exactly?

This is a slightly modified version of the `btservices` package from
Symbian^3 which compiles on Symbian^1 and produces the `kbdlayout.dll` file
that you can replace on your S60v5 phone and use a customized layout.

In addition to the modifications needed for the DLL to work on S60v5, I added
an Alt+Shift switching layout template and a Czech layout and modified the US
layout to make Caps Lock behave like Escape. This is the default and if you
want something else, you'll have to change it before you compile it. I will be
happy to include your layouts in this repository, don't hesitate and send them
to me!

# How to use it?

Most likely you'll want to change the layout and compile it yourself. To
change the layout, you should know this:

  * `btservices/bluetoothengine/bthid/layouts/group/kbdlayout.mmp` contains a
    list of files to compile. You may want to change this.
  * `btservices/bluetoothengine/bthid/layouts/src/hidlayoutimplproxy.cpp`
    contains a `CMyLayout` class. This specifies the layout that is used.
  * To change Caps Lock back to its original meaning, see the relevant git
    commits.

To compile the DLL, you'll need an SDK for your phone, which is most likely
[here][sdk] (it can be installed on Linux as well, see [the gnupoc
project][gnupoc]). When you have it ready and your environment is set up to
use the SDK, perform the following steps which will compile `kbdlayout.dll`
for you.

  1. chdir to `btservices/bluetoothengine/bthid/dependencies`
  2. `$ bldmake bldfiles`
  3. `$ abld build gcce urel`
  4. chdir to `btservices/bluetoothengine/bthid/group`
  5. `$ bldmake bldfiles`
  6. `$ abld build gcce urel`
  7. copy `…/s60_50/epoc32/release/gcce/urel/kbdlayout.dll` to `C:/sys/bin` in your phone  
     (please note that you'll either need to use RomPatcher to unlock system
     folders, or repackage the sis file with the Wireless Keyboard application
     and sign that using your dev certificate; it is beyond the scope of this
     README to document any of this)

[sdk]: http://www.forum.nokia.com/info/sw.nokia.com/id/ec866fab-4b76-49f6-b5a5-af0631419e9c/S60_All_in_One_SDKs.html
[gnupoc]: http://www.martin.st/symbian/
[su8]: http://europe.nokia.com/support/product-support/nokia-wireless-keyboard-su-8w/software

# Author & Copyright

The original source code is taken from Symbian<sup>3</sup>, that is © Nokia
Corporation, and it is available under the terms of Eclipse Public License
v1.0.

The modifications and new layouts have been done by Tomáš Janoušek
<http://work.lisk.in/> in 2010.
