#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

#define I2C_CLIENT_ADDRESS 0x10
#define MOTOR_COUNT 8

// Function to map a value from one range to another
double map(double value, double in_min, double in_max, double out_min, double out_max)
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function to set the PWM frequency and duty cycle
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

int main()
{
    stdio_init_all();

    i2c_init(i2c0, 100000);                             // Initialize I2C at 100kHz
    gpio_set_function(16, GPIO_FUNC_I2C);               // Set GP16 as SDA
    gpio_set_function(17, GPIO_FUNC_I2C);               // Set GP17 as SCL
    i2c_set_slave_mode(i2c0, true, I2C_CLIENT_ADDRESS); // Set Pico as I2C client/server

    uint8_t data[8];

    // Initialize PWM
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

    while (true)
    {
        // Receive data from Pi 4
        i2c_read_blocking(i2c0, I2C_CLIENT_ADDRESS, data, 8, false);

        printf("Received data package: ");
        for (int i = 0; i < 8; i++)
        {
            printf("%d ", data[i]);
        }
        printf("\n");

        // Process the received data and write to each motor
        for (int i = 2; i <= 9; i++)
        {
            // Assuming data[i-2] represents the desired duty cycle for motor connected to pin i

            // Map the motor value from 0-100 to the PWM range 6.25-8.75
            double mappedValue = map(data[i - 2], 0, 100, 6.25, 8.75);

            // Set motor PWM frequency to 1000Hz with mapped duty cycle
            uint slice_num = pwm_gpio_to_slice_num(i);
            pwm_set_freq_duty(slice_num, i, 50, mappedValue);

            printf("Motor %d: Frequency = %d Hz, Duty Cycle = %f%%\n", i, 50, mappedValue);
        }

        // Send a response back to Pi 4
        i2c_write_blocking(i2c0, I2C_CLIENT_ADDRESS, data, 8, false);
    }

    return 0;
}

/*

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

*/
