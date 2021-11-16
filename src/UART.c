
#include "UART.h"

static TaskHandle_t parse_handle = NULL;
static intr_handle_t handle_console;
static uint8_t recieve_buffer_pos = UINT8_MAX;
char recieve_buffer[BUFFER_SIZE];
char send_buffer[BUFFER_SIZE];

static void parse_input();
static void skip_buffer_spaces();
static void skip_buffer_characters();

static void IRAM_ATTR uart_interupt_handler(void *arg)
{
    uint16_t rx_fifo_len;

    // TODO UART0.int_st.val
	UART0.status.ctsn;
    rx_fifo_len = UART0.status.rxfifo_cnt; // read number of bytes in UART buffer
    
    while(rx_fifo_len--)
    {
        recieve_buffer[++recieve_buffer_pos] = UART0.fifo.rw_byte;
        uart_write_bytes(ACTIVE_UART, &recieve_buffer[recieve_buffer_pos], 1);
		if (recieve_buffer[recieve_buffer_pos] == '\r')
		{
			recieve_buffer[recieve_buffer_pos] = '\n';
			uart_write_bytes(ACTIVE_UART, &recieve_buffer[recieve_buffer_pos], 1);
		}
    }
	
	if (recieve_buffer[recieve_buffer_pos] == '\n')
	{
		xTaskCreate(&parse_input, "parse_input", 2048, NULL, 5, &parse_handle);
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
	ESP_ERROR_CHECK(uart_driver_install(ACTIVE_UART, BUFFER_SIZE << 1, 0, 0, NULL, 0));

	// release the pre registered UART handler/subroutine
	ESP_ERROR_CHECK(uart_isr_free(ACTIVE_UART));

	// register new UART subroutine
	ESP_ERROR_CHECK(uart_isr_register(ACTIVE_UART, uart_interupt_handler, NULL, ESP_INTR_FLAG_IRAM, &handle_console));

	// enable RX interrupt
	ESP_ERROR_CHECK(uart_enable_rx_intr(ACTIVE_UART));
}

void uart_print_measurment(measurment_t *measurment)
{
	struct tm local_time;
	localtime_r(&measurment->time, &local_time);
	sprintf(&send_buffer[strftime(send_buffer, BUFFER_SIZE, "%c", &local_time) - 5], ": %.3f °C\r\n", measurment->temperature);

	uart_write_bytes(ACTIVE_UART, send_buffer, strlen(send_buffer));
}

static void parse_input()
{
	int32_t parsed = 0;
	uint8_t unrecognized = 0;
	recieve_buffer[++recieve_buffer_pos] = '\0';
	
	recieve_buffer_pos = 0;
	skip_buffer_spaces();

	if (recieve_buffer[recieve_buffer_pos] == 's')
	{

	}
	else if (recieve_buffer[recieve_buffer_pos] == 'p')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || (strncmp(&recieve_buffer[recieve_buffer_pos += 4], "rint", 4) && isspace(recieve_buffer[recieve_buffer_pos])))
		{
			skip_buffer_spaces();

			parsed = atoi(&recieve_buffer[recieve_buffer_pos]);
			if (parsed < 0)
			{
				unrecognized = 2;
			}
			else
			{
				skip_buffer_characters();
				skip_buffer_spaces();

				unrecognized = print_samples((uint32_t)parsed, recieve_buffer[recieve_buffer_pos]);
			}
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'l')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || (strncmp(&recieve_buffer[recieve_buffer_pos += 2], "og", 2) && isspace(recieve_buffer[recieve_buffer_pos])))
		{
			skip_buffer_spaces();

			parsed = atoi(&recieve_buffer[recieve_buffer_pos]);
			if (parsed < 0)
			{
				unrecognized = 2;
			}
			else
			{
				skip_buffer_characters();
				skip_buffer_spaces();

				unrecognized = set_log_interval((uint32_t)parsed, recieve_buffer[recieve_buffer_pos]);
			}
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'h')
	{

	}
	else
	{
		unrecognized = 1;
	}

	if (unrecognized)
	{
		sprintf(recieve_buffer, "Command was not recognized.\r\n");
		uart_write_bytes(ACTIVE_UART, recieve_buffer, strlen(recieve_buffer));
	}	

	recieve_buffer_pos = UINT8_MAX;
	vTaskDelete(parse_handle);
}

static void skip_buffer_spaces()
{
	while (isspace(recieve_buffer[recieve_buffer_pos]))
	{
		recieve_buffer_pos++;
	}
}

static void skip_buffer_characters()
{
	while (!isspace(recieve_buffer[recieve_buffer_pos]))
	{
		recieve_buffer_pos++;
	}
}
