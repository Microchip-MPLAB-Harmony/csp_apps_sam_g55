/*******************************************************************************
  FLEXCOM7 USART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom7_usart.c

  Summary:
    FLEXCOM7 USART PLIB Implementation File

  Description
    This file defines the interface to the FLEXCOM7 USART
    peripheral library. This library provides access to and control of the
    associated peripheral instance.

  Remarks:
    None.
*******************************************************************************/

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "plib_flexcom7_usart.h"
#include "interrupts.h"

#define FLEXCOM_USART_RHR_8BIT_REG      (*(volatile uint8_t* const)((USART7_BASE_ADDRESS + US_RHR_REG_OFST)))
#define FLEXCOM_USART_RHR_9BIT_REG      (*(volatile uint16_t* const)((USART7_BASE_ADDRESS + US_RHR_REG_OFST)))

#define FLEXCOM_USART_THR_8BIT_REG      (*(volatile uint8_t* const)((USART7_BASE_ADDRESS + US_THR_REG_OFST)))
#define FLEXCOM_USART_THR_9BIT_REG      (*(volatile uint16_t* const)((USART7_BASE_ADDRESS + US_THR_REG_OFST)))

#define FLEXCOM7_USART_READ_BUFFER_SIZE             20U
#define FLEXCOM7_USART_9BIT_READ_BUFFER_SIZE        (20U >> 1U)

/* Disable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM7_USART_RX_INT_DISABLE()      USART7_REGS->US_IDR = (US_IDR_RXRDY_Msk | US_IDR_FRAME_Msk | US_IDR_PARE_Msk | US_IDR_OVRE_Msk)
/* Enable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM7_USART_RX_INT_ENABLE()       USART7_REGS->US_IER = (US_IER_RXRDY_Msk | US_IER_FRAME_Msk | US_IER_PARE_Msk | US_IER_OVRE_Msk)

#define FLEXCOM7_USART_WRITE_BUFFER_SIZE            128U
#define FLEXCOM7_USART_9BIT_WRITE_BUFFER_SIZE       (128U >> 1U)

#define FLEXCOM7_USART_TX_INT_DISABLE()      USART7_REGS->US_IDR = US_IDR_TXRDY_Msk
#define FLEXCOM7_USART_TX_INT_ENABLE()       USART7_REGS->US_IER = US_IER_TXRDY_Msk

volatile static uint8_t FLEXCOM7_USART_ReadBuffer[FLEXCOM7_USART_READ_BUFFER_SIZE];
volatile static uint8_t FLEXCOM7_USART_WriteBuffer[FLEXCOM7_USART_WRITE_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM7 USART Ring Buffer Implementation
// *****************************************************************************
// *****************************************************************************

volatile static FLEXCOM_USART_RING_BUFFER_OBJECT flexcom7UsartObj;

void FLEXCOM7_USART_Initialize( void )
{
    /* Set FLEXCOM USART operating mode */
    FLEXCOM7_REGS->FLEXCOM_MR = FLEXCOM_MR_OPMODE_USART;

    /* Reset FLEXCOM7 USART */
    USART7_REGS->US_CR = (US_CR_RSTRX_Msk | US_CR_RSTTX_Msk | US_CR_RSTSTA_Msk);

    /* Enable FLEXCOM7 USART */
    USART7_REGS->US_CR = (US_CR_TXEN_Msk | US_CR_RXEN_Msk);

    /* Configure FLEXCOM7 USART mode */
    USART7_REGS->US_MR = ((US_MR_USCLKS_MCK) | US_MR_CHRL_8_BIT | US_MR_PAR_NO | US_MR_NBSTOP_1_BIT | (0UL << US_MR_OVER_Pos));

    /* Configure FLEXCOM7 USART Baud Rate */
    USART7_REGS->US_BRGR = US_BRGR_CD(54U) | US_BRGR_FP(2U);

    flexcom7UsartObj.rdCallback = NULL;
    flexcom7UsartObj.rdInIndex = 0;
    flexcom7UsartObj.rdOutIndex = 0;
    flexcom7UsartObj.isRdNotificationEnabled = false;
    flexcom7UsartObj.isRdNotifyPersistently = false;
    flexcom7UsartObj.rdThreshold = 0;
    flexcom7UsartObj.wrCallback = NULL;
    flexcom7UsartObj.wrInIndex = 0;
    flexcom7UsartObj.wrOutIndex = 0;
    flexcom7UsartObj.isWrNotificationEnabled = false;
    flexcom7UsartObj.isWrNotifyPersistently = false;
    flexcom7UsartObj.wrThreshold = 0;
    flexcom7UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
    {
        flexcom7UsartObj.rdBufferSize = FLEXCOM7_USART_9BIT_READ_BUFFER_SIZE;
        flexcom7UsartObj.wrBufferSize = FLEXCOM7_USART_9BIT_WRITE_BUFFER_SIZE;
    }
    else
    {
        flexcom7UsartObj.rdBufferSize = FLEXCOM7_USART_READ_BUFFER_SIZE;
        flexcom7UsartObj.wrBufferSize = FLEXCOM7_USART_WRITE_BUFFER_SIZE;
    }

    FLEXCOM7_USART_RX_INT_ENABLE();
}

