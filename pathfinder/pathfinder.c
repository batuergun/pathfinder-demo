#include "pico/stdlib.h"
#include "hardware/pwm.h"

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
    sleep_ms(5000);

    gpio_set_function(3, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(3);
    uint chan = pwm_gpio_to_channel(3);

    pwm_set_freq_duty(slice_num, chan, 50, 7.5);
    pwm_set_enabled(slice_num, true);
    sleep_ms(5000);

    pwm_set_freq_duty(slice_num, chan, 50, 6.5);
    sleep_ms(1000);

    pwm_set_freq_duty(slice_num, chan, 50, 8);
    sleep_ms(2000);

    // 6.25 min
    // 7.5  mid
    // 8.25 max

    while (true)
    {
        for (int i = 0; i < 25; i++)
        {
            pwm_set_freq_duty(slice_num, chan, 50, 7.5 - (0.05 * i));
            sleep_ms(200);
        }
        sleep_ms(1000);

        for (int i = 0; i < 25; i++)
        {
            pwm_set_freq_duty(slice_num, chan, 50, 6.5 + (0.05 * i));
            sleep_ms(200);
        }

        pwm_set_freq_duty(slice_num, chan, 50, 7.5);
        sleep_ms(1000);

        for (int i = 0; i < 25; i++)
        {
            pwm_set_freq_duty(slice_num, chan, 50, 7.5 + (0.05 * i));
            sleep_ms(200);
        }
        sleep_ms(1000);

        for (int i = 0; i < 25; i++)
        {
            pwm_set_freq_duty(slice_num, chan, 50, 8.25 - (0.05 * i));
            sleep_ms(200);
        }

        pwm_set_freq_duty(slice_num, chan, 50, 7.5);
        sleep_ms(2000);

        pwm_set_freq_duty(slice_num, chan, 50, 7.7);
        sleep_ms(1000);

        pwm_set_freq_duty(slice_num, chan, 50, 7.5);
        sleep_ms(2000);
    }
}