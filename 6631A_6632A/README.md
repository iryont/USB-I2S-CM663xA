### CM6631A / CM6632 USB Audio (1.0 / 2.0 Full Speed & High Speed) to I2S converter

## Master Mode:
* Support up to 384 kHz / 32 bit PCM Audio to I2S output.
* Support of DSD64 and DSD128 via DoP (DSD over PCM) to I2S output.
* SPDIF output generated along with I2S output from a single audio playback.
* Output pins such as 44.1k, P1, P2 and P3 to indicate current frequency.
* MCLK output of either 22.5792 MHz and 24.576 MHz and or 45.1584 MHz and 49.152 MHz.

## Slave Mode (to an FPGA):
* Full support of 768 kHz / 32 bit PCM Audio to I2S output. It should be noted that support for 1536 kHz / 16 bit is there, but is highly experimental.
* Support of DSD64, DSD128, DSD256 in DoP and DSD512 in a native format.
* Serial output for old R-2R DACs such as e.g. PCM56, AD1862, AD1865, PCM58, PCM63, PCM17XX and so on.
* I2S output with a frame of 64 or 32 bits, selectable by a jumper.
* Left-Justified output with a frame of 64 or 32 bits, selectable by a jumper.
* SPDIF output generated from I2S input.
* Each digital output is differential in a way that its data is inverted (e.g. PCM data of +200 to -200).
* MCLK output of 45.1584 MHz or 49.152 MHz.

### Requirements
* C51 compiler From Keil located in ``c:\keil\c51`` (check ``make.bat``).

### How to build
* Run ``build.bat 0921`` from command promp inside ``6631A_6632A`` directory and check ``stdout.txt`` for possible errors.

### License
GPLv3

### More information
* More information can be found on Polish DIY Audio forum:

https://ssl.diyaudio.pl/showthread.php/29314-USB-Audio-forumowe-quot-Amanero-quot
