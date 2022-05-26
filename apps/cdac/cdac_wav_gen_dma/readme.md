---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: CDAC Waveform Generation with DMA 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# DAC waveform generation with DMA

This example application shows how to use the CDAC with the DMA to generate a 1 KHz sinusoidal waveform without CPU intervention.

## Description

The CDAC Peripheral library is used with DMA to generate a Sine wave. The TMR1 peripheral is configured to trigger DMA every ten microseconds. DMA is used to setup in Auto mode to transfer sine wave samples from lookup table to the CDAC DATA register. In this application, the number of DAC samples per cycle (0 to 360 degrees sine wave) is 100.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/cdac/cdac_wav_gen_dma/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_mcj_curiosity_pro.X | MPLABX project for [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113) |
| pic32mk_mca_curiosity_pro.X | MPLABX project for PIC32MK MCA Curiosity Pro Board |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_mcj_curiosity_pro.X | [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113) |
| pic32mk_mca_curiosity_pro.X | MPLABX project for PIC32MK MCA Curiosity Pro Board |
|||

### Setting up [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/DT100113)

- Connect an oscilloscope to monitor the DAC1 (RC10) pin (pin 16 of J601)
- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up PIC32MK MCA Curiosity Pro Board

- Connect an oscilloscope to monitor the DAC1 (RA8) pin (Pin 16 of J601)
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the application using its IDE
2. Observe a sine wave of 1 KHz frequency on DAC1 output pin