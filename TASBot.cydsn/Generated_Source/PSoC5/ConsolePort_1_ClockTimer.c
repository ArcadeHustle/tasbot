/*******************************************************************************
* File Name: ConsolePort_1_ClockTimer.c
* Version 2.70
*
* Description:
*  The Timer component consists of a 8, 16, 24 or 32-bit timer with
*  a selectable period between 2 and 2^Width - 1.  The timer may free run
*  or be used as a capture timer as well.  The capture can be initiated
*  by a positive or negative edge signal as well as via software.
*  A trigger input can be programmed to enable the timer on rising edge
*  falling edge, either edge or continous run.
*  Interrupts may be generated due to a terminal count condition
*  or a capture event.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "ConsolePort_1_ClockTimer.h"

uint8 ConsolePort_1_ClockTimer_initVar = 0u;


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_Init
********************************************************************************
*
* Summary:
*  Initialize to the schematic state
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_Init(void) 
{
    #if(!ConsolePort_1_ClockTimer_UsingFixedFunction)
            /* Interrupt State Backup for Critical Region*/
            uint8 ConsolePort_1_ClockTimer_interruptState;
    #endif /* Interrupt state back up for Fixed Function only */

    #if (ConsolePort_1_ClockTimer_UsingFixedFunction)
        /* Clear all bits but the enable bit (if it's already set) for Timer operation */
        ConsolePort_1_ClockTimer_CONTROL &= ConsolePort_1_ClockTimer_CTRL_ENABLE;

        /* Clear the mode bits for continuous run mode */
        #if (CY_PSOC5A)
            ConsolePort_1_ClockTimer_CONTROL2 &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_MODE_MASK));
        #endif /* Clear bits in CONTROL2 only in PSOC5A */

        #if (CY_PSOC3 || CY_PSOC5LP)
            ConsolePort_1_ClockTimer_CONTROL3 &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_MODE_MASK));
        #endif /* CONTROL3 register exists only in PSoC3 OR PSoC5LP */

        /* Check if One Shot mode is enabled i.e. RunMode !=0*/
        #if (ConsolePort_1_ClockTimer_RunModeUsed != 0x0u)
            /* Set 3rd bit of Control register to enable one shot mode */
            ConsolePort_1_ClockTimer_CONTROL |= 0x04u;
        #endif /* One Shot enabled only when RunModeUsed is not Continuous*/

        #if (ConsolePort_1_ClockTimer_RunModeUsed == 2)
            #if (CY_PSOC5A)
                /* Set last 2 bits of control2 register if one shot(halt on
                interrupt) is enabled*/
                ConsolePort_1_ClockTimer_CONTROL2 |= 0x03u;
            #endif /* Set One-Shot Halt on Interrupt bit in CONTROL2 for PSoC5A */

            #if (CY_PSOC3 || CY_PSOC5LP)
                /* Set last 2 bits of control3 register if one shot(halt on
                interrupt) is enabled*/
                ConsolePort_1_ClockTimer_CONTROL3 |= 0x03u;
            #endif /* Set One-Shot Halt on Interrupt bit in CONTROL3 for PSoC3 or PSoC5LP */

        #endif /* Remove section if One Shot Halt on Interrupt is not enabled */

        #if (ConsolePort_1_ClockTimer_UsingHWEnable != 0)
            #if (CY_PSOC5A)
                /* Set the default Run Mode of the Timer to Continuous */
                ConsolePort_1_ClockTimer_CONTROL2 |= ConsolePort_1_ClockTimer_CTRL_MODE_PULSEWIDTH;
            #endif /* Set Continuous Run Mode in CONTROL2 for PSoC5A */

            #if (CY_PSOC3 || CY_PSOC5LP)
                /* Clear and Set ROD and COD bits of CFG2 register */
                ConsolePort_1_ClockTimer_CONTROL3 &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_RCOD_MASK));
                ConsolePort_1_ClockTimer_CONTROL3 |= ConsolePort_1_ClockTimer_CTRL_RCOD;

                /* Clear and Enable the HW enable bit in CFG2 register */
                ConsolePort_1_ClockTimer_CONTROL3 &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_ENBL_MASK));
                ConsolePort_1_ClockTimer_CONTROL3 |= ConsolePort_1_ClockTimer_CTRL_ENBL;

                /* Set the default Run Mode of the Timer to Continuous */
                ConsolePort_1_ClockTimer_CONTROL3 |= ConsolePort_1_ClockTimer_CTRL_MODE_CONTINUOUS;
            #endif /* Set Continuous Run Mode in CONTROL3 for PSoC3ES3 or PSoC5A */

        #endif /* Configure Run Mode with hardware enable */

        /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
        ConsolePort_1_ClockTimer_RT1 &= ((uint8)(~ConsolePort_1_ClockTimer_RT1_MASK));
        ConsolePort_1_ClockTimer_RT1 |= ConsolePort_1_ClockTimer_SYNC;

        /*Enable DSI Sync all all inputs of the Timer*/
        ConsolePort_1_ClockTimer_RT1 &= ((uint8)(~ConsolePort_1_ClockTimer_SYNCDSI_MASK));
        ConsolePort_1_ClockTimer_RT1 |= ConsolePort_1_ClockTimer_SYNCDSI_EN;

        /* Set the IRQ to use the status register interrupts */
        ConsolePort_1_ClockTimer_CONTROL2 |= ConsolePort_1_ClockTimer_CTRL2_IRQ_SEL;
    #endif /* Configuring registers of fixed function implementation */

    /* Set Initial values from Configuration */
    ConsolePort_1_ClockTimer_WritePeriod(ConsolePort_1_ClockTimer_INIT_PERIOD);
    ConsolePort_1_ClockTimer_WriteCounter(ConsolePort_1_ClockTimer_INIT_PERIOD);

    #if (ConsolePort_1_ClockTimer_UsingHWCaptureCounter)/* Capture counter is enabled */
        ConsolePort_1_ClockTimer_CAPTURE_COUNT_CTRL |= ConsolePort_1_ClockTimer_CNTR_ENABLE;
        ConsolePort_1_ClockTimer_SetCaptureCount(ConsolePort_1_ClockTimer_INIT_CAPTURE_COUNT);
    #endif /* Configure capture counter value */

    #if (!ConsolePort_1_ClockTimer_UsingFixedFunction)
        #if (ConsolePort_1_ClockTimer_SoftwareCaptureMode)
            ConsolePort_1_ClockTimer_SetCaptureMode(ConsolePort_1_ClockTimer_INIT_CAPTURE_MODE);
        #endif /* Set Capture Mode for UDB implementation if capture mode is software controlled */

        #if (ConsolePort_1_ClockTimer_SoftwareTriggerMode)
            #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)
                if (0u == (ConsolePort_1_ClockTimer_CONTROL & ConsolePort_1_ClockTimer__B_TIMER__TM_SOFTWARE))
                {
                    ConsolePort_1_ClockTimer_SetTriggerMode(ConsolePort_1_ClockTimer_INIT_TRIGGER_MODE);
                }
            #endif /* (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) */
        #endif /* Set trigger mode for UDB Implementation if trigger mode is software controlled */

        /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
        /* Enter Critical Region*/
        ConsolePort_1_ClockTimer_interruptState = CyEnterCriticalSection();

        /* Use the interrupt output of the status register for IRQ output */
        ConsolePort_1_ClockTimer_STATUS_AUX_CTRL |= ConsolePort_1_ClockTimer_STATUS_ACTL_INT_EN_MASK;

        /* Exit Critical Region*/
        CyExitCriticalSection(ConsolePort_1_ClockTimer_interruptState);

        #if (ConsolePort_1_ClockTimer_EnableTriggerMode)
            ConsolePort_1_ClockTimer_EnableTrigger();
        #endif /* Set Trigger enable bit for UDB implementation in the control register*/
		
		
        #if (ConsolePort_1_ClockTimer_InterruptOnCaptureCount && !ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)
            ConsolePort_1_ClockTimer_SetInterruptCount(ConsolePort_1_ClockTimer_INIT_INT_CAPTURE_COUNT);
        #endif /* Set interrupt count in UDB implementation if interrupt count feature is checked.*/

        ConsolePort_1_ClockTimer_ClearFIFO();
    #endif /* Configure additional features of UDB implementation */

    ConsolePort_1_ClockTimer_SetInterruptMode(ConsolePort_1_ClockTimer_INIT_INTERRUPT_MODE);
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_Enable
********************************************************************************
*
* Summary:
*  Enable the Timer
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (ConsolePort_1_ClockTimer_UsingFixedFunction)
        ConsolePort_1_ClockTimer_GLOBAL_ENABLE |= ConsolePort_1_ClockTimer_BLOCK_EN_MASK;
        ConsolePort_1_ClockTimer_GLOBAL_STBY_ENABLE |= ConsolePort_1_ClockTimer_BLOCK_STBY_EN_MASK;
    #endif /* Set Enable bit for enabling Fixed function timer*/

    /* Remove assignment if control register is removed */
    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED || ConsolePort_1_ClockTimer_UsingFixedFunction)
        ConsolePort_1_ClockTimer_CONTROL |= ConsolePort_1_ClockTimer_CTRL_ENABLE;
    #endif /* Remove assignment if control register is removed */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_Start
