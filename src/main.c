#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_conf.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "pico.h"
#include "hardware/vreg.h"
#include <Arduino.h>

#define LDO_0_9V 0
#define LDO_1_2V 1
#define LDO_1_5V 2
#define LDO_1_8V 3
#define LDO_2_0V 4
#define LDO_2_5V 5
#define LDO_2_8V 6
#define LDO_3_3V 7

#define ADC_CLOCK_OUT 13

#define K 8
#define N (1 << K)

#define RCA 1

bool timerCallback(struct repeating_timer *t)
{
    lv_tick_inc(1);
    return true;
}

static lv_obj_t *chart;
struct repeating_timer timer0;
struct repeating_timer timer1;
int16_t adc[N];
int16_t fftVaule[N / 2];

typedef struct
{
    float r;
    float i;
} complex;

static complex w[N / 2];
static complex dat[N];

static void butter_fly(complex *a, complex *b, const complex *c)
{
    complex bc;
    bc.r = b->r * c->r - b->i * c->i;
    bc.i = b->r * c->i + b->i * c->r;
    b->r = a->r - bc.r;
    b->i = a->i - bc.i;
    a->r += bc.r;
    a->i += bc.i;
}

static uint32_t bits_reverse(uint32_t index, uint32_t bits)
{
    uint32_t left, right;
    left = index << 16;
    right = index >> 16;
    index = left | right;
    left = (index << 8) & 0xff00ff00;
    right = (index >> 8) & 0x00ff00ff;
    index = left | right;
    left = (index << 4) & 0xf0f0f0f0;
    right = (index >> 4) & 0x0f0f0f0f;
    index = left | right;
    left = (index << 2) & 0xc3c3c3c3;
    right = (index >> 2) & 0x3c3c3c3c;
    index = left | right;
    left = (index << 1) & 0xa5a5a5a5;
    right = (index >> 1) & 0x5a5a5a5a;
    index = left | right;
    return index >> (32 - bits);
}

static void fft_k(complex *dat, const complex *w, uint32_t k, uint32_t k_all)
{
    uint32_t i, j;
    complex *dat1;
    k_all = 1 << (k_all - k - 1);
    k = 1 << k;
    dat1 = dat + k;
    for (i = 0; i < k_all; i++)
    {
        for (j = 0; j < k; j++)
        {
            butter_fly(dat++, dat1++, w + j * k_all);
        }
        dat += k;
        dat1 += k;
    }
}

void fft_init(complex *w, uint32_t k)
{
    float beta = 0.0f, dbeta = 3.1415926535f / k;
    for (; k; k--)
    {
        w->r = cosf(beta);
        w->i = sinf(beta);
        beta += dbeta;
        w++;
    }
}

void fft(complex *dat, const complex *w, uint32_t k)
{
    uint32_t i, j, n;
    complex temp;
    n = 1 << k;
    for (i = 0; i < n; i++)
    {
        j = bits_reverse(i, k);
        if (i <= j)
        {
            continue;
        }
        temp = dat[i];
        dat[i] = dat[j];
        dat[j] = temp;
    }
    for (i = 0; i < k; i++)
    {
        fft_k(dat, w, i, k);
    }
}

void setLDO(int gpio, int count)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_clr_mask(0x01 << gpio);
    sleep_ms(5);
    for (int i = 0; i < count; i++)
    {
        gpio_set_mask(0x01 << gpio);
        sleep_us(200);
        gpio_clr_mask(0x01 << gpio);
        sleep_us(200);
    }
    gpio_set_mask(0x01 << gpio);
}

void initADC()
{
    gpio_init(1);
    gpio_init(2);
    gpio_init(3);
    gpio_init(4);
    gpio_init(5);
    gpio_init(6);
    gpio_init(7);
    gpio_init(8);
    gpio_init(9);
    gpio_init(10);
    gpio_init(11);
    gpio_init(12);
    gpio_init(13);
    gpio_set_dir(1, GPIO_IN);
    gpio_set_dir(2, GPIO_IN);
    gpio_set_dir(3, GPIO_IN);
    gpio_set_dir(4, GPIO_IN);
    gpio_set_dir(5, GPIO_IN);
    gpio_set_dir(6, GPIO_IN);
    gpio_set_dir(7, GPIO_IN);
    gpio_set_dir(8, GPIO_IN);
    gpio_set_dir(9, GPIO_IN);
    gpio_set_dir(10, GPIO_IN);
    gpio_set_dir(11, GPIO_IN);
    gpio_set_dir(12, GPIO_IN);
    gpio_set_dir(13, GPIO_OUT);
}

