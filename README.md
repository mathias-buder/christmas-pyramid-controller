[[_TOC_]]

# Introduction
Some time ago, my dad called me to tell me that he is planing to build a giant christmas pyramid that is supposed to be placed in front of the local church at christmas time. The setting up should take place in the form of a Christmas party where everyone comes together and drinks mulled wine.

**What a great idea I thought!**

As the pyramid was to remain standing for the entire Christmas season, a control unit was needed for automatic operation.

# The Christmas Pyramid
As can be seen below, the pyramid contains a number of lamps (230 VAC) as well as a motor (230 VAC) for turning the wings.

<img src='images/christmas-pyramid.gif' width='50%'>

# Control Unit Design
This section is going to describe the control unit design process.

## Requirement Definition
The ECU shall satisfy the following list of requirements:
- Shall have the ability to switch multiple 230 VAC actors (at least 2 individually)
- Shall have to ability to detect motion from humans that move in front of the pyramid
- Shall be able to play music
  - that is stored on an SD Card / USB thumb drive
  - that is in .mp3 and .wav format
  - that is automatically played based on motion sensor input
  - that is outputted through a standard 3.5mm audio jack to be fed to an audio amplifier
- shall have an interface to re-programmed the firmware image
- shall have the ability to be accessed over the air
- shall have the ability to setup tasks in which the user can specify
  - task activation/deactivation time
  - is music playback active/inactive
  - duration of music playback
  - is motion detection active/inactive
  - which AC-actor switch shall be active/inactive
- Shall operate within the temperature range of -20°C - 40°C


## System Architecture


<img src='images/TivaC_Series_TM4C123G_LaunchPad_Evaluation_Board.png' width='80%'>

<img src='images/PYCO-Blockdiagram.png' width='80%'>

<img src='images/pcb_design_model.png' width='80%'>





<img src='images/device_top_view.png' width='80%'>
<img src='images/device_if_view.png' width='80%'>

<img src='images/IMG_1225.jpg' width='80%'>
<img src='images/IMG_1228.jpg' width='80%'>



## Software Architecture

<img src='images/PYCO-SoftwareLayerStack-REVA.png' width='80%'>