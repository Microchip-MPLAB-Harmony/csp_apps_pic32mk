---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: SPI EEPROM write read 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI EEPROM write read

This example application demonstrates how to use the SPI peripheral to write and read from the SPI serial EEPROM memory.

## Description

This example uses the SPI peripheral library to write an array of values to the SPI Serial EEPROM and verify the value written by reading the values back and comparing it to the value written. The example application performs the EEPROM writing only once after a power on reset.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/spi/spi_eeprom_write_read/firmware** .

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

- To run the application, following additional hardware are required:
  - [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click)

- Plug the [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click) into the MikroBus socket J1 of the development board
- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up PIC32MK MCJ Curiosity Pro Board

- To run the application, following additional hardware are required:
  - [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click)
  - [mikroBus Xplained Pro board](https://www.microchip.com/Developmenttools/ProductDetails/ATMBUSADAPTER-XPRO)

- Connect the [mikroBus Xplained Pro board](https://www.microchip.com/Developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) with [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click) to J501
- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up PIC32MK MCM Curiosity Pro Board

- To run the application, following additional hardware are required:
  - [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click)

- Connect the [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click) to J300
- Connect micro USB cable to the 'USB Debug' connector on the board to the computer

## Running the Application

- Build and program the application using its IDE
- LED indicates the success or failure:
  - LED is turned ON when the value read from the EEPROM matched with the written value
  - LED is turned OFF when the value read from the EEPROM did not match with the written value

Following table provides LED names:

| Board      | LED Name |
| ---------- |--------- |
| [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) | LED2 |
|  PIC32MK MCJ Curiosity Pro Board  | LED2 |
|  PIC32MK MCM Curiosity Pro Board  | LED1  |
||||
