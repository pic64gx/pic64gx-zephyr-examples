.. zephyr:code-sample:: amp_example_openamp
   :name: OpenAMP using resource table
   :relevant-api: ipm_interface

   Send messages between two cores using OpenAMP and a resource table.

Overview
********

This application demonstrates how to use OpenAMP with Zephyr based on a resource
table. It is designed to respond to the:

* Linux rpmsg client sample
* Linux rpmsg tty driver
* Linux rpmsg custom pingpong client demo.

Please refer to PIC64GX exemples specific to AMP on Linux :
<https://github.com/pic64gx/pic64gx-linux-examples/tree/main/amp>

This sample implementation is compatible with platforms that embed
a Linux kernel OS on the one or multiple harts and a Zephyr application on one
harts. Both Linux and Zephyr use the same type of hart reference as U54

Building the application
************************

You might need to set the following udev rules to access the serial port
in /etc/udev/rules.d/70-microchip.rules assuming your using a Linux host.

.. code-block:: console
   # Update right for flashpro 6
   ACTION=="add", ATTRS{idVendor}=="1514", ATTRS{idProduct}=="200b", \
   GROUP="dialout" MODE="0666"

   # Bind ftdi_sio driver to all input
   ACTION=="add", ATTRS{idVendor}=="1514", ATTRS{idProduct}=="2008", \
   ATTRS{product}=="Embedded FlashPro5", ATTR{bInterfaceNumber}!="00", \
   RUN+="/sbin/modprobe ftdi_sio", RUN+="/bin/sh -c 'echo 1514 2008 > /sys/bus/usb-serial/drivers/ftdi_sio/new_id'"

   # Unbind ftdi_sio driver for channel A which should be the JTAG
   SUBSYSTEM=="usb", DRIVER=="ftdi_sio", ATTR{bInterfaceNumber}=="00", ATTR{interface}=="Embedded FlashPro5",\
   RUN+="/bin/sh -c 'echo $kernel > /sys/bus/usb/drivers/ftdi_sio/unbind'"

   # Helper (optional)
   KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
   ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="01", \
   SYMLINK+="ttyUSB-FlashPro5B" GROUP="dialout" MODE="0666"

   KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
   ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="02", \
   SYMLINK+="ttyUSB-FlashPro5C" GROUP="dialout" MODE="0666"

   KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
   ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="03", \
   SYMLINK+="ttyUSB-FlashPro5D" GROUP="dialout" MODE="0666"

Zephyr
======

.. zephyr-app-commands::
   :zephyr-app: apps/amp_example_openamp
   :goals: test

Running the client sample
*************************

Linux setup
===========

Assuming the PIC64GX Yocto or Buildroot repository have been used to build Linux.
And the AMP target have been selected. The firmware is built-in the Linux image.

Zephyr setup
============

Open a serial terminal (minicom, putty, etc.) and connect to the board using default serial port settings.

Linux console
=============

Open a Linux shell (minicom, ssh, etc.)
By default the Linux build should be already setup for that example
When the sample echo demo is selected on the zephyr side, the Linux side will be notified
and output the following logs

.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~#
   [ 7346.113472] virtio_rpmsg_bus virtio0: creating chan0
   [ 7346.148442] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: new channel: 0x401 -> 0x400!
   [ 7346.166510] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 1 (src: 0x400)
   [ 7346.182632] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 2 (src: 0x400)
   [ 7346.198677] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 3 (src: 0x400)
   ...
   [ 7347.740754] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 98 (src: 0x400)
   [ 7347.757003] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 99 (src: 0x400)
   [ 7347.773426] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: incoming msg 100 (src: 0x400)
   [ 7347.782476] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: goodbye!
   [ 7347.789719] virtio_rpmsg_bus virtio0: destroying channel rpmsg-client-sample addr 0x400
   [ 7347.797954] rpmsg_client_sample virtio0.rpmsg-client-sample.-1.1024: rpmsg sample client driver is removed

Zephyr console
==============

First the firmware boot, the option run ping pong demo must be selected
by pressing 3 than enter