********************************************************************************
*
* Summary:
*  The start function initializes the timer with the default values, the
*  enables the timerto begin counting.  It does not enable interrupts,
*  the EnableInt command should be called if interrupt generation is required.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  ConsolePort_1_ClockTimer_initVar: Is modified when this function is called for the
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_Start(void) 
{
    if(ConsolePort_1_ClockTimer_initVar == 0u)
    {
        ConsolePort_1_ClockTimer_Init();

        ConsolePort_1_ClockTimer_initVar = 1u;   /* Clear this bit for Initialization */
    }

    /* Enable the Timer */
    ConsolePort_1_ClockTimer_Enable();
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_Stop
********************************************************************************
*
* Summary:
*  The stop function halts the timer, but does not change any modes or disable
*  interrupts.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Side Effects: If the Enable mode is set to Hardware only then this function
*               has no effect on the operation of the timer.
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_Stop(void) 
{
    /* Disable Timer */
    #if(!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED || ConsolePort_1_ClockTimer_UsingFixedFunction)
        ConsolePort_1_ClockTimer_CONTROL &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_ENABLE));
    #endif /* Remove assignment if control register is removed */

    /* Globally disable the Fixed Function Block chosen */
    #if (ConsolePort_1_ClockTimer_UsingFixedFunction)
        ConsolePort_1_ClockTimer_GLOBAL_ENABLE &= ((uint8)(~ConsolePort_1_ClockTimer_BLOCK_EN_MASK));
        ConsolePort_1_ClockTimer_GLOBAL_STBY_ENABLE &= ((uint8)(~ConsolePort_1_ClockTimer_BLOCK_STBY_EN_MASK));
    #endif /* Disable global enable for the Timer Fixed function block to stop the Timer*/
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SetInterruptMode
********************************************************************************
*
* Summary:
*  This function selects which of the interrupt inputs may cause an interrupt.
*  The twosources are caputure and terminal.  One, both or neither may
*  be selected.
*
* Parameters:
*  interruptMode:   This parameter is used to enable interrups on either/or
*                   terminal count or capture.
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SetInterruptMode(uint8 interruptMode) 
{
    ConsolePort_1_ClockTimer_STATUS_MASK = interruptMode;
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SoftwareCapture
********************************************************************************
*
* Summary:
*  This function forces a capture independent of the capture signal.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Side Effects:
*  An existing hardware capture could be overwritten.
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SoftwareCapture(void) 
{
    /* Generate a software capture by reading the counter register */
    #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
        (void)CY_GET_REG16(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR);
    #else
        (void)CY_GET_REG8(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR_8BIT);
    #endif/* (ConsolePort_1_ClockTimer_UsingFixedFunction) */
    /* Capture Data is now in the FIFO */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadStatusRegister
********************************************************************************
*
* Summary:
*  Reads the status register and returns it's state. This function should use
*  defined types for the bit-field information as the bits in this register may
*  be permuteable.
*
* Parameters:
*  void
*
* Return:
*  The contents of the status register
*
* Side Effects:
*  Status register bits may be clear on read.
*
*******************************************************************************/
uint8   ConsolePort_1_ClockTimer_ReadStatusRegister(void) 
{
    return (ConsolePort_1_ClockTimer_STATUS);
}


#if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) /* Remove API if control register is unused */


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadControlRegister
********************************************************************************
*
* Summary:
*  Reads the control register and returns it's value.
*
* Parameters:
*  void
*
* Return:
*  The contents of the control register
*
*******************************************************************************/
uint8 ConsolePort_1_ClockTimer_ReadControlRegister(void) 
{
    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) 
        return ((uint8)ConsolePort_1_ClockTimer_CONTROL);
    #else
        return (0);
    #endif /* (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_WriteControlRegister
********************************************************************************
*
* Summary:
*  Sets the bit-field of the control register.
*
* Parameters:
*  control: The contents of the control register
*
* Return:
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_WriteControlRegister(uint8 control) 
{
    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) 
        ConsolePort_1_ClockTimer_CONTROL = control;
    #else
        control = 0u;
    #endif /* (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) */
}

#endif /* Remove API if control register is unused */


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadPeriod
********************************************************************************
*
* Summary:
*  This function returns the current value of the Period.
*
* Parameters:
*  void
*
* Return:
*  The present value of the counter.
*
*******************************************************************************/
uint8 ConsolePort_1_ClockTimer_ReadPeriod(void) 
{
   #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
       return ((uint8)CY_GET_REG16(ConsolePort_1_ClockTimer_PERIOD_LSB_PTR));
   #else
       return (CY_GET_REG8(ConsolePort_1_ClockTimer_PERIOD_LSB_PTR));
   #endif /* (ConsolePort_1_ClockTimer_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_WritePeriod
********************************************************************************
*
* Summary:
*  This function is used to change the period of the counter.  The new period
*  will be loaded the next time terminal count is detected.
*
* Parameters:
*  period: This value may be between 1 and (2^Resolution)-1.  A value of 0 will
*          result in the counter remaining at zero.
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_WritePeriod(uint8 period) 
{
    #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
        uint16 period_temp = (uint16)period;
        CY_SET_REG16(ConsolePort_1_ClockTimer_PERIOD_LSB_PTR, period_temp);
    #else
        CY_SET_REG8(ConsolePort_1_ClockTimer_PERIOD_LSB_PTR, period);
    #endif /*Write Period value with appropriate resolution suffix depending on UDB or fixed function implementation */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadCapture
********************************************************************************
*
* Summary:
*  This function returns the last value captured.
*
* Parameters:
*  void
*
* Return:
*  Present Capture value.
*
*******************************************************************************/
uint8 ConsolePort_1_ClockTimer_ReadCapture(void) 
{
   #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
       return ((uint8)CY_GET_REG16(ConsolePort_1_ClockTimer_CAPTURE_LSB_PTR));
   #else
       return (CY_GET_REG8(ConsolePort_1_ClockTimer_CAPTURE_LSB_PTR));
   #endif /* (ConsolePort_1_ClockTimer_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_WriteCounter
********************************************************************************
*
* Summary:
*  This funtion is used to set the counter to a specific value
*
* Parameters:
*  counter:  New counter value.
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_WriteCounter(uint8 counter) 
{
   #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
        /* This functionality is removed until a FixedFunction HW update to
         * allow this register to be written
         */
        CY_SET_REG16(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR, (uint16)counter);
        
    #else
        CY_SET_REG8(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR, counter);
    #endif /* Set Write Counter only for the UDB implementation (Write Counter not available in fixed function Timer */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadCounter
********************************************************************************
*
* Summary:
*  This function returns the current counter value.
*
* Parameters:
*  void
*
* Return:
*  Present compare value.
*
*******************************************************************************/
uint8 ConsolePort_1_ClockTimer_ReadCounter(void) 
{
    /* Force capture by reading Accumulator */
    /* Must first do a software capture to be able to read the counter */
    /* It is up to the user code to make sure there isn't already captured data in the FIFO */
    #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
        (void)CY_GET_REG16(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR);
    #else
        (void)CY_GET_REG8(ConsolePort_1_ClockTimer_COUNTER_LSB_PTR_8BIT);
    #endif/* (ConsolePort_1_ClockTimer_UsingFixedFunction) */

    /* Read the data from the FIFO (or capture register for Fixed Function)*/
    #if(ConsolePort_1_ClockTimer_UsingFixedFunction)
        return ((uint8)CY_GET_REG16(ConsolePort_1_ClockTimer_CAPTURE_LSB_PTR));
    #else
        return (CY_GET_REG8(ConsolePort_1_ClockTimer_CAPTURE_LSB_PTR));
    #endif /* (ConsolePort_1_ClockTimer_UsingFixedFunction) */
}


#if(!ConsolePort_1_ClockTimer_UsingFixedFunction) /* UDB Specific Functions */

    
/*******************************************************************************
 * The functions below this point are only available using the UDB
 * implementation.  If a feature is selected, then the API is enabled.
 ******************************************************************************/


#if (ConsolePort_1_ClockTimer_SoftwareCaptureMode)


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SetCaptureMode
********************************************************************************
*
* Summary:
*  This function sets the capture mode to either rising or falling edge.
*
* Parameters:
*  captureMode: This parameter sets the capture mode of the UDB capture feature
*  The parameter values are defined using the
*  #define ConsolePort_1_ClockTimer__B_TIMER__CM_NONE 0
#define ConsolePort_1_ClockTimer__B_TIMER__CM_RISINGEDGE 1
#define ConsolePort_1_ClockTimer__B_TIMER__CM_FALLINGEDGE 2
#define ConsolePort_1_ClockTimer__B_TIMER__CM_EITHEREDGE 3
#define ConsolePort_1_ClockTimer__B_TIMER__CM_SOFTWARE 4
 identifiers
*  The following are the possible values of the parameter
*  ConsolePort_1_ClockTimer__B_TIMER__CM_NONE        - Set Capture mode to None
*  ConsolePort_1_ClockTimer__B_TIMER__CM_RISINGEDGE  - Rising edge of Capture input
*  ConsolePort_1_ClockTimer__B_TIMER__CM_FALLINGEDGE - Falling edge of Capture input
*  ConsolePort_1_ClockTimer__B_TIMER__CM_EITHEREDGE  - Either edge of Capture input
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SetCaptureMode(uint8 captureMode) 
{
    /* This must only set to two bits of the control register associated */
    captureMode = ((uint8)((uint8)captureMode << ConsolePort_1_ClockTimer_CTRL_CAP_MODE_SHIFT));
    captureMode &= (ConsolePort_1_ClockTimer_CTRL_CAP_MODE_MASK);

    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)
        /* Clear the Current Setting */
        ConsolePort_1_ClockTimer_CONTROL &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_CAP_MODE_MASK));

        /* Write The New Setting */
        ConsolePort_1_ClockTimer_CONTROL |= captureMode;
    #endif /* (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) */
}
#endif /* Remove API if Capture Mode is not Software Controlled */


#if (ConsolePort_1_ClockTimer_SoftwareTriggerMode)


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SetTriggerMode
********************************************************************************
*
* Summary:
*  This function sets the trigger input mode
*
* Parameters:
*  triggerMode: Pass one of the pre-defined Trigger Modes (except Software)
    #define ConsolePort_1_ClockTimer__B_TIMER__TM_NONE 0x00u
    #define ConsolePort_1_ClockTimer__B_TIMER__TM_RISINGEDGE 0x04u
    #define ConsolePort_1_ClockTimer__B_TIMER__TM_FALLINGEDGE 0x08u
    #define ConsolePort_1_ClockTimer__B_TIMER__TM_EITHEREDGE 0x0Cu
    #define ConsolePort_1_ClockTimer__B_TIMER__TM_SOFTWARE 0x10u
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SetTriggerMode(uint8 triggerMode) 
{
    /* This must only set to two bits of the control register associated */
    triggerMode &= ConsolePort_1_ClockTimer_CTRL_TRIG_MODE_MASK;

    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)   /* Remove assignment if control register is removed */
    
        /* Clear the Current Setting */
        ConsolePort_1_ClockTimer_CONTROL &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_TRIG_MODE_MASK));

        /* Write The New Setting */
        ConsolePort_1_ClockTimer_CONTROL |= (triggerMode | ConsolePort_1_ClockTimer__B_TIMER__TM_SOFTWARE);
    #endif /* Remove code section if control register is not used */
}
#endif /* Remove API if Trigger Mode is not Software Controlled */

