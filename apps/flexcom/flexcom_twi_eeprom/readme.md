[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FLEXCOM TWI (I2C) EEPROM

This example application shows how to use the flexcom module in TWI mode.

## Description

This application configures the flexcom peripheral in TWI mode to read and write data from an external EEPROM memory chip. Data is written to the EEPROM and then read back and compared. Successful comparison is indicated by turning on an LED.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_g55) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/flexcom/flexcom_twi_eeprom/firmware** .

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

- Connect [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) to EXT1 header
- Plug the [EEPROM 3 click Board](https://www.mikroe.com/eeprom-3-click) into the MikroBus socket of the [mikroBUS Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/ATMBUSADAPTER-XPRO)
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the application project using its IDE
2. LED indicates the success or failure:
    - LED is turned ON when the value read from the EEPROM matched with the written value
    - LED is turned OFF when the value read from the EEPROM did not match with the written value

Following table provides the LED name:

| Board      | LED Name |
| ---------- | ---------------- |--------- |
| [SAM G55 Xplained Pro Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/atsamg55-xpro)  | LED0 |
|||
