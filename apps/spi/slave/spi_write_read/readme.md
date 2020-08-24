[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI Slave read write

This example application demonstrates how to use the SPI peripheral in slave mode

## Description

This example uses the SPI peripheral library in slave mode and allows reading and writing data from/to its internal buffer by a SPI master. 

The SPI master writes data by sending a write command followed by two bytes of memory address followed by the data to be written.

< WR_CMD > < ADDR_MSB > < ADDR_LSB > < DATA0 > ... < DATA n >

The SPI slave asserts the Busy line to indicate to the SPI master that it is busy. Once ready, the SPI slave de-asserts the Busy line. Once the SPI slave is ready, the SPI master reads the data by sending read command followed by two bytes of memory address and the number of bytes to read.

< RD_CMD > < ADDR_MSB > < ADDR_LSB > < NUM_BYTES >

The SPI slave responds by sending the data at the requested memory address. The SPI master compares the sent data with the received data and repeats the test if it matches.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_pic32mk) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/spi/slave/spi_write_read/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| pic32mk_gp_db.X | MPLABX project for [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| pic32mk_gp_db.X | [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) |
|||

### Setting up [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106)

- To run this demo two [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) are required. One will be progammed as SPI master and other will be programmed as SPI slave.
- Connect Pin 3, 4, 5, 6 of mikroBus header J1 on SPI master to Pin 3, 4, 6, 5 respectively of mikroBus header J1 on SPI slave
    - Pin 3 - SPI Chip Select
    - Pin 4 - SCK
    - Pin 5 - SDO
    - Pin 6 - SDI
- Connect Pin 15 of mikroBus header J1 on SPI master to Pin 16 of mikroBus header J1 on SPI slave which is configured as GPIO and serves as the slave busy pin.
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the SPI master application project using its IDE. Path of the SPI Master application within the repository is **apps/spi/master/spi_write_read_test_app/firmware** 
2. Build and Program the SPI slave application project using its IDE
3. On the SPI master board, press switch to start reading and writing data
2. LED on SPI master board indicates the success or failure:
    - LED is turned ON when the data read from the SPI slave matches with the data written
    - LED is turned OFF when the data read from the SPI slave does not match with the data written

Following table provides the Switch and LED name:

| Board      | Switch Name | LED Name |
| ---------- |--------- | --------- |
| [PIC32MK GP Development Kit](https://www.microchip.com/developmenttools/ProductDetails/dm320106) | S1 | LED1 |
||||