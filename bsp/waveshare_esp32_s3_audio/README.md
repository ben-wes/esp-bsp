# BSP: WAVESHARE ESP32-S3-AUDIO
*(adapted from esp32s3korvo2)*

## Things that are working
1. Now theres a ws2812 component to take care of the leds
2. Exposed pins seem to work. Three of them go through the tca9555, most of them are duplicates from the display, the other are from usb. Might be worth to disable features like camera, display or touch if there isn't a plan to implement them. Also might be worth to get an adapter from the camera connector to dupont pins to get more exposed pins.

## ISSUES
1. Camera and display are untested (and will probably stay that way).
2. Built-in buttons go through the tca9555, unlike in the korvo2. It's also missing two buttons. Code works but I don't know if it's okay.
3. We might need to add more examples in order to test and debug different features (for now just exposed pins, i2c modules and maybe leds).

## Overview

<table> 
<tr><td>

The Waveshare ESP32-S3-AUDIO is based on the ESP32-S3 with 2.4GHz Wi-Fi and Bluetooth 5 (LE) support, integrates high-capacity Flash and PSRAM, built-in dual microphones, speaker, surround RGB LEDs, onboard multiple interfaces. It enables the rapid development of smart devices such as AI speakers, voice interaction systems, HMI screens and camera applications.

</td><td width="200">
  <img src="doc/waveshare_esp32_s3_audio.jpeg">
</td></tr>
</table>


![image](doc/pic.jpeg)

## Capabilities and dependencies

<div align="center">
<!-- START_DEPENDENCIES -->

1. High-performance MCU: Adopts ESP32-S3R8 module with Xtensa 32-bit LX7 dual-core processor, up to 240MHz main frequency
2. Wireless Connectivity: Supports 2.4GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (LE), with onboard antenna
3. Storage Resources: Integrated 512KB SRAM, 384KB ROM, 8MB PSRAM, and external 16MB Flash memory
4. Voice Interaction: Dual microphone array with noise reduction and echo cancellation, suitable for accurate speech recognition and near/far-field wake-up
5. Clock Management: Integrated PCF85063 RTC chip, supports power-off time retention for alarm, scheduled task, and wake-up functions
6. Colorful Lighting Effects: Onboard 7x surround RGB LEDs, programmable for a variety of dynamic effects
7. HMI Interfaces: Multiple reserved buttons and battery switch for customized function development
8. Expansion Interfaces:
    * SPI LCD display interface (FPC connector / pin header)
    * DVP camera interface (24pin connector)
    * USB, I2C, and some I/O pins (compatible with display interface I/O pins)
9. Multimedia Features: Onboard audio decoding chip, dual microphones and speaker header
10. Storage Expansion: Onboard TF card slot for storing audio files, etc.
11. Power Management: Built-in battery recharge management module, supports multiple power modes and low-power applications

<!-- END_DEPENDENCIES -->
</div>

## Compatible BSP Examples

<div align="center">
<!-- START_EXAMPLES -->

| Example | Description | Try with ESP Launchpad |
| ------- | ----------- | ---------------------- |
| [Audio Example](https://github.com/espressif/esp-bsp/tree/master/examples/audio) | Play and record WAV file | 
<!-- END_EXAMPLES -->
</div>

<!-- START_BENCHMARK -->
<!-- END_BENCHMARK -->
