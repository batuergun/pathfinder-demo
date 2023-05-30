#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include <stdio.h>

#define I2C_SLAVE_ADDRESS 0x10
#define BUFFER_SIZE 4

volatile uint8_t motorData[BUFFER_SIZE]; // Buffer for motor data
volatile size_t bytesReceived = 0;       // Bytes received counter

void i2c_irq_handler()
{
    uint32_t status = i2c0_hw->intr_stat;

    if (status & I2C_IC_INTR_STAT_R_RD_REQ_BITS)
    {
        // Read request interrupt
        if (bytesReceived < BUFFER_SIZE)
        {
            i2c0_hw->data_cmd = motorData[bytesReceived++];
        }
        else
        {
            i2c0_hw->data_cmd = 0; // Send dummy byte if no more data available
        }
    }

    if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
    {
        // Receive FIFO full interrupt
        while (i2c0_hw->status & I2C_IC_STATUS_RFNE_BITS)
        {
            motorData[bytesReceived++] = i2c0_hw->data_cmd;
            if (bytesReceived >= BUFFER_SIZE)
            {
                // Process motor data
                printf("Motor Data: ");
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    printf("%d ", motorData[i]);
                }
                printf("\n");

                bytesReceived = 0; // Reset byte counter
            }
        }
    }

    i2c0_hw->intr_mask = status; // Clear interrupt status
}

int main()
{
    stdio_init_all();

    // Initialize the I2C peripheral
    i2c_init(i2c0, 100000);
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);

    // Set the I2C slave address
    i2c_set_slave_mode(i2c0, true, I2C_SLAVE_ADDRESS);

    // Enable I2C interrupts
    irq_set_enabled(I2C0_IRQ, true);
    irq_set_priority(I2C0_IRQ, 1);
    i2c0_hw->intr_mask = I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS;
    irq_set_exclusive_handler(I2C0_IRQ, i2c_irq_handler);

    while (true)
    {
        tight_loop_contents();
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