_Bool adcFlag = false;
static float oldData = 0.0f;
static int avg = 0;

bool getADC(struct repeating_timer *t)
{
    for (uint16_t i = 0; i < N; i++)
    {

        gpio_set_mask(0x01 << ADC_CLOCK_OUT);
        asm volatile("nop \n nop");
        gpio_clr_mask(0x01 << ADC_CLOCK_OUT);
        asm volatile("nop ");
        adc[i] = sio_hw->gpio_in;
    }

    for (uint16_t i = 0; i < N; i++)
    {

        uint32_t tmp = adc[i];

        uint16_t readValue = !!((1ul << 1) & tmp) << 11;
        readValue += !!((1ul << 2) & tmp) << 10;
        readValue += !!((1ul << 3) & tmp) << 9;
        readValue += !!((1ul << 4) & tmp) << 8;
        readValue += !!((1ul << 5) & tmp) << 7;
        readValue += !!((1ul << 6) & tmp) << 6;
        readValue += !!((1ul << 7) & tmp) << 5;
        readValue += !!((1ul << 8) & tmp) << 4;
        readValue += !!((1ul << 9) & tmp) << 3;
        readValue += !!((1ul << 10) & tmp) << 2;
        readValue += !!((1ul << 11) & tmp) << 1;
        readValue += !!((1ul << 12) & tmp);
        dat[i].r = RCA * readValue + (1.0f - RCA) * oldData;
        oldData = RCA * readValue + (1.0f - RCA) * oldData;
        adc[i] = oldData;
        dat[i].i = 0.0f;
        avg += oldData;
    }
    fft_init((complex *)w, N / 2);
    fft((complex *)dat, (const complex *)w, K);

    adcFlag = true;
    return true;
}



int main()
{
    int max = 0;
    float max_value = 0;
    
    setLDO(29, LDO_1_2V);
    setLDO(28, LDO_1_2V);
    initADC();
    stdio_init_all();

    vreg_set_voltage(VREG_VOLTAGE_1_20); //300MHz需要加到1.35v
    set_sys_clock_khz(280 * 1000, true);

    add_repeating_timer_ms(1, timerCallback, NULL, &timer0);
    add_repeating_timer_ms(20, getADC, NULL, &timer1);

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, 240, 200);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 10);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)(0), (lv_coord_t)(4096));
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
    lv_chart_series_t *ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    //lv_chart_series_t *ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    uint32_t pcnt = sizeof(fftVaule) / sizeof(fftVaule[0]);
    //uint32_t pcnt1 = sizeof(adc) / sizeof(adc[0]);
    lv_chart_set_point_count(chart, pcnt);
    //lv_chart_set_point_count(chart, pcnt1);
    lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)adc);
    //lv_chart_set_ext_y_array(chart, ser1, (lv_coord_t *)fftVaule);

    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(label1, "Vmax:0.0V Vmin:0.0V");
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label2, true);
    lv_label_set_text_fmt(label2, "#ff0000 Freq: %d Khz# #00ff00 Vavg: %d V#", 6*max , avg);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 0);

    while (1)
    {
        lv_task_handler();
        if (adcFlag)
        {
            for (uint16_t i = 0; i < N / 2; i++)
            {
                fftVaule[i] = dat[i].r;
                if (fftVaule[i] > max_value)
                {
                    max_value = fftVaule[i];
                    max = i;
                }
            }
            lv_label_set_text_fmt(label2, "#ff0000 Freq: %d Khz# #00ff00 Vavg: %d V#", 24*max , avg / 256);
            lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)adc);
            //lv_chart_set_ext_y_array(chart, ser1, (lv_coord_t *)fftVaule);
            avg = 0;
            max = 0;
            max_value = 0;
            adcFlag = false;
        }
    }
}
