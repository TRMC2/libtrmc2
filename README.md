# libtrmc2 – Library for controlling the TRMC2 temperature regulator

This library is intended for controlling the [TRMC2 temperature
regulator][TRMC2] from Linux. It can work either on a 32 or 64-bit PC
having a built-in serial port, or on a [Raspberry Pi][] running
[Raspberry Pi OS][].

## Building and installing libtrmc2

You first have to make sure you have a build toolchain installed. The
simplest way is by issuing the command:

    sudo apt install build-essential

On the Raspberry Pi, you will also need the libgpiod library:

    sudo apt install libgpiod-dev

The provided Makefile defaults to installing the shared library in
/usr/local/lib and the header file in /usr/local/include. If you prefer
to install in a different location, you should edit the Makefile and set
the variables `INSTALLDIR`, `LIBDIR` and/or `INCLUDEDIR` accordingly.
Then:

    make
    sudo make install

This last command installs two files and two symbolic links:

    $(INCLUDEDIR)/Trmc.h
    $(LIBDIR)/libtrmc2.so -> libtrmc2.so.2
    $(LIBDIR)/libtrmc2.so.2 -> libtrmc2.so.2.2
    $(LIBDIR)/libtrmc2.so.2.2

## Building your own programs based on libtrmc2

In the source file, put:

    #include <Trmc.h>

Compile with:

    -I$(INCLUDEDIR)

and link with:

    -L$(LIBDIR) -Wl,-rpath,$(LIBDIR) -ltrmc2

where `$(INCLUDEDIR)` and `$(LIBDIR)` are the directories holding Trmc.h
and libtrmc2.so respectively. Then make the program suid root, or start
it with sudo.

## Why run as root?

A program wishing to communicate with the TRMC2 needs low-level access
to the hardware:

* On a PC, the TRMC2 is plugged into a serial port. However, it does not
  use the RS-232 protocol, and therefore we cannot rely on the
  `/dev/ttyS*` devices. Instead, the DTR, RTS and CTS control lines of
  the port are used to bit-bang a custom synchronous serial protocol.
  libtrmc2 uses in/out instructions to access the raw I/O ports and
  needs root privileges to do so.

* On the Raspberry Pi, libtrmc2 uses libgpiod to access the GPIO pins,
  and this also requires root privileges.

* libtrmc2 also uses this privileges to get real-time scheduling
  priority, which helps get good communication timings. This feature can
  be disabled by compiling the library with the option
  `-DNORMAL_SCHEDULE`.

## Copyright

Copyright (C) 2022 CNRS, Institut NEEL Grenoble.

This library was written by Jean-Christian Anglès d’Auriac, initially
for Windows. Edgar Bonet contributed the Linux port (the file
TrmcLin.c).

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
General Public License for more details.

[TRMC2]: http://neel-2007-2019.neel.cnrs.fr/spip.php?article862
[Raspberry Pi]: https://www.raspberrypi.org/products/
[Raspberry Pi OS]: https://www.raspberrypi.org/downloads/raspberry-pi-os/
