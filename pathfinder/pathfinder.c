#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <string.h>

#define MOTOR_COUNT 8

// Function declaration
double map_motor_value(int value);
uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t f, double d);

void set_motor_values(const int *motorValues)
{
    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        int motorValue = motorValues[i];

        // Map the input motor value (0-100) to the PWM duty cycle value (6.25-8.25)
        double mappedValue = map_motor_value(motorValue);

        // Set the PWM frequency and duty cycle using the provided function
        uint slice_num = pwm_gpio_to_slice_num(i + 2);
        uint chan = pwm_gpio_to_channel(i + 2);
        pwm_set_freq_duty(slice_num, chan, 50, mappedValue);
    }
}

double map_motor_value(int value)
{
    double mappedValue = 6.25 + (value / 100.0) * 2.0;
    return mappedValue;
}

int main()
{
    // Initialize the Pico SDK
    stdio_init_all();

    // Configure GPIO pins 3 to 10 for PWM
    for (int pin = 2; pin <= 9; ++pin)
    {
        gpio_set_function(pin, GPIO_FUNC_PWM);
    }

    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        uint slice_num = pwm_gpio_to_slice_num(i + 2);
        pwm_set_enabled(slice_num, true);
    }

    // Serial port configuration
    uart_inst_t *uart_instance = uart0; // Use UART1 for port 16 and 17
    const int baud_rate = 9600;

    // Initialize the UART
    uart_init(uart_instance, baud_rate);
    gpio_set_function(16, GPIO_FUNC_UART); // Set GPIO 16 as UART RX
    gpio_set_function(17, GPIO_FUNC_UART); // Set GPIO 17 as UART TX
    uart_set_hw_flow(uart_instance, false, false);

    // Set up the UART data format
    uart_set_format(uart_instance, 8, 1, UART_PARITY_NONE);

    // Motor values received over serial will be stored in this array
    int motorValues[MOTOR_COUNT] = {0};

    // Custom framing variables
    char startMarker = '{';
    char endMarker = '}';
    char receivedMsg[256];
    bool isUARTDataReceived = false;
    int receivedIndex = 0;

    while (true)
    {
        while (uart_is_readable(uart_instance))
        {
            char c = uart_getc(uart_instance);

            if (c == startMarker)
            {
                // Start of a new packet, reset the buffer and index
                memset(receivedMsg, 0, sizeof(receivedMsg));
                receivedIndex = 0;
                isUARTDataReceived = true;
                printf("Received new packet\n");
            }
            else if (c == endMarker && isUARTDataReceived)
            {
                // End of the packet, process the received data
                printf("Received data: %s\n", receivedMsg);

                // Tokenize the received data to extract motor values
                char *token;
                token = strtok(receivedMsg, ",");
                int motorIndex = 0;
                while (token != NULL && motorIndex < MOTOR_COUNT)
                {
                    motorValues[motorIndex] = strtol(token, NULL, 16);
                    printf("Motor %d value: %d\n", motorIndex, motorValues[motorIndex]);
                    token = strtok(NULL, ",");
                    motorIndex++;
                }

                // Update the motor values based on the received packet
                set_motor_values(motorValues);

                // Reset the flags and received index
                isUARTDataReceived = false;
                receivedIndex = 0;
            }
            else if (isUARTDataReceived)
            {
                // Append the received character to the buffer
                receivedMsg[receivedIndex++] = c;
            }
        }

        // No UART input received, set all motor values to 50
        if (!isUARTDataReceived)
        {
            for (int i = 0; i < MOTOR_COUNT; ++i)
            {
                motorValues[i] = 50;
            }

            // Update the motor values
            set_motor_values(motorValues);
            printf("No UART input received. Setting motor values to 50.\n");
        }
    }

    return 0;
}

// Set the PWM frequency and duty cycle for a given slice and channel
uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t f, double d)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / f / 4096 + (clock % (f * 4096) != 0);
    if (divider16 / 16 == 0)
        divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / f - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * d / 100);
    return wrap;
}
