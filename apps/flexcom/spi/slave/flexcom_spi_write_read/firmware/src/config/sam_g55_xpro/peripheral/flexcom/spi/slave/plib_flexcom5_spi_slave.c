/*******************************************************************************
  SPIFLEXCOM5 SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom5_spi_slave.c

  Summary:
    SPIFLEXCOM5 SPI Slave PLIB Implementation File.

  Description:
    This file defines the interface to the FLEXCOM SPI peripheral library.
    This library provides access to and control of the associated
    peripheral instance.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#include "plib_flexcom5_spi_slave.h"
#include "peripheral/pio/plib_pio.h"
#include <string.h>
#include "interrupts.h"

#define FLEXCOM_SPI_RDR_REG      (*(volatile uint8_t* const)((SPI5_BASE_ADDRESS + SPI_RDR_REG_OFST)))
#define FLEXCOM_SPI_TDR_REG      (*(volatile uint8_t* const)((SPI5_BASE_ADDRESS + SPI_TDR_REG_OFST)))

#define FLEXCOM5_READ_BUFFER_SIZE            256U
#define FLEXCOM5_WRITE_BUFFER_SIZE           256U

volatile static uint8_t FLEXCOM5_ReadBuffer[FLEXCOM5_READ_BUFFER_SIZE];
volatile static uint8_t FLEXCOM5_WriteBuffer[FLEXCOM5_WRITE_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: SPIFLEXCOM5 SPI Implementation
// *****************************************************************************
// *****************************************************************************

/* Global object to save FLEXCOM SPI Exchange related data */
volatile static FLEXCOM_SPI_SLAVE_OBJECT flexcom5SpiObj;

static void mem_copy(volatile void* pDst, volatile void* pSrc, uint32_t nBytes)
{
    volatile uint8_t* pSource = (volatile uint8_t*)pSrc;
    volatile uint8_t* pDest = (volatile uint8_t*)pDst;

    for (uint32_t i = 0U; i < nBytes; i++)
    {
        pDest[i] = pSource[i];
    }
}

