---
parent: Harmony 3 peripheral library application examples for SAM G55 family
title: FLEXCOM TWI EEPROM emulation 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FLEXCOM TWI EEPROM emulation

This example application demonstrates how to use the FLEXCOM TWI peripheral in slave mode.

## Description

This example uses the FLEXCOM TWI peripheral library in slave mode and emulates an EEPROM of 512 bytes. There are two pages each of size 256 bytes. TWI slave expects two bytes of memory address from the TWI master and the memory address can range from 0x00 to 0x1FF.
TWI slave application supports following:

**Byte Write**: The TWI master sends the slave address, followed by two bytes of memory address. The slave provides the data present at the requested memory address.

**Page Write**: A page write is initiated the same way as a byte write, but the TWI master can write up-to 256 bytes (1 page). If more than 256 bytes are sent by the TWI master to the TWI slave, the memory address will “roll over” and previous data will be overwritten. The address “roll over” during write is from the last byte of the current page to the first byte of the same page.

**Current Address Read**: The internal memory address counter maintains the last address accessed during the last read or write operation, incremented by one. Once the device address with the read/write bit set to one is clocked in and acknowledged by the EEPROM, the data byte at the current address is serially clocked out. After reading the data from the current address, the TWI master sends NAK and generates a STOP condition.

**Random Read**: The TWI master writes the 2 byte memory address and then reads the data from that memory address. After reading the data, the TWI master sends NAK and generates a STOP condition.

**Sequential Read**: Sequential reads are initiated by either a current address read or a random address read. As long as the EEPROM receives an acknowledge, it will continue to increment the memory address and serially clock out sequential data bytes. When the memory address limit is reached (0x1FF), the memory address will “roll over” to 0x00 and the sequential read will continue. The sequential read operation is terminated when the TWI master sends a NAK and generates a STOP condition.

To run the application, two evaluation kits will be required - one acting as the TWI master and the other as the TWI slave. The TWI master application to be used for this demo application is available under **apps/flexcom/twi/master/flexcom_twi_eeprom/firmware** folder. TWI master writes an array of values to the TWI slave and verifies the value written by reading the values back and comparing it to the value written.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_g55) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/flexcom/twi/slave/flexcom_twi_eeprom_emulation/firmware** .

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

Hardware setup requires two boards, one acting as the TWI Master and the other as the TWI slave.

### Setting up [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro)

- On EXT1 header, connect TWI SDA line on Pin 11 (PB08) and TWI SCL line on Pin 12 (PB09) with the corresponding SDA and SCL lines of the TWI master
- Connect a ground wire between TWI master and TWI slave boards
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and program FLEXCOM TWI EEPROM application from *apps/flexcom/twi/master/flexcom_twi_eeprom/firmware* onto the evaluation kit used as TWI master
2. Build and Program TWI slave application onto the evaluation kit used as TWI slave
3. Run application on TWI slave board and then run the application on TWI master board
4. LED on the TWI master board indicates success or failure:

    - The LED is turned ON when the value read from the TWI slave matches with the written value
    - The LED is turned OFF when the value read from the TWI slave did not match with the written value
