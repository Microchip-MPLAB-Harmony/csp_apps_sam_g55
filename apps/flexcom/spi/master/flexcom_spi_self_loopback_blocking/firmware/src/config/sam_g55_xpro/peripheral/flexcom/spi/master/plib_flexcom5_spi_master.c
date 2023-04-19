/*******************************************************************************
  FLEXCOM5 SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom5_spi_master.c

  Summary:
    FLEXCOM5 SPI Master PLIB Implementation File.

  Description:
    This file defines the interface to the FLEXCOM SPI Master peripheral library.
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

#include "plib_flexcom5_spi_master.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM5 SPI Implementation
// *****************************************************************************
// *****************************************************************************

void FLEXCOM5_SPI_Initialize( void )
{
    /* Set FLEXCOM SPI operating mode */
    FLEXCOM5_REGS->FLEXCOM_MR = FLEXCOM_MR_OPMODE_SPI;

    /* Disable and Reset the FLEXCOM SPI */
    SPI5_REGS->SPI_CR = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;

    /* Enable Master mode, select clock source, select particular NPCS line for chip select and disable mode fault detection */
    SPI5_REGS->SPI_MR = SPI_MR_MSTR_Msk | SPI_MR_BRSRCCLK_PERIPH_CLK | SPI_MR_DLYBCS(0) | SPI_MR_PCS(FLEXCOM_SPI_CHIP_SELECT_NPCS0) | SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width, Baud Rate */
    SPI5_REGS->SPI_CSR[0] = SPI_CSR_CPOL(0) | SPI_CSR_NCPHA(1) | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(100)| SPI_CSR_DLYBS(0) | SPI_CSR_DLYBCT(0)  | SPI_CSR_CSAAT_Msk ;






    /* Enable FLEXCOM5 SPI */
    SPI5_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
}


bool FLEXCOM5_SPI_WriteRead( void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize )
{
    size_t txCount = 0;
    size_t rxCount = 0;
    size_t dummySize = 0;
    size_t receivedData = 0;
    uint32_t dataBits = 0U;
    bool isSuccess = false;

    /* Verify the request */
    if (((txSize > 0U) && (pTransmitData != NULL)) || ((rxSize > 0U) && (pReceiveData != NULL)))
    {
        if (pTransmitData == NULL)
        {
            txSize = 0;
        }
        if (pReceiveData == NULL)
        {
            rxSize = 0;
        }

        dataBits = SPI5_REGS->SPI_CSR[0] & SPI_CSR_BITS_Msk;

        /* Flush out any unread data in SPI read buffer from the previous transfer */
        receivedData = (SPI5_REGS->SPI_RDR & SPI_RDR_RD_Msk) >> SPI_RDR_RD_Pos;

        if (rxSize > txSize)
        {
            dummySize = rxSize - txSize;
        }
        if (dataBits != SPI_CSR_BITS_8_BIT)
        {
            rxSize >>= 1;
            txSize >>= 1;
            dummySize >>= 1;
        }

        /* Make sure TDR is empty */
        while((SPI5_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U)
        {
            /* Wait for TDRE flag to rise */
        }

        while ((txCount != txSize) || (dummySize != 0U))
        {
            if (txCount != txSize)
            {
                if(dataBits == SPI_CSR_BITS_8_BIT)
                {
                    SPI5_REGS->SPI_TDR = ((uint8_t*)pTransmitData)[txCount++];
                }
                else
                {
                    SPI5_REGS->SPI_TDR = ((uint16_t*)pTransmitData)[txCount++];
                }
            }
            else if (dummySize > 0U)
            {
                SPI5_REGS->SPI_TDR = 0xff;
                dummySize--;
            }
            else
            {
                /* Do nothing */
            }

            if (rxSize == 0U)
            {
                /* For transmit only request, wait for TDR to become empty */
                while((SPI5_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U)
                {
                   /* Wait for TDRE flag to rise */
                }
            }
            else
            {
                /* If data is read, wait for the Receiver Data Register to become full*/
                while((SPI5_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U)
                {
                    /* Wait for RDRF flag to rise */
                }

                receivedData = (SPI5_REGS->SPI_RDR & SPI_RDR_RD_Msk) >> SPI_RDR_RD_Pos;

                if (rxCount < rxSize)
                {
                    if(dataBits == SPI_CSR_BITS_8_BIT)
                    {
                        ((uint8_t*)pReceiveData)[rxCount++] = (uint8_t)receivedData;
                    }
                    else
                    {
                        ((uint16_t*)pReceiveData)[rxCount++] = (uint16_t)receivedData;
                    }
                }
            }
        }

        /* Make sure no data is pending in the shift register */
        while ((SPI5_REGS->SPI_SR & SPI_SR_TXEMPTY_Msk) == 0U)
        {
            /* Wait for TXEMPTY flag to rise */
        }

        /* Set Last transfer to deassert NPCS after the last byte written in TDR has been transferred. */
        SPI5_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;

        isSuccess = true;
    }

    return isSuccess;
}

bool FLEXCOM5_SPI_TransferSetup( FLEXCOM_SPI_TRANSFER_SETUP * setup, uint32_t spiSourceClock )
{
    uint32_t scbr;

    if ((setup == NULL) || (setup->clockFrequency == 0U))
    {
        return false;
    }
    if(spiSourceClock == 0U)
    {
        // Fetch Master Clock Frequency directly
        spiSourceClock = 100007936;
    }

    scbr = spiSourceClock/setup->clockFrequency;

    if(scbr == 0U)
    {
        scbr = 1U;
    }
    else if(scbr > 255U)
    {
        scbr = 255U;
    }
    else
    {
        /* Do Nothing */
    }

    SPI5_REGS->SPI_CSR[0] = (SPI5_REGS->SPI_CSR[0] & ~(SPI_CSR_CPOL_Msk | SPI_CSR_NCPHA_Msk | SPI_CSR_BITS_Msk | SPI_CSR_SCBR_Msk)) | ((uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase | (uint32_t)setup->dataBits | SPI_CSR_SCBR(scbr));

    return true;
}

bool FLEXCOM5_SPI_Write( void* pTransmitData, size_t txSize )
{
    return (FLEXCOM5_SPI_WriteRead(pTransmitData, txSize, NULL, 0));
}

bool FLEXCOM5_SPI_Read( void* pReceiveData, size_t rxSize )
{
    return (FLEXCOM5_SPI_WriteRead(NULL, 0, pReceiveData, rxSize));
}

bool FLEXCOM5_SPI_IsTransmitterBusy( void )
{
    return ((SPI5_REGS->SPI_SR & SPI_SR_TXEMPTY_Msk) == 0U);
}

