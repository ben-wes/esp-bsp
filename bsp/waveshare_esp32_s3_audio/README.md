# BSP: WAVESHARE ESP32-S3-AUDIO
*(adapted from esp32s3korvo2)*

## ISSUES
1. What to do with LEDs? Is it a good idea to make a component like neopixel? Or just address them as they are? In the board definition they are defined as a strip, not as individual leds.
2. Camera and display are untested (and will probably stay that way).
3. Expansion header is untested.
4. There are two buttons that were present in the korvo2 that are not present in this one. 'bsp_button.c' is modified and has 2 dummy button definitions. 
5. We might need to add more examples in order to debug different features.

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