void static FLEXCOM7_USART_ErrorClear( void )
{
    USART7_REGS->US_CR = US_CR_RSTSTA_Msk;

    /* Flush existing error bytes from the RX FIFO */
    while ((USART7_REGS->US_CSR& US_CSR_RXRDY_Msk) != 0U)
    {
        (void)(USART7_REGS->US_RHR);
    }
}

FLEXCOM_USART_ERROR FLEXCOM7_USART_ErrorGet( void )
{
    FLEXCOM_USART_ERROR errors = flexcom7UsartObj.errorStatus;

    flexcom7UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errors;
}

static void FLEXCOM7_USART_BaudCalculate(uint32_t srcClkFreq, uint32_t reqBaud, uint8_t overSamp, uint32_t* cd, uint32_t* fp, uint32_t* baudError)
{
    uint32_t actualBaud = 0U;

    *cd = srcClkFreq / (reqBaud * 8U * (2U - (uint32_t)overSamp));

    if (*cd > 0U)
    {
        *fp = ((srcClkFreq / (reqBaud * (2U - (uint32_t)overSamp))) - ((*cd) * 8U));
        actualBaud = (srcClkFreq / (((*cd) * 8U) + (*fp))) / (2U - overSamp);
        *baudError = ((100U * actualBaud)/reqBaud) - 100U;
    }
}

bool FLEXCOM7_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    uint32_t baud = 0U;
    uint32_t overSampVal = 0U;
    uint32_t usartMode;
    uint32_t cd0, fp0, cd1, fp1, baudError0, baudError1;
    bool status = false;

    cd0 = fp0 = cd1 = fp1 = baudError0 = baudError1 = 0U;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if(srcClkFreq == 0U)
        {
            srcClkFreq = FLEXCOM7_USART_FrequencyGet();
        }

        /* Calculate baud register values for 8x/16x oversampling values */

        FLEXCOM7_USART_BaudCalculate(srcClkFreq, baud, 0, &cd0, &fp0, &baudError0);
        FLEXCOM7_USART_BaudCalculate(srcClkFreq, baud, 1, &cd1, &fp1, &baudError1);

        if ( !(cd0 > 0U && cd0 <= 65535U) && !(cd1 > 0U && cd1 <= 65535U) )
        {
            /* Requested baud cannot be generated with current clock settings */
            return status;
        }

        if ( (cd0 > 0U && cd0 <= 65535U) && (cd1 > 0U && cd1 <= 65535U) )
        {
            if (baudError1 < baudError0)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << US_MR_OVER_Pos) & US_MR_OVER_Msk;
            }
        }
        else
        {
            /* Requested baud can be generated with either with 8x oversampling or with 16x oversampling. Select valid one. */
            if (cd1 > 0U && cd1 <= 65535U)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << US_MR_OVER_Pos) & US_MR_OVER_Msk;
            }
        }

        /* Configure FLEXCOM7 USART mode */
        usartMode = USART7_REGS->US_MR;
        usartMode &= ~(US_MR_CHRL_Msk | US_MR_MODE9_Msk | US_MR_PAR_Msk | US_MR_NBSTOP_Msk | US_MR_OVER_Msk);
        USART7_REGS->US_MR = usartMode | ((uint32_t)setup->dataWidth | (uint32_t)setup->parity | (uint32_t)setup->stopBits | overSampVal);

        /* Configure FLEXCOM7 USART Baud Rate */
        USART7_REGS->US_BRGR = US_BRGR_CD(cd0) | US_BRGR_FP(fp0);

        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            flexcom7UsartObj.rdBufferSize = FLEXCOM7_USART_9BIT_READ_BUFFER_SIZE;
            flexcom7UsartObj.wrBufferSize = FLEXCOM7_USART_9BIT_WRITE_BUFFER_SIZE;
        }
        else
        {
            flexcom7UsartObj.rdBufferSize = FLEXCOM7_USART_READ_BUFFER_SIZE;
            flexcom7UsartObj.wrBufferSize = FLEXCOM7_USART_WRITE_BUFFER_SIZE;
        }

        status = true;
    }

    return status;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool FLEXCOM7_USART_TxPullByte(void* pWrData)
{
    bool isSuccess = false;
    uint32_t wrOutIndex = flexcom7UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom7UsartObj.wrInIndex;
    uint8_t* pWrByte = (uint8_t*)pWrData;
    if (wrOutIndex != wrInIndex)
    {
        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            uint32_t wrOutIdx = wrOutIndex << 1U;
            pWrByte[0] = FLEXCOM7_USART_WriteBuffer[wrOutIdx];
            pWrByte[1] = FLEXCOM7_USART_WriteBuffer[wrOutIdx + 1U];
        }
        else
        {
            *pWrByte = FLEXCOM7_USART_WriteBuffer[wrOutIndex];
        }
        wrOutIndex++;

        if (wrOutIndex >= flexcom7UsartObj.wrBufferSize)
        {
            wrOutIndex = 0U;
        }

        flexcom7UsartObj.wrOutIndex = wrOutIndex;

        isSuccess = true;
    }

    return isSuccess;
}

