# LaMouse
This repository serves to add USB mouse capabilities to FortunaOS, for the 'La Fortuna' board (built on the Atmel AT90USB1286).

The code is derived from the LUFA library written by Dean Camera, which can be found at http://www.fourwalledcubicle.com/LUFA.php.

The (ClassDriver) Mouse Demo has been adapted by converting the JoyStick input to use FortunaOS' ruota input, along with various other changes such as extending the Min/Max Axis values in the Descriptors file for more convenient navigation.

# Features
- Mouse navigation using the North, South, East and West switches
- Left-clicking using a short press of the center switch
- Right-clicking using a long press of the center switch
- Additive delta for faster movement when a direction is maintained

# Future Extensions
- Scrolling
- Improve smoothness of navigation (e.g. hold button to maintain motion)