.. code-block:: console
      *** Booting Zephyr OS build v#.#.#-####-g########## ***



   **** PIC64GX SoC Curiosity Kit AMP RPMsg Remote Zephyr Example ****



   Press 0 to show this menu
   Press 1 to run ping pong demo
   Press 2 to run console demo
   Press 3 to run sample echo demo

Once the demo is launched, the firmware will create a rpmsg channel and send messages
.. code-block:: console
   Sending name service announcement

   Please run sample echo demo on the RPMsg master context

   received message 1: hello world! of size 12
   rpmsg sample test: message 1 sent
   received message 2: hello world! of size 12
   rpmsg sample test: message 2 sent
   received message 3: hello world! of size 12
   rpmsg sample test: message 3 sent
   received message 4: hello world! of size 12
   rpmsg sample test: message 4 sent
   ...
   received message 99: hello world! of size 12
   rpmsg sample test: message 99 sent
   received message 100: hello world! of size 12
   rpmsg sample test: message 100 sent
   **********************************
   Test Results: Error count = 0
   **********************************

   End of sample echo demo. Press 0 to show menu

Running the rpmsg TTY demo
**************************

Linux setup
===========

Assuming the PIC64GX Yocto or Buildroot repository have been used to build Linux.
And the AMP target have been selected. The firmware is built-in the Linux image.

Zephyr setup
============

Open a serial terminal (minicom, putty, etc.) and connect to the board using default serial port settings.

Linux console
=============

Open a Linux shell (minicom, ssh, etc.):

Assuming the remote firmware has not been launch by the bootloader, the remote firmware must be launched manually.

.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# echo start > /sys/class/remoteproc/remoteproc0/state
   [   62.682876] remoteproc remoteproc0: powering up remote-context
   [   62.838085] remoteproc remoteproc0: Booting fw image rproc-remote-context-fw, size 897088
   [   62.881933] virtio_rpmsg_bus virtio0: rpmsg host is online
   [   62.887688] rproc-virtio rproc-virtio.1.auto: registered virtio0 (type 7)

.. code-block:: console
   [ 6905.895513] virtio_rpmsg_bus virtio0: creating channel rpmsg-tty addr 0x400

.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# cd /opt/microchip/amp/rpmsg-tty-example/
   root@pic64gx-curiosity-kit-amp:/opt/microchip/amp/rpmsg-tty-example# ./rpmsg-tty
   Opening device /dev/ttyRPMSG0...
   Device is open
   Enter message to send or type quit to quit :test
   foo
   quit
   [ 6933.532501] virtio_rpmsg_bus virtio0: destroying channel rpmsg-tty addr 0x400


Zephyr console
==============

On the Zephyr console, the received message is displayed as shown below, then sent back to Linux.

First the firmware boot, the option run ping pong demo must be selected
by pressing 2 than enter

.. code-block:: console
      *** Booting Zephyr OS build v#.#.#-####-g########## ***



   **** PIC64GX SoC Curiosity Kit AMP RPMsg Remote Zephyr Example ****



   Press 0 to show this menu
   Press 1 to run ping pong demo
   Press 2 to run console demo
   Press 3 to run sample echo demo

.. code-block:: console
   Sending name service announcement

   Please run console demo on the RPMsg master context

   test
   foo
   quit

   End of console/tty demo. Press 0 to show menu

Running the pingpong demo
*******************************

Linux setup
===========

Assuming the PIC64GX Yocto or Buildroot repository have been used to build Linux.
And the AMP target have been selected. The firmware is built-in the Linux image.

Zephyr setup
============

Open a serial terminal (minicom, putty, etc.) and connect to the board using default serial port settings.

Linux console
=============

Assuming the remote firmware has not been launch by the bootloader, the remote firmware must be launched manually.
.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# echo start > /sys/class/remoteproc/remoteproc0/state
   [   62.682876] remoteproc remoteproc0: powering up remote-context
   [   62.838085] remoteproc remoteproc0: Booting fw image rproc-remote-context-fw, size 897088
   [   62.881933] virtio_rpmsg_bus virtio0: rpmsg host is online
   [   62.887688] rproc-virtio rproc-virtio.1.auto: registered virtio0 (type 7)
   [   62.894692] remoteproc remoteproc0: remote processor remote-context is now up