void FLEXCOM5_SPI_Initialize( void )
{
    /* Set FLEXCOM SPI operating mode */
    FLEXCOM5_REGS->FLEXCOM_MR = FLEXCOM_MR_OPMODE_SPI;

    /* Disable and Reset the SPI*/
    SPI5_REGS->SPI_CR = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;

    /* SPI is by default in Slave Mode, disable mode fault detection */
    SPI5_REGS->SPI_MR = SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width */
    SPI5_REGS->SPI_CSR[0] = SPI_CSR_CPOL(0) | SPI_CSR_NCPHA(1) | SPI_CSR_BITS_8_BIT;

    flexcom5SpiObj.rdInIndex = 0;
    flexcom5SpiObj.wrOutIndex = 0;
    flexcom5SpiObj.nWrBytes = 0;
    flexcom5SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;
    flexcom5SpiObj.callback = NULL ;
    flexcom5SpiObj.transferIsBusy = false ;

    /* Set the Busy Pin to ready state */
    PIO_PinWrite((PIO_PIN)PIO_PIN_PA24, 0);

    /* Enable Receive full and chip deselect interrupt */
    SPI5_REGS->SPI_IER = (SPI_IER_RDRF_Msk | SPI_IER_NSSR_Msk);

    /* Enable SPIFLEXCOM5 */
    SPI5_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM5_SPI_Read(void* pRdBuffer, size_t size)
{
    size_t rdSize = size;
    size_t rdInIndex = flexcom5SpiObj.rdInIndex;

    if (rdSize > rdInIndex)
    {
        rdSize = rdInIndex;
    }

    (void) mem_copy(pRdBuffer, FLEXCOM5_ReadBuffer, rdSize);

    return rdSize;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM5_SPI_Write(void* pWrBuffer, size_t size )
{
    uint32_t intState = SPI5_REGS->SPI_IMR;
    size_t wrSize = size;
    uint32_t wrOutIndex = 0;

    SPI5_REGS->SPI_IDR = intState;

    if (wrSize > (uint32_t)FLEXCOM5_WRITE_BUFFER_SIZE)
    {
        wrSize = FLEXCOM5_WRITE_BUFFER_SIZE;
    }

   (void) mem_copy(FLEXCOM5_WriteBuffer, pWrBuffer, wrSize);

    flexcom5SpiObj.nWrBytes = wrSize;

    while (((SPI5_REGS->SPI_SR & SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < wrSize))
    {
        FLEXCOM_SPI_TDR_REG = FLEXCOM5_WriteBuffer[wrOutIndex];
        wrOutIndex++;
    }

    flexcom5SpiObj.wrOutIndex = wrOutIndex;

    /* Restore interrupt enable state and also enable TDRE interrupt */
    SPI5_REGS->SPI_IER = (intState | SPI_IER_TDRE_Msk);

    return wrSize;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM5_SPI_ReadCountGet(void)
{
    return flexcom5SpiObj.rdInIndex;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM5_SPI_ReadBufferSizeGet(void)
{
    return FLEXCOM5_READ_BUFFER_SIZE;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM5_SPI_WriteBufferSizeGet(void)
{
    return FLEXCOM5_WRITE_BUFFER_SIZE;
}

void FLEXCOM5_SPI_CallbackRegister(FLEXCOM_SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    flexcom5SpiObj.callback = callBack;

    flexcom5SpiObj.context = context;
}

/* The status is returned busy during the period the chip-select remains asserted */
bool FLEXCOM5_SPI_IsBusy(void)
{
    return flexcom5SpiObj.transferIsBusy;
}

/* Drive the GPIO pin to indicate to SPI Master that the slave is ready now */
void FLEXCOM5_SPI_Ready(void)
{
    PIO_PinWrite((PIO_PIN)PIO_PIN_PA24, 0);
}

FLEXCOM_SPI_SLAVE_ERROR FLEXCOM5_SPI_ErrorGet(void)
{
    FLEXCOM_SPI_SLAVE_ERROR errorStatus = flexcom5SpiObj.errorStatus;

    flexcom5SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

void __attribute__((used)) FLEXCOM5_InterruptHandler(void)
{
    uint8_t txRxData = 0U;

    uint32_t statusFlags = SPI5_REGS->SPI_SR;

    if ((statusFlags & SPI_SR_OVRES_Msk) != 0U)
    {
        /*OVRES flag is cleared on reading SPI SR*/

        /* Save the error to report it to application later */
        flexcom5SpiObj.errorStatus = SPI_SR_OVRES_Msk;
    }

    if((statusFlags & SPI_SR_RDRF_Msk) != 0U)
    {
        if (flexcom5SpiObj.transferIsBusy == false)
        {
            flexcom5SpiObj.transferIsBusy = true;

            PIO_PinWrite((PIO_PIN)PIO_PIN_PA24, 1);
        }

        uint32_t rdInIndex = flexcom5SpiObj.rdInIndex;

        while (((statusFlags |= SPI5_REGS->SPI_SR) & SPI_SR_RDRF_Msk) != 0U)
        {
            txRxData = FLEXCOM_SPI_RDR_REG;

            if (rdInIndex < (uint32_t)FLEXCOM5_READ_BUFFER_SIZE)
            {
                FLEXCOM5_ReadBuffer[rdInIndex] = txRxData;
                rdInIndex++;
            }

            statusFlags &= ~SPI_SR_RDRF_Msk;
        }

        flexcom5SpiObj.rdInIndex = rdInIndex;
    }

    if((statusFlags & SPI_SR_TDRE_Msk) != 0U)
    {
        uint32_t wrOutIndex = flexcom5SpiObj.wrOutIndex;
        uint32_t nWrBytes = flexcom5SpiObj.nWrBytes;

        while ((((statusFlags |= SPI5_REGS->SPI_SR) & SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < nWrBytes))
        {
            FLEXCOM_SPI_TDR_REG = FLEXCOM5_WriteBuffer[wrOutIndex];
            wrOutIndex++;
            statusFlags &= ~SPI_SR_TDRE_Msk;
        }

        if (wrOutIndex >= nWrBytes)
        {
            /* Disable TDRE interrupt. The last byte sent by the master will be shifted out automatically */
            SPI5_REGS->SPI_IDR = SPI_IDR_TDRE_Msk;
        }

        flexcom5SpiObj.wrOutIndex = wrOutIndex;
    }

    if((statusFlags & SPI_SR_NSSR_Msk) != 0U)
    {
        /* NSSR flag is cleared on reading SPI SR */

        flexcom5SpiObj.transferIsBusy = false;

        flexcom5SpiObj.wrOutIndex = 0;
        flexcom5SpiObj.nWrBytes = 0;

        if(flexcom5SpiObj.callback != NULL)
        {
            uintptr_t context = flexcom5SpiObj.context;

            flexcom5SpiObj.callback(context);
        }

        /* Clear the rdInIndex. Application must read the received data in the callback. */
        flexcom5SpiObj.rdInIndex = 0;
    }
}