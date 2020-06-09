/*******************************************************************************
  Cyclic Redundancy Check Calculation Unit (CRCCU) PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_crccu.c

  Summary:
    CRCCU PLIB Implementation file

  Description:
    This file defines the interface to the CRCCU peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

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

#include "plib_crccu.h"

crccu_dscr_type_t crc_dscr __ALIGNED(512);
void CRCCU_Initialize (void)
{
    CRCCU_REGS->CRCCU_MR = CRCCU_MR_PTYPE_CCITT8023 | CRCCU_MR_DIVIDER(0) | CRCCU_MR_BITORDER(0);
    crc_dscr.ul_tr_ctrl = 0 << 24;
    CRCCU_REGS->CRCCU_DSCR = (uint32_t) &crc_dscr;
}

bool CRCCU_CRCCalculate(uint32_t startAddress, uint16_t length, uint32_t * crc, bool chain)
{
    bool statusValue = false;

    if( (length != 0) && (crc != NULL) )
    {
        crc_dscr.ul_tr_addr = startAddress;

        if(chain == false)
        {
            CRCCU_REGS->CRCCU_CR = CRCCU_CR_RESET_Msk;
        }

        crc_dscr.ul_tr_ctrl = crc_dscr.ul_tr_ctrl & (0xffff0000);
        crc_dscr.ul_tr_ctrl |= length;

        CRCCU_REGS->CRCCU_MR |= CRCCU_MR_ENABLE_Msk;

        CRCCU_REGS->CRCCU_DMA_EN = CRCCU_DMA_EN_DMAEN_Msk;

        while((CRCCU_REGS->CRCCU_DMA_ISR & CRCCU_DMA_ISR_DMAISR_Msk) != CRCCU_DMA_ISR_DMAISR_Msk)
        {
            /* Wait for the DSU Operation to Complete */
        }

        /* Reading the resultant crc value from the DATA register */
        *crc = CRCCU_REGS->CRCCU_SR;
        
        statusValue = true;
    }

    return statusValue;
}

void CRCCU_Setup (CRCCU_POLYNOMIAL polynomial, CRCCU_TWIDTH width)
{
    crc_dscr.ul_tr_ctrl = width << 24;
    CRCCU_REGS->CRCCU_MR &= ~(CRCCU_MR_PTYPE_Msk);
    CRCCU_REGS->CRCCU_MR |= polynomial;
}