In any case the rpmsg-pingpong demo must be launched on the Linux side. Note that the rpmsg-pingpong
demo must have been selected on the zephyr side first.

Upon launching of the demo on zephyr side, a rpmsg channel is created and the
Linux side is notified.
.. code-block:: console
   [   68.943534] virtio_rpmsg_bus virtio0: creating channel rpmsg-amp-demo-channel addr 0x0

Then the demo can be launch on the Linux side.

.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# cd /opt/microchip/amp/rpmsg-pingpong/
   root@pic64gx-curiosity-kit-amp:/opt/microchip/amp/rpmsg-pingpong# ./rpmsg-pingpong

   Echo test start
   No dev file for rpmsg-openamp-demo-channel in /sys/bus/rpmsg/devices
   checking /sys/class/rpmsg/rpmsg_ctrl0/rpmsg0/name
   checking /sys/class/rpmsg/rpmsg_ctrl0/rpmsg1/name
   svc_name: rpmsg-openamp-demo-channel
   .
   open /dev/rpmsg1

   **************************************

   Echo Test Round 0

   **************************************

   sending payload number 0 of size 17
   echo test: sent : 17
   received payload number 0 of size 17

   sending payload number 1 of size 18
   echo test: sent : 18
   received payload number 1 of size 18

   sending payload number 2 of size 19
   echo test: sent : 19
   received payload number 2 of size 19

   ...

   sending payload number 214 of size 231
   echo test: sent : 231
   received payload number 214 of size 231

   sending payload number 215 of size 232
   echo test: sent : 232
   received payload number 215 of size 232

   **************************************

   Echo Test Round 0 Test Results: Error count = 0

   **************************************

Zephyr console
==============

First the firmware boot, the option run ping pong demo must be selected
by pressing 1 than enter

.. code-block:: console
      *** Booting Zephyr OS build v#.#.#-####-g########## ***



   **** PIC64GX SoC Curiosity Kit AMP RPMsg Remote Zephyr Example ****



   Press 0 to show this menu
   Press 1 to run ping pong demo
   Press 2 to run console demo
   Press 3 to run sample echo demo

Once the exemple is selected, the ping pong demo is launched and the firmware
is waiting for the Linux side to send the first message.
.. code-block:: console

   Sending name service announcement

   Please run pingpong demo on the RPMsg master context

   received payload number 0 of size 17
   received payload number 1 of size 18
   received payload number 2 of size 19
   received payload number 3 of size 20
   ...
   received payload number 211 of size 228
   received payload number 212 of size 229
   received payload number 213 of size 230
   received payload number 214 of size 231
   received payload number 215 of size 232

   End of ping pong demo. Press 0 to show menu


Loading the application
************************
By default a working firmware is already included in the Linux image.
But in case you want to load a new firmware without rebuilding the Linux image,

The firmware is located there :
/lib/firmware/rproc-remote-context-fw

and the generated elf file resulting from the build of that example should be
copied there and renamed to rproc-remote-context-fw

The firmware can be loaded using the remoteproc framework.
.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# echo start > /sys/class/remoteproc/remoteproc0/state
   [   62.682876] remoteproc remoteproc0: powering up remote-context
   [   62.838085] remoteproc remoteproc0: Booting fw image rproc-remote-context-fw, size 897088
   [   62.881933] virtio_rpmsg_bus virtio0: rpmsg host is online
   [   62.887688] rproc-virtio rproc-virtio.1.auto: registered virtio0 (type 7)
   [   62.894692] remoteproc remoteproc0: remote processor remote-context is now up

And stopped using:
.. code-block:: console
   root@pic64gx-curiosity-kit-amp:~# echo stop > /sys/class/remoteproc/remoteproc0/state
   [ 8946.864741] remoteproc remoteproc0: stopped remote processor remote-context