static inline bool FLEXCOM7_USART_TxPushByte(uint16_t wrByte)
{
    uint32_t tempInIndex;
    uint32_t wrOutIndex = flexcom7UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom7UsartObj.wrInIndex;

    bool isSuccess = false;

    tempInIndex = wrInIndex + 1U;

    if (tempInIndex >= flexcom7UsartObj.wrBufferSize)
    {
        tempInIndex = 0U;
    }
    if (tempInIndex != wrOutIndex)
    {
        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            uint32_t wrInIdx = wrInIndex << 1U;
            FLEXCOM7_USART_WriteBuffer[wrInIdx] = (uint8_t)wrByte;
            FLEXCOM7_USART_WriteBuffer[wrInIdx + 1U] = (uint8_t)(wrByte >> 8U);
        }
        else
        {
            FLEXCOM7_USART_WriteBuffer[wrInIndex] = (uint8_t)wrByte;
        }

        flexcom7UsartObj.wrInIndex = tempInIndex;

        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM7_USART_SendWriteNotification(void)
{
    uint32_t nFreeWrBufferCount;

    if (flexcom7UsartObj.isWrNotificationEnabled == true)
    {
        uintptr_t wrContext = flexcom7UsartObj.wrContext;

        nFreeWrBufferCount = FLEXCOM7_USART_WriteFreeBufferCountGet();

        if(flexcom7UsartObj.wrCallback != NULL)
        {
            if (flexcom7UsartObj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= flexcom7UsartObj.wrThreshold)
                {
                    flexcom7UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == flexcom7UsartObj.wrThreshold)
                {
                    flexcom7UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
        }
    }
}

static size_t FLEXCOM7_USART_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;

    /* Take a snapshot of indices to avoid creation of critical section */
    uint32_t wrOutIndex = flexcom7UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom7UsartObj.wrInIndex;

    if ( wrInIndex >=  wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex - wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (flexcom7UsartObj.wrBufferSize -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t FLEXCOM7_USART_WriteCountGet(void)
{
    size_t nPendingTxBytes;

    nPendingTxBytes = FLEXCOM7_USART_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t FLEXCOM7_USART_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0;

    while (nBytesWritten < size)
    {
        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            uint16_t halfWordData = (uint16_t)(pWrBuffer[(2U * nBytesWritten) + 1U]);
            halfWordData <<= 8U;
            halfWordData |= (uint16_t)pWrBuffer[2U * nBytesWritten];
            if (FLEXCOM7_USART_TxPushByte(halfWordData) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
        else
        {
            if (FLEXCOM7_USART_TxPushByte(pWrBuffer[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }

    }

    /* Check if any data is pending for transmission */
    if (FLEXCOM7_USART_WritePendingBytesGet() > 0U)
    {
        /* Enable TX interrupt as data is pending for transmission */
        FLEXCOM7_USART_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t FLEXCOM7_USART_WriteFreeBufferCountGet(void)
{
    return (flexcom7UsartObj.wrBufferSize - 1U) - FLEXCOM7_USART_WriteCountGet();
}

size_t FLEXCOM7_USART_WriteBufferSizeGet(void)
{
    return (flexcom7UsartObj.wrBufferSize - 1U);
}

bool FLEXCOM7_USART_TransmitComplete( void )
{
    return ((USART7_REGS->US_CSR & US_CSR_TXEMPTY_Msk) != 0U);
}

bool FLEXCOM7_USART_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom7UsartObj.isWrNotificationEnabled;

    flexcom7UsartObj.isWrNotificationEnabled = isEnabled;

    flexcom7UsartObj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void FLEXCOM7_USART_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        flexcom7UsartObj.wrThreshold = nBytesThreshold;
    }
}

void FLEXCOM7_USART_WriteCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom7UsartObj.wrCallback = callback;

    flexcom7UsartObj.wrContext = context;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool FLEXCOM7_USART_RxPushByte(uint16_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = flexcom7UsartObj.rdInIndex + 1U;

    if (tempInIndex >= flexcom7UsartObj.rdBufferSize)
    {
        tempInIndex = 0U;
    }

    if (tempInIndex == flexcom7UsartObj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(flexcom7UsartObj.rdCallback != NULL)
        {
            uintptr_t rdContext = flexcom7UsartObj.rdContext;

            flexcom7UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_BUFFER_FULL, rdContext);

            /* Read the indices again in case application has freed up space in RX ring buffer */
            tempInIndex = flexcom7UsartObj.rdInIndex + 1U;

            if (tempInIndex >= flexcom7UsartObj.rdBufferSize)
            {
                tempInIndex = 0U;
            }
        }
    }

    /* Attempt to push the data into the ring buffer */
    if (tempInIndex != flexcom7UsartObj.rdOutIndex)
    {
        uint32_t rdInIdx;

        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            rdInIdx = flexcom7UsartObj.rdInIndex << 1U;
            FLEXCOM7_USART_ReadBuffer[rdInIdx] = (uint8_t)rdByte;
            FLEXCOM7_USART_ReadBuffer[rdInIdx + 1U] = (uint8_t)(rdByte >> 8U);
        }
        else
        {
            rdInIdx = flexcom7UsartObj.rdInIndex;
            FLEXCOM7_USART_ReadBuffer[rdInIdx] = (uint8_t)rdByte;
        }

        flexcom7UsartObj.rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM7_USART_SendReadNotification(void)
{
    uint32_t nUnreadBytesAvailable;

    if (flexcom7UsartObj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = FLEXCOM7_USART_ReadCountGet();

        if(flexcom7UsartObj.rdCallback != NULL)
        {
            uintptr_t rdContext = flexcom7UsartObj.rdContext;

            if (flexcom7UsartObj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= flexcom7UsartObj.rdThreshold)
                {
                    flexcom7UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == flexcom7UsartObj.rdThreshold)
                {
                    flexcom7UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
        }
    }
}

size_t FLEXCOM7_USART_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0;
    uint32_t rdOutIndex = flexcom7UsartObj.rdOutIndex;
    uint32_t rdInIndex = flexcom7UsartObj.rdInIndex;

    while (nBytesRead < size)
    {
        if (rdOutIndex != rdInIndex)
        {
            if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
            {
                uint32_t rdOutIdx = rdOutIndex << 1U;
                uint32_t nBytesReadIdx = nBytesRead << 1U;
                pRdBuffer[nBytesReadIdx] = FLEXCOM7_USART_ReadBuffer[rdOutIdx];
                pRdBuffer[nBytesReadIdx + 1U] = FLEXCOM7_USART_ReadBuffer[rdOutIdx + 1U];
            }
            else
            {
                pRdBuffer[nBytesRead] = FLEXCOM7_USART_ReadBuffer[rdOutIndex];
            }
            nBytesRead++;
            rdOutIndex++;

            if (rdOutIndex >= flexcom7UsartObj.rdBufferSize)
            {
                rdOutIndex = 0U;
            }
        }
        else
        {
            /* No more data available in the RX buffer */
            break;
        }
    }

    flexcom7UsartObj.rdOutIndex = rdOutIndex;

    return nBytesRead;
}

size_t FLEXCOM7_USART_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
    uint32_t rdOutIndex;
    uint32_t rdInIndex;

    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = flexcom7UsartObj.rdOutIndex;
    rdInIndex = flexcom7UsartObj.rdInIndex;

    if ( rdInIndex >= rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex - rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (flexcom7UsartObj.rdBufferSize -  rdOutIndex) + rdInIndex;
    }

    return nUnreadBytesAvailable;
}

size_t FLEXCOM7_USART_ReadFreeBufferCountGet(void)
{
    return (flexcom7UsartObj.rdBufferSize - 1U) - FLEXCOM7_USART_ReadCountGet();
}

size_t FLEXCOM7_USART_ReadBufferSizeGet(void)
{
    return (flexcom7UsartObj.rdBufferSize - 1U);
}

bool FLEXCOM7_USART_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom7UsartObj.isRdNotificationEnabled;

    flexcom7UsartObj.isRdNotificationEnabled = isEnabled;

    flexcom7UsartObj.isRdNotifyPersistently = isPersistent;

    return previousStatus;
}

void FLEXCOM7_USART_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        flexcom7UsartObj.rdThreshold = nBytesThreshold;
    }
}

void FLEXCOM7_USART_ReadCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom7UsartObj.rdCallback = callback;

    flexcom7UsartObj.rdContext = context;
}

void static __attribute__((used)) FLEXCOM7_USART_ISR_RX_Handler( void )
{
    uint16_t rdData = 0U;

    /* Keep reading until there is a character availabe in the RX FIFO */
    while((USART7_REGS->US_CSR & US_CSR_RXRDY_Msk) != 0U)
    {
        if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
        {
            rdData = FLEXCOM_USART_RHR_9BIT_REG & (uint16_t)US_RHR_RXCHR_Msk;
        }
        else
        {
            rdData = FLEXCOM_USART_RHR_8BIT_REG;
        }

        if (FLEXCOM7_USART_RxPushByte( rdData ) == true)
        {
            FLEXCOM7_USART_SendReadNotification();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }
}

void static __attribute__((used)) FLEXCOM7_USART_ISR_TX_Handler( void )
{
    uint16_t wrByte;

    /* Keep writing to the TX FIFO as long as there is space */
    while ((USART7_REGS->US_CSR & US_CSR_TXRDY_Msk) != 0U)
    {
        if (FLEXCOM7_USART_TxPullByte(&wrByte) == true)
        {
            if ((USART7_REGS->US_MR & US_MR_MODE9_Msk) != 0U)
            {
                FLEXCOM_USART_THR_9BIT_REG = wrByte & (uint16_t)US_THR_TXCHR_Msk;
            }
            else
            {
                FLEXCOM_USART_THR_8BIT_REG = (uint8_t)wrByte;
            }

            /* Send notification */
            FLEXCOM7_USART_SendWriteNotification();
        }
        else
        {
            /* Nothing to transmit. Disable the data register empty interrupt. */
            FLEXCOM7_USART_TX_INT_DISABLE();
            break;
        }
    }
}

void __attribute__((used)) FLEXCOM7_InterruptHandler( void )
{
    /* Channel status */
    uint32_t channelStatus = USART7_REGS->US_CSR;

    /* Error status */
    uint32_t errorStatus = (channelStatus & (US_CSR_OVRE_Msk | US_CSR_FRAME_Msk | US_CSR_PARE_Msk));

    if(errorStatus != 0U)
    {
        /* Save the error so that it can be reported when application calls the FLEXCOM7_USART_ErrorGet() API */
        flexcom7UsartObj.errorStatus = (FLEXCOM_USART_ERROR)errorStatus;

        /* Clear the error flags and flush out the error bytes */
        FLEXCOM7_USART_ErrorClear();

        /* USART errors are normally associated with the receiver, hence calling receiver context */
        if( flexcom7UsartObj.rdCallback != NULL )
        {
            uintptr_t rdContext = flexcom7UsartObj.rdContext;

            flexcom7UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_ERROR, rdContext);
        }
    }

    /* Receiver status */
    if ((channelStatus & US_CSR_RXRDY_Msk) != 0U)
    {
        FLEXCOM7_USART_ISR_RX_Handler();
    }

    /* Transmitter status */
    if (((channelStatus & US_CSR_TXRDY_Msk) != 0U) && ((USART7_REGS->US_IMR & US_IMR_TXRDY_Msk) != 0U) )
    {
        FLEXCOM7_USART_ISR_TX_Handler();
    }
}
