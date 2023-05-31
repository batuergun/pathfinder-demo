#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"

// define I2C addresses to be used for this peripheral
#define I2C0_PERIPHERAL_ADDR 0x10

// GPIO pins to use for I2C
#define GPIO_SDA0 16
#define GPIO_SCK0 17

#define BUFFER_SIZE 8
uint8_t bytesReceived;
uint8_t motorData[8];

// Interrupt handler implements the RAM
void i2c0_irq_handler()
{

    // Get interrupt status
    uint32_t status = i2c0->hw->intr_stat;

    if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
    {
        // Read the data (this will clear the interrupt)
        uint32_t value = i2c0->hw->data_cmd;

        motorData[bytesReceived++] = value;
        if (bytesReceived >= BUFFER_SIZE)
        {
            // Process motor data
            /*
            printf("Motor Data: ");
            for (int i = 0; i < BUFFER_SIZE; i++)
            {
                printf("%d ", motorData[i]);
            }
            printf("\n");
            */

            bytesReceived = 0; // Reset byte counter
        }
    }
}

// Main loop - initilises system and then loops while interrupts get on with processing the data
int main()
{

    // Setup I2C0 as slave (peripheral)
    i2c_init(i2c0, 100 * 1000);
    i2c_set_slave_mode(i2c0, true, I2C0_PERIPHERAL_ADDR);

    // Setup GPIO pins to use and add pull up resistors
    gpio_set_function(GPIO_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(GPIO_SCK0, GPIO_FUNC_I2C);
    gpio_pull_up(GPIO_SDA0);
    gpio_pull_up(GPIO_SCK0);

    // Enable the I2C interrupts we want to process
    i2c0->hw->intr_mask = (I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS);

    // Set up the interrupt handler to service I2C interrupts
    irq_set_exclusive_handler(I2C0_IRQ, i2c0_irq_handler);

    // Enable I2C interrupt
    irq_set_enabled(I2C0_IRQ, true);

    // Do nothing in main loop
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

/*

// Check to see if we have received data from the I2C controller
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

*/