#if (ConsolePort_1_ClockTimer_EnableTriggerMode)


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_EnableTrigger
********************************************************************************
*
* Summary:
*  Sets the control bit enabling Hardware Trigger mode
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_EnableTrigger(void) 
{
    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)   /* Remove assignment if control register is removed */
        ConsolePort_1_ClockTimer_CONTROL |= ConsolePort_1_ClockTimer_CTRL_TRIG_EN;
    #endif /* Remove code section if control register is not used */
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_DisableTrigger
********************************************************************************
*
* Summary:
*  Clears the control bit enabling Hardware Trigger mode
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_DisableTrigger(void) 
{
    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED )   /* Remove assignment if control register is removed */
        ConsolePort_1_ClockTimer_CONTROL &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_TRIG_EN));
    #endif /* Remove code section if control register is not used */
}
#endif /* Remove API is Trigger Mode is set to None */

#if(ConsolePort_1_ClockTimer_InterruptOnCaptureCount)


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SetInterruptCount
********************************************************************************
*
* Summary:
*  This function sets the capture count before an interrupt is triggered.
*
* Parameters:
*  interruptCount:  A value between 0 and 3 is valid.  If the value is 0, then
*                   an interrupt will occur each time a capture occurs.
*                   A value of 1 to 3 will cause the interrupt
*                   to delay by the same number of captures.
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SetInterruptCount(uint8 interruptCount) 
{
    /* This must only set to two bits of the control register associated */
    interruptCount &= ConsolePort_1_ClockTimer_CTRL_INTCNT_MASK;

    #if (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED)
        /* Clear the Current Setting */
        ConsolePort_1_ClockTimer_CONTROL &= ((uint8)(~ConsolePort_1_ClockTimer_CTRL_INTCNT_MASK));
        /* Write The New Setting */
        ConsolePort_1_ClockTimer_CONTROL |= interruptCount;
    #endif /* (!ConsolePort_1_ClockTimer_UDB_CONTROL_REG_REMOVED) */
}
#endif /* ConsolePort_1_ClockTimer_InterruptOnCaptureCount */


