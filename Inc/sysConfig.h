/*!
    \file  sysConfig.h
    \brief System and peripherial configuration
*/

#include "gd32e23x.h"
#include "systick.h"
#include <string.h>



//
// System data struct
//
struct system_s
{
    uint8_t resetRequest;
    volatile uint8_t uart_tx_buff[4];
    volatile uint8_t uart_rx_buff[4];
    uint32_t uart_baud;
};
typedef struct system_s system_t;

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void rcu_config(void)
{
    /* enable GPIO clocks */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable DMA clock*/
    rcu_periph_clock_enable(RCU_DMA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
}

/*!
    \brief      configure the NVIC
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void nvic_config(void)
{
    /* UART_DMA_RX ISR */
    nvic_irq_enable(DMA_Channel1_2_IRQn, 1);
    /* UART_DMA_TX ISR */
    nvic_irq_enable(DMA_Channel3_4_IRQn, 1);
}

/*!
    \brief      configure the GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void gpio_config(void)
{

}

/*!
    \brief      configure the DMA
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void uart_dma_config(system_t *p)
{
    dma_parameter_struct dma_init_struct;
    /* initialize USART */

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, p->uart_baud);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    /* deinitialize DMA channel3(USART0 tx) */
    dma_deinit(DMA_CH2);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)p->uart_tx_buff;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = sizeof(p->uart_tx_buff);
    dma_init_struct.periph_addr = ((uint32_t)&USART_TDATA(USART0));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA_CH2, &dma_init_struct);
    /* deinitialize DMA channel4 (USART0 rx) */
    dma_deinit(DMA_CH3);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)p->uart_rx_buff;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = sizeof(p->uart_rx_buff);
    dma_init_struct.periph_addr = ((uint32_t)&USART_RDATA(USART0));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA_CH3, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA_CH2);
    dma_circulation_disable(DMA_CH3);
    /* USART DMA enable for transmission and reception */
    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);
    usart_dma_receive_config(USART0, USART_DENR_ENABLE);
    dma_interrupt_enable(DMA_CH2, DMA_INT_FTF);
    dma_interrupt_enable(DMA_CH3, DMA_INT_FTF);
    /* enable DMA channel3 */
    dma_channel_enable(DMA_CH2);
    dma_channel_enable(DMA_CH3);
}


/*!
    \brief      erase fmc pages from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void fmc_erase_page(uint32_t page_address)
{

    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

    /* erase the flash pages */
    fmc_page_erase(page_address);
    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

    /* lock the main FMC after the erase operation */
    fmc_lock();
}

/*!
    \brief      program fmc word by word from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void fmc_program(uint32_t address, void *data, uint32_t data_size)
{
    uint32_t *ptr = (uint32_t *)data;
    /* unlock the flash program/erase controller */
    fmc_unlock();
    for (uint32_t i = 0; i < data_size; i += 4)
    {
        /* program flash */
        fmc_word_program((address + i), *(ptr++));
        fmc_flag_clear(FMC_FLAG_END);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_PGERR);
    }

    /* lock the main FMC after the program operation */
    fmc_lock();
}

/*!
    \brief      program fmc word by word from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  none
    \param[out] none
    \retval     none
*/
static inline void fmc_read(uint32_t address, void *data, uint32_t data_size)
{
    uint32_t *ptr = (uint32_t *)data;

    for (uint32_t i = 0; i < data_size; i += 4)
    {
        *(ptr++) = *(uint32_t *)(address + i);
    }
}

/*!
    \brief      send new packet via UART using DMA
    \param[in]  buff: data buffer
    \param[out] none
    \retval     none
*/
static inline void usart_dma_startTx(uint32_t buff, uint32_t size)
{
    /* re-enable transmition */
    usart_transmit_config(USART0, USART_TRANSMIT_DISABLE);
    dma_memory_address_config(DMA_CH2, buff);
    dma_transfer_number_config(DMA_CH2, size);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    dma_channel_enable(DMA_CH2);
}

/*!
    \brief      receive new packet via UART using DMA
    \param[in]  buff: data buffer
    \param[out] none
    \retval     none
*/
static inline void usart_dma_startRx(uint32_t buff, uint32_t size)
{
    /* re-enable transmition */
    usart_receive_config(USART0, USART_RECEIVE_DISABLE);
    dma_memory_address_config(DMA_CH4, buff);
    dma_transfer_number_config(DMA_CH4, size);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    dma_channel_enable(DMA_CH4);
}

/*!
    \brief      config all system modules
    \param[in]  p: pointer to system data struct
    \param[out] none
    \retval     none
*/
static inline void coreStart(system_t *p)
{
    /* init system data */
    p->uart_baud = 115200U;
    /* enable peripherial */
    rcu_config();
    nvic_config();
    systick_config();
    gpio_config();
    //uart_dma_config(p);
    /* start peripherial */
    //usart_dma_startRx((uint32_t)&p->rdc_rx_buff, sizeof(p->rdc_rx_buff));
}
