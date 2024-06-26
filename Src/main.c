/*!
    \file  main.c
    \brief Firmware for template project
*/

/**
 * @todo
 *
 */

#include "gd32e23x.h"
#include "sysConfig.h"

system_t mcu;


int main(void)
{
    /* Init periph */
    coreStart(&mcu);

    while (1)
    {
        if (mcu.resetRequest == 1)
        {
            NVIC_SystemReset();
        }
    }
}

//
// 1ms tick ISR
//
void SysTick_Handler(void)
{
    delay_decrement();
}

//
// USART DMA RX ISR
//
void DMA_Channel3_4_IRQHandler(void)
{
    if (dma_flag_get(DMA_CH3, DMA_INTF_FTFIF) == SET)
    {
        dma_flag_clear(DMA_CH3, DMA_INTF_FTFIF);
        dma_channel_disable(DMA_CH3);
    }
}

//
// USART DMA TX ISR
//
void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_flag_get(DMA_CH2, DMA_INTF_FTFIF) == SET)
    {
        dma_flag_clear(DMA_CH2, DMA_FLAG_G);
        dma_channel_disable(DMA_CH2);
    }
}
