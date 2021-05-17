# libtrmc2 – Library for controlling the TRMC2 temperature regulator

This library is intended for controlling the [TRMC2 temperature
regulator][TRMC2] from Linux. It can work either on a 32 or 64-bit PC
having a built-in serial port, or on a [Raspberry Pi][] running
[Raspberry Pi OS][].

The library was written by Jean-Christian Anglès d’Auriac, initially for
Windows. Edgar Bonet contributed the Linux port (the file TrmcLin.c),
save for the code specific to the Raspberry Pi.

## Building and installing libtrmc2

You first have to make sure you have a build toolchain installed. The
simplest way is by issuing the command:

    sudo apt install build-essential

On the Raspberry Pi, you will also need the latest version of the
[wiringPi][] library. Beware that Raspbian provides wiringPi 2.50, which
is an outdated version that will not work on the Raspberry Pi&nbsp;4. If
you have that version installed, you first have to remove it:

    version=$(dpkg-query -Wf '${Version}' wiringpi)
    if [ "$version" = "2.50" ]; then sudo apt -y purge wiringpi; fi

Then, install wiringPi from the author's package:

    curl -O https://project-downloads.drogon.net/wiringpi-latest.deb
    sudo dpkg -i wiringpi-latest.deb

The provided Makefile defaults to installing the shared library in
/usr/local/lib and the header file in /usr/local/include. If you prefer
to install in a different location, you should edit the Makefile and set
the variables `INSTALLDIR`, `LIBDIR` and/or `INCLUDEDIR` accordingly.
Then:

    make
    sudo make install

This last command installs two files and two symbolic links:

    $(INCLUDEDIR)/Trmc.h
    $(LIBDIR)/libtrmc2.so -> libtrmc2.so.1
    $(LIBDIR)/libtrmc2.so.1 -> libtrmc2.so.1.0
    $(LIBDIR)/libtrmc2.so.1.0

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

* On the Raspberry Pi, libtrmc2 uses the wiringPi library to access the
  GPIO pins. By default, wiringPi will attempt an `mmap()` on /dev/mem
  in order to gain access to the GPIO ports, which also requires root
  privileges.

* libtrmc2 also uses this privileges to get real-time scheduling
  priority, which helps get good communication timings. This feature can
  be disabled by compiling the library with the option
  `-DNORMAL_SCHEDULE`.

[TRMC2]: http://neel-2007-2019.neel.cnrs.fr/spip.php?article862
[Raspberry Pi]: https://www.raspberrypi.org/products/
[Raspberry Pi OS]: https://www.raspberrypi.org/downloads/raspberry-pi-os/
[wiringPi]: http://wiringpi.com/
