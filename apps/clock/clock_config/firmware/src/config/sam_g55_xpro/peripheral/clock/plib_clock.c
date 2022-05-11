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

#include "device.h"
#include "plib_clock.h"
#include "interrupts.h"






/*********************************************************************************
Initialize Main Clock (MAINCK)
*********************************************************************************/
static void CLK_MainClockInitialize(void)
{

    /* Enable the RC Oscillator */
    PMC_REGS->CKGR_MOR|= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCEN_Msk;

    /* Wait until the RC oscillator clock is ready. */
    while( (PMC_REGS->PMC_SR & PMC_SR_MOSCRCS_Msk) != PMC_SR_MOSCRCS_Msk)
    {

    }

    /* Configure the RC Oscillator frequency */
    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCRCF_Msk) | CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCF_8_MHz;

    /* Wait until the RC oscillator clock is ready */
    while( (PMC_REGS->PMC_SR& PMC_SR_MOSCRCS_Msk) != PMC_SR_MOSCRCS_Msk)
    {

    }

    /* Main RC Oscillator is selected as the Main Clock (MAINCK) source.
       Switch Main Clock (MAINCK) to the RC Oscillator clock */
    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCSEL_Msk) | CKGR_MOR_KEY_PASSWD;

}

/*********************************************************************************
Initialize PLLA (PLLACK)
*********************************************************************************/

static void CLK_PLLAInitialize(void)
{
    /* Configure and Enable PLLA */
    PMC_REGS->CKGR_PLLAR = CKGR_PLLAR_ZERO(0U) | CKGR_PLLAR_PLLACOUNT(0x3fU) |
                              CKGR_PLLAR_MULA(3051U) | CKGR_PLLAR_PLLAEN(1U) ;

    while ( (PMC_REGS->PMC_SR & PMC_SR_LOCKA_Msk) != PMC_SR_LOCKA_Msk)
    {

    }

}


/*********************************************************************************
Initialize Master clock (MCK)
*********************************************************************************/

static void CLK_MasterClockInitialize(void)
{
    /* Program PMC_MCKR.PRES and wait for PMC_SR.MCKRDY to be set   */
    PMC_REGS->PMC_MCKR = (PMC_REGS->PMC_MCKR & ~PMC_MCKR_PRES_Msk) | PMC_MCKR_PRES_CLK_1;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }

    /* Program PMC_MCKR.CSS and Wait for PMC_SR.MCKRDY to be set    */
    PMC_REGS->PMC_MCKR = (PMC_REGS->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_PLLA_CLK;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }

}




/*********************************************************************************
Initialize Programmable Clock (PCKx)
*********************************************************************************/

static void CLK_ProgrammableClockInitialize(void)
{
    /* Disable selected programmable clock  */
    PMC_REGS->PMC_SCDR = PMC_SCDR_PCK1_Msk;

    /* Configure selected programmable clock    */
    PMC_REGS->PMC_PCK[1]= PMC_PCK_CSS_MCK | PMC_PCK_PRES(50);

    /* Enable selected programmable clock   */
    PMC_REGS->PMC_SCER =    PMC_SCER_PCK1_Msk;

    /* Wait for clock to be ready   */
    while( (PMC_REGS->PMC_SR & (PMC_SR_PCKRDY1_Msk) ) != (PMC_SR_PCKRDY1_Msk));


}


/*********************************************************************************
Clock Initialize
*********************************************************************************/
void CLOCK_Initialize( void )
{

    /* Initialize Main Clock */
    CLK_MainClockInitialize();

    /* Initialize PLLA */
    CLK_PLLAInitialize();


    /* Initialize Master Clock */
    CLK_MasterClockInitialize();


    /* Initialize Programmable Clock */
    CLK_ProgrammableClockInitialize();

    /* Enable Peripheral Clock */
    PMC_REGS->PMC_PCER0 = 0x1800;
}

