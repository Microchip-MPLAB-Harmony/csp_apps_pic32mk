---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: ICAP capture mode 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# ICAP capture mode

This example application shows how to use the ICAP peripheral to measure the pulse width of the input signal.

## Description

In this application, a pulse signal is generated using the OCMP peripheral and is fed to the ICAP input. ICAP peripheral captures the time at every edge and displays the pulse width on the serial terminal.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/icap/icap_capture_mode/firmware** .

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

- Connect the OC3 pin RPB9 (Pin 2 of the J31) to the IC1 pin RPC6 (pin 9 of the J30)
- Connect the Debug USB port (J12) on the board to the computer using a micro USB cable
- Connect the USB to UART port (J25) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCJ Curiosity Pro Board

- Connect the IC1 pin RPA7 (Pin 28 of the J601) to the OC3 pin RPB14 (pin 19 of the J601)
- Connect the Debug USB port (J1) on the board to the computer using a micro USB cable

### Setting up PIC32MK MCM Curiosity Pro Board

- Connect the OC3 pin RPB0 (Pin RPB0 of the J300) to the IC1 pin RPB6 (pin RPB6 of the J301)
- Connect the Debug USB port (J500) on the board to the computer using a micro USB cable
- Connect the USB to UART port (J400) on the board to the computer using a micro USB cable

## Running the Application

1. Open the Terminal application (Ex.:Tera term) on the computer
2. Connect to the "USB to UART" COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application project using its IDE
4. Console displays the captured pulse width as shown below:

    ![output](images/output_icap_capture_mode.png)