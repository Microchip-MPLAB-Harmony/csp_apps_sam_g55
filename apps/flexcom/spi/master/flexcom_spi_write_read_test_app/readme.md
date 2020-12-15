---
parent: Harmony 3 peripheral library application examples for SAM G55 family
title: FLEXCOM SPI Master read write test application 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FLEXCOM SPI Master read write test application

This is a FLEXCOM SPI Master test application which is provided to demonstrate communication between SPI master and the corresponding SPI slave application available under -  apps/flexcom/spi/slave/spi_write_read/firmware 

## Description

This example uses the FLEXCOM SPI peripheral library in master mode and serves as a test application to demonstrate communication between SPI master and corresponding SPI slave application available at apps/flexcom/spi/slave/spi_write_read/firmware.

The SPI master writes data by sending a write command followed by two bytes of memory address followed by the data to be written.

< WR_CMD > < ADDR_MSB > < ADDR_LSB > < DATA0 > ... < DATA n >

The SPI slave asserts the Busy line to indicate to the SPI master that it is busy. Once ready, the SPI slave de-asserts the Busy line. Once the SPI slave is ready, the SPI master reads the data by sending read command followed by two bytes of memory address and the number of bytes to read.

< RD_CMD > < ADDR_MSB > < ADDR_LSB > < NUM_BYTES >

The SPI slave responds by sending the data at the requested memory address. The SPI master compares the sent data with the received data and repeats the test if it matches.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_g55) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/flexcom/spi/master/spi_write_read_test_app/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_g55_xpro.X | MPLABX project for [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro) |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_g55_xpro.X | [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro)
|||

### Setting up [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro)

- To run this demo two [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro) are required. One will be progammed as SPI master and other will be programmed as SPI slave.
- Connect Pin 15, 16, 17, 18 on EXT1 of SPI slave to Pin 15, 16, 17, 18 on EXT1 of SPI master 
    - Pin 15 - SPI Chip Select
    - Pin 16 - MOSI
    - Pin 17 - MISO
    - Pin 18 - SCK
- Connect Pin 9 on EXT1 of SPI slave to Pin 9 on EXT1 of SPI master. Pin 9 is configured as GPIO and serves as the slave busy pin.
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the SPI master application project using its IDE
2. Build and Program the SPI slave application project using its IDE. Path of the SPI Slave application within the repository is **apps/flexcom/spi/slave/spi_write_read/firmware** 
3. On the SPI master board, press switch to start reading and writing data
2. LED on SPI master board indicates the success or failure:
    - LED is turned ON when the data read from the SPI slave matches with the data written
    - LED is turned OFF when the data read from the SPI slave does not match with the data written

Following table provides the LED and Switch name:

| Board      | Switch Name| LED Name |
| ---------- | ---------------- | ---------------- |
| [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro)  | SW0 | LED0 |
|||
