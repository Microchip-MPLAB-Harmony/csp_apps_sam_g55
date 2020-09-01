---
parent: Harmony 3 peripheral library application examples for SAM G55 family
title: CRCCU Compute CRC32 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# CRCCU Compute CRC32

This example demonstrates how to use the CRCCU module to calculate 32 bit CRC value of a data block.

## Description

This application uses CRCCU to compute the CRC value of a predefined data block. Once the computation is done, it compares the output with precomputed CRC value for the same data block. If the comparison is successful, application will light up an LED.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_g55) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/crccu/crccu_crc32_generate/firmware** .

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

- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and program the application project using its IDE
2. The LED indicates the success or failure:
    - The LED is turned ON when the hardware CRC value matched with the software calculated CRC value
    - The LED is turned OFF when the hardware CRC value did not match with the software calculated CRC value

The following table provides the LED name:

| Board| LED |
|------|-----|
| [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro) | LED0 |
|||
