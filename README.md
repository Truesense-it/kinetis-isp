# kinetis-isp
This is a fork of https://gitlab.com/akrenz/kinetis-isp with added support for boards without FTDI interface.
Such boards normally have a button or a jumper to enforce ISP mode in HW, e.g. Truesense' [UWB Development Kit](https://ultrawideband.treusense.it)

## USAGE
`./nxp-isp -i /dev/ttyUSB0 -d -v --erase FLASH --noftdi -f /Path/to/bin/file.bin`

## TODO
- improve support for higher speeds (currently not working)
 