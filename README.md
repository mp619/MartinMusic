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
