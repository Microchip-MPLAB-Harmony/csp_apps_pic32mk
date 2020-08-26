---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: Clock configuration 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# Clock configuration

This example application shows how to configure the clock system to run the device at maximum frequency. It also outputs a prescaled clock signal on a GPIO pin for measurement and verification.

## Description

Clock system generates and distributes the clock for the processor and peripherals. This example application shows how to use the clock manager to configure the device to run at the max possible speed. A prescaled clock signal is routed to GPIO pin to measure the frequency and accuracy of the internal device clock.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/clock/clock_config/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_gp_db.X | MPLABX project for [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
| pic32mk_mcj_curiosity_pro.X | MPLABX project for PIC32MK MCJ Curiosity Pro Board |
| pic32mk_mcm_curiosity_pro.X | MPLABX project for PIC32MK MCM Curiosity Pro Board |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_gp_db.X | [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
| pic32mk_mcj_curiosity_pro.X | PIC32MK MCJ Curiosity Pro Board |
| pic32mk_mcm_curiosity_pro.X | PIC32MK MCM Curiosity Pro Board |
|||

### Setting up [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106)

- Connect an oscilloscope to monitor the PORT pin RPA0 (Pin number #4 on the J29 connector)
- Connect the Debug USB port (J12) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCJ Curiosity Pro Board

- Connect an oscilloscope to monitor the PORT pin RPB11 (Pin #8 on J503/EXT3 connector)
- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect an oscilloscope to monitor the PORT pin RPC1 (Pin number #1 on J301 connector)
- Connect the Debug USB port (J500) on the board to the computer using a micro USB cable


## Running the Application

1. Build and Program the application using its IDE
2. Observe a clock of 4 MHz on the clock output pin
3. LED should be blinking continuosly

Refer to the following table for clock output pin and LED name for different boards:

| Board      | Clock output pin | LED Name |
| ---------- | ---------------- |--------- |
|  [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106)  | RPA0 (Pin number #4 on the J29 connector)  | LED0 |
|  PIC32MK MCJ Curiosity Pro Board  | (Pin #8 on J503/EXT3 connector)  | LED2  |
|  PIC32MK MCM Curiosity Pro Board  | (Pin number #1 on J301 connector)  | LED1  |
||||