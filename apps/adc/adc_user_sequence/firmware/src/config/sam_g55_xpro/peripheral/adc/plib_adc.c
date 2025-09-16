/*******************************************************************************
  ADC Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_adc.c

  Summary
    ADC peripheral library source.

  Description
    This file implements the ADC peripheral library.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#include "device.h"
#include "plib_adc.h"
#include "interrupts.h"

#define ADC_SEQ1_CHANNEL_NUM (8U)


// *****************************************************************************
// *****************************************************************************
// Section: ADC Implementation
// *****************************************************************************
// *****************************************************************************

/* Object to hold callback function and context */
static volatile ADC_CALLBACK_OBJECT ADC_CallbackObj;

/* Initialize ADC peripheral */
void ADC_Initialize( void )
{
    /* Software reset */
    ADC_REGS->ADC_CR = ADC_CR_SWRST_Msk;

    /* Prescaler and different time settings as per CLOCK section  */
    ADC_REGS->ADC_MR = ADC_MR_PRESCAL(7U) | ADC_MR_TRACKTIM(15U) | ADC_MR_STARTUP_SUT64 | ADC_MR_TRANSFER(2U) | ADC_MR_ANACH_Msk ;

    /* resolution and sign mode of result */
    ADC_REGS->ADC_EMR = ADC_EMR_OSR_NO_AVERAGE  | ADC_EMR_SRCCLK_PERIPH_CLK | ADC_EMR_TAG_Msk | ADC_EMR_CMPFILTER(0U)  | ADC_EMR_CMPSEL(0U)  | ADC_EMR_CMPTYPE(ADC_EMR_CMPTYPE_FLAG_ONLY_Val) | ADC_EMR_CMPMODE(ADC_EMR_CMPMODE_LOW);


    /* User defined channel conversion sequence */
    ADC_REGS->ADC_MR |= ADC_MR_USEQ_Msk;

    ADC_REGS->ADC_SEQR1 = ADC_SEQR1_USCH1(ADC_CH2) | ADC_SEQR1_USCH2(ADC_CH0) | ADC_SEQR1_USCH3(ADC_CH1);

    /* Enable interrupt */
    ADC_REGS->ADC_IER = ADC_IER_EOC1_Msk;

    ADC_CallbackObj.callback_fn = NULL;

    /* Enable channel */
    ADC_REGS->ADC_CHER = ADC_CHER_CH0_Msk | ADC_CHER_CH1_Msk | ADC_CHER_CH2_Msk;
}

/* Enable ADC channels */
void ADC_ChannelsEnable( ADC_CHANNEL_MASK channelsMask )
{
    ADC_REGS->ADC_CHER |= (uint32_t)channelsMask;
}

/* Disable ADC channels */
void ADC_ChannelsDisable( ADC_CHANNEL_MASK channelsMask )
{
    ADC_REGS->ADC_CHDR |= (uint32_t)channelsMask;
}

/* Enable channel end of conversion interrupt */
void ADC_ChannelsInterruptEnable( ADC_INTERRUPT_MASK channelsInterruptMask )
{
    ADC_REGS->ADC_IER |= (uint32_t)channelsInterruptMask;
}

/* Disable channel end of conversion interrupt */
void ADC_ChannelsInterruptDisable( ADC_INTERRUPT_MASK channelsInterruptMask )
{
    ADC_REGS->ADC_IDR |= (uint32_t)channelsInterruptMask;
}

/* Start the conversion with software trigger */
void ADC_ConversionStart( void )
{
    ADC_REGS->ADC_CR = 0x1U << ADC_CR_START_Pos;
}

/* Check if conversion result is available */
bool ADC_ChannelResultIsReady( ADC_CHANNEL_NUM channel )
{
    return (((ADC_REGS->ADC_ISR >> (uint32_t)channel) & 0x1U) != 0U);
}

/* Read the conversion result */
uint16_t ADC_ChannelResultGet( ADC_CHANNEL_NUM channel )
{
    return (uint16_t)(ADC_REGS->ADC_CDR[channel]);
}

/* Configure the user defined conversion sequence */
void ADC_ConversionSequenceSet( ADC_CHANNEL_NUM *channelList, uint8_t numChannel )
{
    uint8_t channelIndex;
    (void)numChannel;

    ADC_REGS->ADC_SEQR1 = 0U;

    for (channelIndex = 0U; channelIndex < ADC_SEQ1_CHANNEL_NUM; channelIndex++)
    {
        ADC_REGS->ADC_SEQR1 |= (uint32_t)channelList[channelIndex] << (channelIndex * 4U);
    }
}

/* Set the comparator channel */
void ADC_ComparatorChannelSet(ADC_CHANNEL_NUM channel)
{
    ADC_REGS->ADC_EMR &= ~(ADC_EMR_CMPSEL_Msk | ADC_EMR_CMPALL_Msk);
    ADC_REGS->ADC_EMR |= ((uint32_t)channel << ADC_EMR_CMPSEL_Pos);
}

/* Enable compare on all channels */
void ADC_CompareAllChannelsEnable(void)
{
    ADC_REGS->ADC_EMR |= ADC_EMR_CMPALL_Msk;
}

/* Disable compare on all channels */
void ADC_CompareAllChannelsDisable(void)
{
    ADC_REGS->ADC_EMR &= ~ADC_EMR_CMPALL_Msk;
}

/* Stops the conversion result storage until the next comparison match */
void ADC_CompareRestart(void)
{
    ADC_REGS->ADC_CR |= ADC_CR_CMPRST_Msk;
}

/* Set the comparator mode */
void ADC_ComparatorModeSet(ADC_COMPARATOR_MODE cmpMode)
{
    ADC_REGS->ADC_EMR &= ~(ADC_EMR_CMPMODE_Msk);
    ADC_REGS->ADC_EMR |= ((uint32_t)cmpMode << ADC_EMR_CMPMODE_Pos);
}

/* Register the callback function */
void ADC_CallbackRegister( ADC_CALLBACK callback, uintptr_t context )
{
    ADC_CallbackObj.callback_fn = callback;

    ADC_CallbackObj.context = context;
}

/* Interrupt Handler */
void __attribute__((used)) ADC_InterruptHandler( void )
{
    uint32_t status = ADC_REGS->ADC_ISR;

    if (ADC_CallbackObj.callback_fn != NULL)
    {
        uintptr_t context = ADC_CallbackObj.context;
        ADC_CallbackObj.callback_fn(status, context);
    }
}
