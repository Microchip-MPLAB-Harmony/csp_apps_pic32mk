---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: CDAC Waveform Generation
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# DAC waveform generation

This example application shows how to use the CDAC to generate a 5 KHz sinusoidal waveform.

## Description

The CDAC Peripheral library is used to generate a Sine wave. The TMR1 peripheral is configured to trigger CDAC every two microseconds. In this application, the number of DAC samples per cycle (0 to 360 degrees sine wave) is 100.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/cdac/cdac_wav_gen/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_mcj_curiosity_pro.X | MPLABX project for [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113) |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_mcj_curiosity_pro.X | [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113) |
|||

### Setting up [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113)

- Connect an oscilloscope to monitor the DAC1 (RC10) pin
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the application using its IDE
2. Observe a sine wave of 5 KHz frequency on DAC1 output pin