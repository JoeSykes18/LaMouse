# LaMouse
This repository adds the LUFA library to FortunaOS (for the 'La Fortuna' board) and demonstrates use of the La Fortuna as a USB mouse.

## LUFA (Lightweight USB Framework for AVRs)
The LUFA library provides a USB framework for AVR microcontrollers. Support for a wide variety of devices from mice to MIDI is included, along with a series of prepared demos.

The library, written by Dean Camera, can be found at http://www.fourwalledcubicle.com/LUFA.php.

## This Project
Adapted from the "ClassLoader Mouse" demo, this project integrates the LUFA library with FortunaOS and allows for the La Fortuna to be used as a (rather crude) USB mouse. 

Current functionality includes:
- Mouse navigation using the North, South, East and West switches
- Left-clicking using a (very) short press of the center switch
- Right-clicking using a (normal/long) press of the center switch

Ideas for extensions:
- Scrolling
- Improve clicking (left clicks frequently mistaken as right clicks, perhaps due to scheduling times in os_add_task)
- Add a timer to reset delta after stationary time, for more natural navigation


