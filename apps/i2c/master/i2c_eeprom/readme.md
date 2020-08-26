---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: I2C EEPROM read write 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# I2C EEPROM read write

This example application demonstrates how to use the I2C peripheral to write and read from the I2C serial EEPROM memory.

## Description

This example uses the I2C peripheral library to write an array of values to the I2C Serial EEPROM and verify the value written by reading the values back and comparing it to the value written.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/i2c/master/i2c_eeprom/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_mcj_curiosity_pro.X | MPLABX project for PIC32MK MCJ Curiosity Pro Board |
| pic32mk_mcm_curiosity_pro.X | MPLABX project for PIC32MK MCM Curiosity Pro Board |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_mcj_curiosity_pro.X | PIC32MK MCJ Curiosity Pro Board |
| pic32mk_mcm_curiosity_pro.X | PIC32MK MCM Curiosity Pro Board |
|||

### Setting up PIC32MK MCJ Curiosity Pro Board

- To run the application, following additional hardware are required:
  - [EEPROM 3 click board](https://www.mikroe.com/eeprom-3-click)
  - [mikroBus Xplained Pro board](https://www.microchip.com/Developmenttools/ProductDetails/ATMBUSADAPTER-XPRO)

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer
- Connect the [mikroBus Xplained Pro board](https://www.microchip.com/Developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) board with [EEPROM 3 click board](https://www.mikroe.com/eeprom-3-click) to J501

### Setting up PIC32MK MCM Curiosity Pro Board

To run the application, following additional hardware are required:
    - [EEPROM 3 click board](https://www.mikroe.com/eeprom-3-click)

Connect micro USB cable to the 'USB Debug' connector on the board to the computer
Connect the [EEPROM 3 click board](https://www.mikroe.com/eeprom-3-click) to J300

## Running the Application

- Build and program the application using its IDE
-LED indicates the success or failure:
  - LED is turned ON when the value read from the EEPROM matched with the written value
  - LED is turned OFF when the value read from the EEPROM did not match with the written value

Following table provides LED names:

| Board      | LED Name |
| ---------- |--------- |
|  PIC32MK MCJ Curiosity Pro Board  | LED2 |
|  PIC32MK MCM Curiosity Pro Board  | LED1  |
||||
