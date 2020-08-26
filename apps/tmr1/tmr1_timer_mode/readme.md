---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: TMR1 periodic interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# TMR1 periodic interrupt

This example demonstrates how to use the TMR1 module in timer mode to generate periodic interrupt.

## Description

The TMR1 module generates 100 ms periodic interrupt. The LED is toggled in the interrupt handler to indicate periodic callback.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/tmr1/tmr1_timer_mode/firmware** .

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

- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up PIC32MK MCJ Curiosity Pro Board

- Connect the Debug USB port (J1) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect the Debug USB port (J500) on the board to the computer using a micro USB cable

## Running the Application

1. Build and program the application project using its IDE
2. On successful execution, LED starts blinking

Following table provides the LED names:

| Board      | LED Name |
| ---------- |--------- |
|  [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106)  | LED1 |
|  PIC32MK MCJ Curiosity Pro Board  | LED2 |
|  PIC32MK MCM Curiosity Pro Board  | LED1  |
||||