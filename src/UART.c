
#include "UART.h"

static TaskHandle_t parse_handle = NULL;
static intr_handle_t handle_console;
static uint8_t recieve_buffer_pos = UINT8_MAX;
char recieve_buffer[BUFFER_SIZE];
char send_buffer[BUFFER_SIZE];
static uint8_t wifi_status = NO_WIFI;

extern credentials_t credentials;
extern esp_ip4_addr_t s_ip_addr;

static void parse_input();
static void skip_buffer_spaces();
static void skip_buffer_characters();
void print_status();

static void IRAM_ATTR uart_interupt_handler(void *arg)
{
    uint16_t rx_fifo_len;

	UART0.status.ctsn;
    rx_fifo_len = UART0.status.rxfifo_cnt; // read number of bytes in UART buffer
    
    while(rx_fifo_len--)
    {
        recieve_buffer[++recieve_buffer_pos] = UART0.fifo.rw_byte;
		if (wifi_status == WIFI_PASSWORD)
		{
			if (recieve_buffer[recieve_buffer_pos] == '\r')
			{
				recieve_buffer[recieve_buffer_pos] = '\0';
				recieve_buffer[++recieve_buffer_pos] = '\r';
				uart_write_bytes(ACTIVE_UART, "\r\n", 2);
			}
			else if (recieve_buffer[recieve_buffer_pos] != '\n' && recieve_buffer[recieve_buffer_pos] != 0x08 && recieve_buffer[recieve_buffer_pos] != 0x7f)
			{
				uart_write_bytes(ACTIVE_UART, "*", 1);
			}
		}
		else if (wifi_status == WIFI_UNAME)
		{
			if (recieve_buffer[recieve_buffer_pos] == '\r')
			{
				recieve_buffer[recieve_buffer_pos] = '\0';
				recieve_buffer[++recieve_buffer_pos] = '\r';
				uart_write_bytes(ACTIVE_UART, "\r\n", 2);
			}
			else if (recieve_buffer[recieve_buffer_pos] != '\n' && recieve_buffer[recieve_buffer_pos] != 0x08 && recieve_buffer[recieve_buffer_pos] != 0x7f)
			{
				uart_write_bytes(ACTIVE_UART, &recieve_buffer[recieve_buffer_pos], 1);
			}
		}
		else if (recieve_buffer[recieve_buffer_pos] != 0x08 && recieve_buffer[recieve_buffer_pos] != 0x7f) // backspace
		{
        	uart_write_bytes(ACTIVE_UART, &recieve_buffer[recieve_buffer_pos], 1);
		}

		if (recieve_buffer[recieve_buffer_pos] == 0x08 || recieve_buffer[recieve_buffer_pos] == 0x7f) // backspace
		{
			uart_write_bytes(ACTIVE_UART, "\b \b", 3);
			recieve_buffer_pos = recieve_buffer_pos < 2 ?  UINT8_MAX : recieve_buffer_pos - 2;
		}

		if (recieve_buffer[recieve_buffer_pos] == '\r')
		{
			uart_clear_intr_status(ACTIVE_UART, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
			recieve_buffer[recieve_buffer_pos] = '\n';
			uart_write_bytes(ACTIVE_UART, &recieve_buffer[recieve_buffer_pos], 1);
			xTaskCreate(&parse_input, "parse_input", 4096, NULL, 5, &parse_handle);
			return;
		}
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
	sprintf(&send_buffer[strftime(send_buffer, BUFFER_SIZE, "| %c", &local_time) - 5], ":       \t     %3.3f °C\t|\r\n", measurment->temperature);

	uart_write_bytes(ACTIVE_UART, send_buffer, strlen(send_buffer));
	vTaskDelay(25 / portTICK_RATE_MS);
}

void uart_log_measurment(measurment_t *measurment)
{
	struct tm local_time;
	localtime_r(&measurment->time, &local_time);
	sprintf(&send_buffer[strftime(send_buffer, BUFFER_SIZE, "* log: %c", &local_time) - 5], ": %.3f °C\r\n", measurment->temperature);

	uart_write_bytes(ACTIVE_UART, send_buffer, strlen(send_buffer));
	vTaskDelay(25 / portTICK_RATE_MS);
}

static void parse_input()
{
	int32_t parsed = 0;
	uint8_t unrecognized = 0;
	recieve_buffer[++recieve_buffer_pos] = '\0';

	recieve_buffer_pos = 0;
	skip_buffer_spaces();

	if (wifi_status == WIFI_UNAME)
	{
		parsed = strlen(recieve_buffer);
		strncpy(credentials.name, recieve_buffer, parsed >= CREDENTIAL_NAME ? CREDENTIAL_NAME - 1 : parsed);
		wifi_status = WIFI_PASSWORD;
		uart_print_string("password: ");
	}
	else if (wifi_status == WIFI_PASSWORD)
	{
		parsed = strlen(recieve_buffer);
		strncpy(credentials.password, recieve_buffer, parsed >= CREDENTIAL_PASS ? CREDENTIAL_PASS - 1 : parsed);
		wifi_status = NO_WIFI;
		store_credentials();

		uart_print_string("Changing credentials ...\r\n");
		wifi_disconnect();
		wifi_connect();
		
		vTaskDelay(50 / portTICK_RATE_MS);
		print_status();
	}
	else if (recieve_buffer[recieve_buffer_pos] == 's')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "tatus", 5) && isspace(recieve_buffer[recieve_buffer_pos += 5])))
		{
			print_status();
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'p')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "rint", 4) && isspace(recieve_buffer[recieve_buffer_pos += 4])))
		{
			skip_buffer_spaces();

			parsed = atoi(&recieve_buffer[recieve_buffer_pos]);
			if (parsed <= 0)
			{
				unrecognized = 2;
			}
			else
			{
				skip_buffer_characters();
				skip_buffer_spaces();

				if (isspace(recieve_buffer[recieve_buffer_pos + 1]))
				{
					unrecognized = print_samples((uint32_t)parsed, recieve_buffer[recieve_buffer_pos]);
				}
				else
				{
					unrecognized = 4;
				}
			}
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'w')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "ifi", 3) && isspace(recieve_buffer[recieve_buffer_pos += 3])))
		{
			skip_buffer_spaces();

			if (!strncmp(&recieve_buffer[recieve_buffer_pos], "connect", 7) && isspace(recieve_buffer[recieve_buffer_pos += 7]))
			{
				uart_print_string("Connecting ...\r\n");
				if (!wifi_is_connected())
				{
					wifi_connect();
				}
				vTaskDelay(50 / portTICK_RATE_MS);
				print_status();
			}
			else if (!strncmp(&recieve_buffer[recieve_buffer_pos], "disconnect", 10) && isspace(recieve_buffer[recieve_buffer_pos += 10]))
			{
				uart_print_string("Disconnecting ...\r\n");
				if (wifi_is_connected())
				{
					wifi_disconnect();
				}
				print_status();
				
			}
			else if (!strncmp(&recieve_buffer[recieve_buffer_pos], "auth", 4) && isspace(recieve_buffer[recieve_buffer_pos += 4]))
			{
				wifi_status = WIFI_UNAME;
				memset(&credentials, 0, CREDENTIAL_SIZE);
				uart_print_string("name: ");
				
			}
			else
			{
				unrecognized = 2;
			}
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'l')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "og", 2) && isspace(recieve_buffer[recieve_buffer_pos += 2])))
		{
			skip_buffer_spaces();

			if (recieve_buffer[recieve_buffer_pos] == '0' && isspace(recieve_buffer[recieve_buffer_pos + 1]))
			{
				unrecognized = set_log_interval(0, 0);
			}
			else
			{
				parsed = atoi(&recieve_buffer[recieve_buffer_pos]);
				if (parsed <= 0)
				{
					unrecognized = 2;
				}
				else
				{
					skip_buffer_characters();
					skip_buffer_spaces();

					if (isspace(recieve_buffer[recieve_buffer_pos + 1]))
					{
						unrecognized = set_log_interval((uint32_t)parsed, recieve_buffer[recieve_buffer_pos]);
					}
					else
					{
						unrecognized = 4;
					}
				}
			}
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 'h')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "elp", 3) && isspace(recieve_buffer[recieve_buffer_pos += 3])))
		{
			skip_buffer_spaces();
			uart_print_string("+------------------------------------------------------------------------------+\r\n");
			uart_print_string("|                                 HELP MESSAGE                                 |\r\n");
			uart_print_string("+------------------------------------------------------------------------------+\r\n");
			uart_print_string("| Meaning of used notation:                                                    |\r\n");
			uart_print_string("| - <'characters'>                                                             |\r\n");
			uart_print_string("|     Marks a group of characters located at a specific position in a command. |\r\n");
			uart_print_string("| - 'characters1'|'characters2' Marks two different groups of characters.      |\r\n");
			uart_print_string("| - NL New line character (Enter key)                                          |\r\n");
			uart_print_string("| - WS* Zero or more white space characters appart from <NL>.                  |\r\n");
			uart_print_string("| - WS+ One or more white space characters appart from <NL>.                   |\r\n");
			uart_print_string("| Command line interface:                                                      |\r\n");
			uart_print_string("| - <WS*><h|help><WS*><NL> Prints this help message.                           |\r\n");
			uart_print_string("| - <WS*><s|status><WS*><NL> Prints the current status of the device.          |\r\n");
			uart_print_string("| - <WS*><l|log><WS+><1-N><WS+><s|m|h|d><WS*><NL>                              |\r\n");
			uart_print_string("|     Logs measured temperature each N seconds, minutes, hours or days.        |\r\n");
			uart_print_string("| - <WS*><l|log><WS+><0><WS*><NL> Stops temperature logging.                   |\r\n");
			uart_print_string("| - <WS*><p|print><WS+><1-N><WS+><s|m|h|d><WS*><NL>                            |\r\n");
			uart_print_string("|     Prints measured temperature of last N seconds, minutes, hours or days.   |\r\n");
			uart_print_string("| - <WS*><t|time><WS+><sync><WS*><NL> Synchronizes the device time.            |\r\n");
			uart_print_string("| - <WS*><w|wifi><WS+><connect><WS*><NL> Tries to connect to Wi-Fi.            |\r\n");
			uart_print_string("| - <WS*><w|wifi><WS+><disconnect><WS*><NL> Disconnects from Wi-Fi.            |\r\n");
			uart_print_string("| - <WS*><w|wifi><WS+><auth><WS*><NL>                                          |\r\n");
			uart_print_string("|     Asks the user for new Wi-Fi credentials and tries to connect with them.  |\r\n");
			uart_print_string("+------------------------------------------------------------------------------+\r\n");
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos] == 't')
	{
		recieve_buffer_pos++;
		if (isspace(recieve_buffer[recieve_buffer_pos]) || 
		    (!strncmp(&recieve_buffer[recieve_buffer_pos], "ime", 3) && isspace(recieve_buffer[recieve_buffer_pos += 3])))
		{
			skip_buffer_spaces();
			if (!strncmp(&recieve_buffer[recieve_buffer_pos], "sync", 4) && isspace(recieve_buffer[recieve_buffer_pos += 4]))
			{
				uart_print_string("Synchronizing...\r\n");
				set_current_time();
				print_status();
			}
			else
			{
				unrecognized = 2;
			}
		}
		else
		{
			unrecognized = 3;
		}
	}
	else if (recieve_buffer[recieve_buffer_pos - 1] != '\n')
	{
		unrecognized = 1;
	}

	if (unrecognized)
	{
		sprintf(recieve_buffer, "Command was not recognized.\r\nType 'h' or 'help' on a single line to view the help message.\r\n");
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

void uart_print_string(const char * string)
{
	uart_write_bytes(ACTIVE_UART, string, strlen(string));
	vTaskDelay(25 / portTICK_RATE_MS);
}

void print_status()
{
	uart_print_string("+-------------------------------------------------------------------------------+\r\n");
	uart_print_string("|                               DEVICE STATUS                                   |\r\n");
	uart_print_string("|-------------------------------------------------------------------------------|\r\n");
	time_t time_sec = get_start_time();
	struct tm local_time;
	uint8_t lenght = 0;
	localtime_r(&time_sec, &local_time);
	sprintf(send_buffer, "| - Temperature measuring started at ");
	lenght = strlen(send_buffer);
	strftime(&send_buffer[lenght], BUFFER_SIZE - lenght, "%c.\t\t\t|\r\n", &local_time);
	uart_print_string(send_buffer);

	time(&time_sec);
	localtime_r(&time_sec, &local_time);
	sprintf(send_buffer, "| - Current time on the device is ");
	lenght = strlen(send_buffer);
	strftime(&send_buffer[lenght], BUFFER_SIZE - lenght, "%c.\t\t\t|\r\n", &local_time);
	uart_print_string(send_buffer);

	time_sec = get_last_sync();
	localtime_r(&time_sec, &local_time);
	sprintf(send_buffer, "| - Last synchronization of time carried out at ");
	lenght = strlen(send_buffer);
	strftime(&send_buffer[lenght], BUFFER_SIZE - lenght, "%c.\t|\r\n", &local_time);
	uart_print_string(send_buffer);

	print_log_interval(send_buffer);
	if (wifi_is_connected())
	{
		sprintf(send_buffer, "| - WiFi is connected, device IP address is: %d.%d.%d.%d\t\t\t|\r\n", 
																							 s_ip_addr.addr & 0xFF,
																							 (s_ip_addr.addr >> 8) & 0xFF, 
																							 (s_ip_addr.addr >> 16) & 0xFF,
																							 s_ip_addr.addr >> 24);
		uart_print_string(send_buffer);	
	}
	else
	{
		uart_print_string("| - WiFi is not connected.\t\t\t\t\t\t\t|\r\n");	
	}
	uart_print_string("+-------------------------------------------------------------------------------+\r\n");
}
