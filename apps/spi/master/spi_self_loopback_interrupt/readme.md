---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: SPI interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI interrupt

This example application demonstrates how to use the SPI peripheral to transmit and receive a block of data using interrupt.

## Description

This example shows the transmit and receive operation over a SPI interface in a non-blocking manner. The peripheral interrupt is used to manage the transfer.The example application transmits known values from an array and verifies the data received through the loop back by comparing the transmitted data and the received data. The example application performs the loop back test only once after a power on reset. After the loop back is complete, the status of the loop back is indicated and the application remains idle. The loop back test requires the MOSI output pin to be connected to the MISO input pin so that anything transmitted will also be received.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/spi/spi_self_loopback_interrupt/firmware** .

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

- Use a jumper wire to short pin MOSI and pin MISO of the J1 connector
- Connect the Debug USB port on the board to the computer using a micro USB cable.

### Setting up PIC32MK MCJ Curiosity Pro Board

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer
- Use a jumper wire to short pin 16 and 17 on J501

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer
- Use a jumper wire to short pin MOSI and pin MISO of J300

## Running the Application

- Build and program the application using its IDE
- LED indicates the success or failure:
  - LED is turned ON when the when the value received matched with the transmitted value
  - LED is turned OFF when the when the value received did not match with the transmitted value

Following table provides LED names:

| Board      | LED Name |
| ---------- |--------- |
| [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) | LED2 |
|  PIC32MK MCJ Curiosity Pro Board  | LED2 |
|  PIC32MK MCM Curiosity Pro Board  | LED1  |
||||
