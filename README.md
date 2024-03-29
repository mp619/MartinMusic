# Imperial College EEE - ELEC60013 Embedded Systems

## Martin Music - Real-time Music Synthesizer

The team at Martin Music developed software packages for a fully functioning real-time music synthesizer on the StackSynth keyboard which is capable of generating various waveforms
at different octaves, volumes and decay rates. Includes support for performing autodetection upon connection of keyboard modules to one another and the management of their communication protocol via
the shared CAN bus. An intiuitive user interface is provided to allow for the easy interaction and configuration of the synthesizers allowing you to be in control. 

Our team has performed extensive resource and real time system analysis to ensure realiable, safe, and timely functionality of the keyboard modules. 

## Structure

```bash
.
├───.vscode             
├───doc                 # Technical report documenting resource and real-time system analysis
├───include             # Header files for various classes
├───lib                      # Provided libraries
├───src                      # Main file to be uploaded onto synthesizer
├───prev_versions             # Previous versions of the main file
├───test                      # Timing analysis files
├───platformio.ini                   # Platformio IDE init file
```
## Requirements

- [STM32L432KC MCU](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html) 
- Piano attachement
- VS Code - [Platformio](https://platformio.org/)
- C++ Libraries
  - Arduino.h
  - ES_CAN.h
  - platformio - [U8g2lib.h](https://www.arduino.cc/reference/en/libraries/u8g2/)
  - platformio - [STM32FreeRTOS.h](https://www.digikey.co.uk/en/maker/projects/getting-started-with-stm32-introduction-to-freertos/ad275395687e4d85935351e16ec575b1)
  - local - knobs.h
  - local - display.h
  - local - wave.h

## Setup
- For editing and flashing - VS Code with Platformio extension is required
- Clone all contents
- Please follow this [guide](https://hank.feild.org/courses/common/cpp-compiler.html) to install a c++ compiler for all systems
- Install Platformio as a VS code extension
- Goto Platformio Home using ![home](/doc/resources/Home.PNG) on the bottom left of VS code
- Install U8g2 display driver library and STM32duino FreeRTOS library
- Enable CAN module by opening: ~\.platformio\packages\frameworkarduinoststm32\cores\arduino\stm32\stm32yyxx_hal_conf.h
- add **#define HAL_CAN_MODULE_ENABLED**
- Build \src folder using ![BuildIcon](/doc/resources/Build.PNG) on the bottom left
- Connect STM32 to PC using micro USB port
- Upload onto STM32 using ![UploadIcon](/doc/resources/Upload.PNG) on the bottom left

## Links
- [Documentation](https://github.com/mp619/MartinMusic/tree/master/doc)
- [Video](https://web.microsoftstream.com/video/5c28ea65-837a-4dd1-8151-f86ed1e84635)
- [Source Code](https://github.com/mp619/MartinMusic/tree/master/src)

![Intro](/doc/resources/ViedoStart.gif)
