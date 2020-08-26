---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: OCMP compare mode 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# OCMP compare mode

This example application shows how to use the OCMP peripheral to generate an active low, active high, and toggle output on compare match.

## Description

In this application, three OCMP modules are used to generate waveforms.

**Active Low Output**: By default output is set as high and it is set as low on the compare match

**Active High Output**: By default output is set as low and it is set as high on the compare match

**Toggled Output**: Compare match toggles the output

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/ocmp/ocmp_compare_mode/firmware** .

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

- Connect the Debug USB port (J1) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect the Debug USB port (J500) on the board to the computer using a micro USB cable

## Running the Application

1. Build and program the application project using its IDE
2. Observe active low, active high and toggle output on the oscilloscope

    ![ouptut](images/output_ocmp_compare_mode.png)

3. Refer the following table for pin details:

    | OCMP output | [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) pins |  PIC32MK MCJ Curiosity Pro Board pins | PIC32MK MCM Curiosity Pro Board pins |
    |------------- | ---------- | --------------- | ----------------------- |
    | OC1 output | RPA0 "Pin 4 of J29" | RPA0 "Pin 3 of J501"  | RPA0 "Pin 1 of J300"|
    | OC2 output | RPA14 "Pin 5 of J31" | RPA1 "Pin 3 of J502" | RPA14 "Pin 11 of J302" |
    | OC3 output | RPB0 "Pin 8 of J29" | RPB14 "Pin 7 of J501" | RPB0 "Pin 6 of J300" |
    ||||
