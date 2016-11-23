The default COM port is /dev/ttyS0.  To change the COM port, you can set the SIVOICE_SPI_IF variable to point to the correct USB device. For example:

export SIVOICE_SPI_IF=/dev/ttyUSB0

The CP210x VCP driver is assumed to be already part of your kernel and user permissions are set to access the device.  Please refer to your Linux distribution documentation on how to do this.

Note: Linux is a registered trademark of Linus Torvalds.