#if (ConsolePort_1_ClockTimer_UsingHWCaptureCounter)


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_SetCaptureCount
********************************************************************************
*
* Summary:
*  This function sets the capture count
*
* Parameters:
*  captureCount: A value between 2 and 127 inclusive is valid.  A value of 1
*                to 127 will cause the interrupt to delay by the same number of
*                captures.
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_SetCaptureCount(uint8 captureCount) 
{
    ConsolePort_1_ClockTimer_CAP_COUNT = captureCount;
}


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ReadCaptureCount
********************************************************************************
*
* Summary:
*  This function reads the capture count setting
*
* Parameters:
*  void
*
* Return:
*  Returns the Capture Count Setting
*
*******************************************************************************/
uint8 ConsolePort_1_ClockTimer_ReadCaptureCount(void) 
{
    return ((uint8)ConsolePort_1_ClockTimer_CAP_COUNT);
}
#endif /* ConsolePort_1_ClockTimer_UsingHWCaptureCounter */


/*******************************************************************************
* Function Name: ConsolePort_1_ClockTimer_ClearFIFO
********************************************************************************
*
* Summary:
*  This function clears all capture data from the capture FIFO
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConsolePort_1_ClockTimer_ClearFIFO(void) 
{
    while(0u != (ConsolePort_1_ClockTimer_ReadStatusRegister() & ConsolePort_1_ClockTimer_STATUS_FIFONEMP))
    {
        (void)ConsolePort_1_ClockTimer_ReadCapture();
    }
}

#endif /* UDB Specific Functions */


/* [] END OF FILE */
