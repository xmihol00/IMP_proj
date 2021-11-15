
#include "UART.h"

static intr_handle_t handle_console;
uint8_t recieve_buffer[BUFFER_SIZE];

static void IRAM_ATTR uart_interupt_handler(void *arg)
{
    uint16_t rx_fifo_len;
    uint16_t i = 0;

    // TODO UART0.int_st.val
    rx_fifo_len = UART0.status.rxfifo_cnt; // read number of bytes in UART buffer
    
    recieve_buffer[rx_fifo_len] = '\0';
    while(rx_fifo_len-- && i < BUFFER_SIZE)
    {
        recieve_buffer[i] = UART0.fifo.rw_byte;
        uart_write_bytes(ACTIVE_UART, (const char*) &recieve_buffer[i], 1);
        i++;
    }

    uart_clear_intr_status(ACTIVE_UART, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
}

void init_uart()
{
    uart_config_t uart_config = 
    {
		.baud_rate = UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	ESP_ERROR_CHECK(uart_param_config(ACTIVE_UART, &uart_config));

	//Set UART pins (using UART0 default pins ie no changes.)
	ESP_ERROR_CHECK(uart_set_pin(ACTIVE_UART, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	//Install UART driver, and get the queue.
	ESP_ERROR_CHECK(uart_driver_install(ACTIVE_UART, BUFFER_SIZE * 2, 0, 0, NULL, 0));

	// release the pre registered UART handler/subroutine
	ESP_ERROR_CHECK(uart_isr_free(ACTIVE_UART));

	// register new UART subroutine
	ESP_ERROR_CHECK(uart_isr_register(ACTIVE_UART, uart_interupt_handler, NULL, ESP_INTR_FLAG_IRAM, &handle_console));

	// enable RX interrupt
	ESP_ERROR_CHECK(uart_enable_rx_intr(ACTIVE_UART));
}
