---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: CORETIMER periodic interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# CORETIMER periodic interrupt

This example application shows how to use the CoreTimer to generate periodic interrupts.

## Description

This example application configures the CoreTimer Peripheral Library to generate periodic interrupts. The application registers a periodic timeout callback. It toggles an LED every time the callback is triggered.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/coretimer/coretimer_periodic_timeout/firmware** .

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

- Connect the Debug USB port (J12) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCJ Curiosity Pro Board

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer

## Running the Application

1. Build and program the application project using its IDE
2. LED Blinks continuosly

The following table provides the details of LED:

| Board | LED Name|
| ---- | ---------|
| [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) | LED1 |
| PIC32MK MCJ Curiosity Pro Board | LED2 |
| PIC32MK MCM Curiosity Pro Board | LED1 |
|||