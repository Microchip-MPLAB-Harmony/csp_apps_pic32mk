---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: I2C EEPROM read write 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SMBUS read write

This example application demonstrates how to use the I2C peripheral to exchange data between a device acting as a SMBUS master and SMBUS slave.

## Description

This example uses two instances of I2C peripheral. Each instance of I2C is configured as both I2C/SMBUS master and I2C/SMBUS slave. SMBUS master of I2C peripheral instance 1 talks to SMBUS slave of I2C peripheral instance 2. Similarly, SMBUS master of I2C peripheral instance 2 talks to SMBUS slave of I2C peripheral instance 1. The data is exchanged using the standard SMBUS data transfer formats - Write/Read word, Write Bulk, Read Bulk, Write-Read Bulk etc. Each slave always echoes the data it receives from the SMBUS master.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/i2c/smbus_master_slave/smbus_read_write/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_mcm_curiosity_pro.X | MPLABX project for [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A) |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_mcm_curiosity_pro.X | [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A) |
|||

### Setting up [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A)

- Connect a wire from SDA4 on mikroBUS header J300 to SDA2 on mikroBUS header J301
- Connect a wire from SCL4 on mikroBUS header J300 to SCL2 on mikroBUS header J301
- Connect micro USB cable to the 'USB Debug' connector on the board to the computer

## Running the Application

- Build and program the application using its IDE
- LED is turned ON when the data transfer is successful.

Following table provides LED names:

| Board      | LED Name |
| ---------- |--------- |
|  [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A)  | LED1  |
|||
