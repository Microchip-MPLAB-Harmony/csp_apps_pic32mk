---
parent: Harmony 3 peripheral library application examples for PIC32MK family
title: RTCC alarm interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# RTCC alarm interrupt

This example application shows how to use the RTCC to configure the time and generate the alarm.

## Description

This example application shows how to setup RTCC time and configure alarm using the RTCC Peripheral Library. The application sets up an alarm to be generated every day at a specified time. A message is sent via the Virtual COM port on the alarm trigger.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/rtcc/rtcc_alarm/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_gp_db.X | MPLABX project for [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
| pic32mk_mcj_curiosity_pro.X | MPLABX project for [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/DT100113) |
| pic32mk_mcm_curiosity_pro.X | MPLABX project for [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A) |
| pic32mk_mca_curiosity_pro.X | MPLABX project for [PIC32MK MCA Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV15D86A) |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_gp_db.X | [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
| pic32mk_mcj_curiosity_pro.X | [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/DT100113) |
| pic32mk_mcm_curiosity_pro.X | [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A) |
| pic32mk_mca_curiosity_pro.X | [PIC32MK MCA Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV15D86A) |
|||

### Setting up [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106)

- Connect micro USB cables from the J12(Debug USB) and J25(USB-to-UART) connectors on the board to the computer

### Setting up [PIC32MK MCJ Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/DT100113)

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer

### Setting up [PIC32MK MCM Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV31E34A)

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer
- Connect the USB to UART port (J400) on the board to the computer using a micro USB cable

### Setting up [PIC32MK MCA Curiosity Pro Board](https://www.microchip.com/en-us/development-tool/EV15D86A)

- Connect micro USB cable to the 'USB Debug' connector on the board to the computer


## Running the Application

1. Open the Terminal application (Ex.:Tera term) on the computer
2. Connect to the "USB to UART" COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application project using its IDE
4. Console output will be as given below in the beginning:

    ![initial](images/rtcc_alarm_initial.png)
    
5. Once alarm triggers, following will be console output:

    ![output](images/rtcc_alarm_output.png)