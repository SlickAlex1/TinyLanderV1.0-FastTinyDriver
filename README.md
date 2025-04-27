Tiny Dungeon – FastTinyDriver Edition

This is a fork of the Tiny Lander for the ATtiny85-based TinyJoypad console, updated to use the FastTinyDriver OLED display driver by Daniel C / Electro L.I.B.

Features:

Replaces the classic ssd1306xled display library with FastTinyDriver by Daniel C / Electro L.I.B for direct, high-speed I2C SSD1306 support.
Optimized display routine reduces RAM and flash usage, allowing headroom for new features and improved device compatibility.
Fully compatible with TinyJoypad hardware (ATtiny85 @ 16MHz) and other SSD1306-based microcontroller consoles.

Why this fork? The original Tiny Lander used the ssd1306xled library. This fork upgrades to the modern, optimized FastTinyDriver by Daniel C / Electro L.I.B, improving speed, memory footprint, and hardware compatibility.
