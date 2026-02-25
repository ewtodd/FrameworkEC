# Framework Laptop Chromebook Embedded Controller (EC)
<!---->
## This code comes with no warranty, guaranties of functionality, etc.
If you do not have the capability to recover from a bad flash, do not mess with the firmware.
<!---->
This is a fork of the ChromiumOS embedded controller firmware.
It contains only the board banshee, aka the Framework Laptop Chromebook.
This version of the banshee EC is designed to make the keyboard layout standard (and add functionality to function keys) so that the Chromebook can be used with Linux. I use the blank keyboard (and not the transparent version), so the backlight for the keyboard has been completely disabled. Unclear if it is still necessary to blacklist the kernel module now that that is the case.
<!---->
There is a flake.nix file to provide the necessary dependencies to compile the firmware.
It can be compiled easily by running the build-ec.sh script.
<!---->
If you are running this EC firmware and are having trouble with suspend on Linux, try blacklisting the "cros_kbd_led_backlight" kernel module.
