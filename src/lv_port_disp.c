/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "lvgl.h"
#include <stdio.h>
#include "pico/stdlib.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#define LCD_D0 20
#define LCD_D1 21
#define LCD_D2 22
#define LCD_D3 23
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 26
#define LCD_D7 27

#define TFT_WR_HIGH gpio_set_mask(1ul << LCD_WR)
#define TFT_WR_LOW gpio_clr_mask(1ul << LCD_WR)

#define FIRST_LCD_DATA 20

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YEL0 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430

#define setxcmd 0x2A
#define setycmd 0x2B
#define wramcmd 0x2C

#define width 240
#define height 240
#define LCD_RD 18
#define LCD_WR 17
#define LCD_RS 16

#define LCD_CS 14
#define LCD_RST 15

void LCD_WriteCmd(uint8_t Reg)
{
    gpio_clr_mask(0x01 << LCD_RS);
    gpio_set_mask(Reg << FIRST_LCD_DATA);
    gpio_clr_mask((0xff - Reg) << FIRST_LCD_DATA);
    asm volatile("nop \n nop \n nop \n nop \n nop");
    gpio_set_mask(0x01 << LCD_WR);
    gpio_clr_mask(0x01 << LCD_WR);
}

void LCD_WriteData(uint8_t Data)
{
    gpio_set_mask(0x01 << LCD_RS);
    gpio_set_mask(Data << FIRST_LCD_DATA);
    gpio_clr_mask((0xff - Data) << FIRST_LCD_DATA);
    asm volatile("nop \n nop \n nop \n nop \n nop ");
    gpio_set_mask(0x01 << LCD_WR);
    gpio_clr_mask(0x01 << LCD_WR);
}

void initGpioOut(uint16_t pinNumber)
{
    gpio_init(pinNumber);
    gpio_set_dir(pinNumber, 1);
}

void writeGPIO(uint8_t pinNumber, uint8_t value)
{
    if (value)
    {
        gpio_set_mask(1ul << pinNumber);
    }
    else
    {
        gpio_clr_mask(1ul << pinNumber);
    }
}

void delayMs(uint32_t ms)
{
    sleep_ms(ms);
}

void LCD_PIN_Init()
{
    initGpioOut(LCD_RD);
    initGpioOut(LCD_WR);
    initGpioOut(LCD_RST);
    initGpioOut(LCD_RS);
    initGpioOut(LCD_CS);

    initGpioOut(LCD_D0);
    initGpioOut(LCD_D1);
    initGpioOut(LCD_D2);
    initGpioOut(LCD_D3);
    initGpioOut(LCD_D4);
    initGpioOut(LCD_D5);
    initGpioOut(LCD_D6);
    initGpioOut(LCD_D7);

    writeGPIO(LCD_D0, 0);
    writeGPIO(LCD_D1, 0);
    writeGPIO(LCD_D2, 0);
    writeGPIO(LCD_D3, 0);
    writeGPIO(LCD_D4, 0);
    writeGPIO(LCD_D5, 0);
    writeGPIO(LCD_D6, 0);
    writeGPIO(LCD_D7, 0);
}

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    LCD_WriteCmd(LCD_Reg);
    LCD_WriteData(LCD_RegValue);
}

void LCD_Reset(void)
{
    writeGPIO(LCD_RST, 1);
    delayMs(50);
    writeGPIO(LCD_RST, 0);
    delayMs(50);
    writeGPIO(LCD_RST, 1);
    delayMs(50);
}

void LCD_WriteRAM_Prepare(void)
{
    LCD_WriteCmd(wramcmd);
}

void LCD_SetCursor(uint16_t x, uint16_t y)
{
    LCD_WriteReg(0x20, y);
    LCD_WriteReg(0x21, x);
    LCD_WriteRAM_Prepare();
}

void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
    LCD_WriteCmd(setxcmd);
    LCD_WriteData(xStar >> 8);
    LCD_WriteData(0x00FF & xStar);
    LCD_WriteData(xEnd >> 8);
    LCD_WriteData(0x00FF & xEnd);

    LCD_WriteCmd(setycmd);
    LCD_WriteData(yStar >> 8);
    LCD_WriteData(0x00FF & yStar);
    LCD_WriteData(yEnd >> 8);
    LCD_WriteData(0x00FF & yEnd);
    LCD_WriteRAM_Prepare();
}

void LCD_FillScreen(uint16_t Color)
{
    uint32_t index = 0;
    LCD_SetWindows(0, 0, width - 1, height - 1);
    writeGPIO(LCD_RS, 1);
    for (index = 0; index < width * height; index++)
    {
        LCD_WriteData(Color >> 8);
        LCD_WriteData(Color);
    }
}

void LCD_Address_Set(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
    LCD_WriteCmd(setxcmd);
    LCD_WriteData(xStar >> 8);
    LCD_WriteData(0x00FF & xStar);
    LCD_WriteData(xEnd >> 8);
    LCD_WriteData(0x00FF & xEnd);

    LCD_WriteCmd(setycmd);
    LCD_WriteData(yStar >> 8);
    LCD_WriteData(0x00FF & yStar);
    LCD_WriteData(yEnd >> 8);
    LCD_WriteData(0x00FF & yEnd);
    LCD_WriteRAM_Prepare();
}

void LCD_Fill(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *color)
{
    uint32_t index = 0;
    LCD_SetWindows(xStart, yStart, xEnd, yEnd);
    writeGPIO(LCD_RS, 1);
    for (index = 0; index < (xEnd - xStart + 1) * (yEnd - yStart + 1); index++)
    {
        LCD_WriteData(*color >> 8);
        LCD_WriteData(*color);
        color++;
    }
}

void ScreenInit(void)
{
    LCD_PIN_Init();
    LCD_Reset();

    writeGPIO(LCD_RD, 1);

    LCD_WriteCmd(0x11);
    delayMs(100);       // Delay 120ms
    LCD_WriteCmd(0X36); // Memory Access Control
    LCD_WriteData(0xA0);
    LCD_WriteCmd(0X3A);
    LCD_WriteData(0X05);
    //--------------------------------ST7789S Frame rate setting-------------------------
    LCD_WriteCmd(0xb2);
    LCD_WriteData(0x0c);
    LCD_WriteData(0x0c);
    LCD_WriteData(0x00);
    LCD_WriteData(0x33);
    LCD_WriteData(0x33);

    LCD_WriteCmd(0xb7);
    LCD_WriteData(0x35);
    //---------------------------------ST7789S Power setting-----------------------------

    LCD_WriteCmd(0xbb);
    LCD_WriteData(0x35);

    LCD_WriteCmd(0xc0);
    LCD_WriteData(0x2c);

    LCD_WriteCmd(0xc2);
    LCD_WriteData(0x01);

    LCD_WriteCmd(0xc3);
    LCD_WriteData(0x13);

    LCD_WriteCmd(0xc4);
    LCD_WriteData(0x20);

    LCD_WriteCmd(0xc6);
    LCD_WriteData(0x0f);

    LCD_WriteCmd(0xca);
    LCD_WriteData(0x0f);

    LCD_WriteCmd(0xc8);
    LCD_WriteData(0x08);

    LCD_WriteCmd(0x55);
    LCD_WriteData(0x90);

    LCD_WriteCmd(0xd0);
    LCD_WriteData(0xa4);
    LCD_WriteData(0xa1);
    //--------------------------------ST7789S gamma setting------------------------------

    LCD_WriteCmd(0xe0);
    LCD_WriteData(0xd0);
    LCD_WriteData(0x00);
    LCD_WriteData(0x06);
    LCD_WriteData(0x09);
    LCD_WriteData(0x0b);
    LCD_WriteData(0x2a);
    LCD_WriteData(0x3c);
    LCD_WriteData(0x55);
    LCD_WriteData(0x4b);
    LCD_WriteData(0x08);
    LCD_WriteData(0x16);
    LCD_WriteData(0x14);
    LCD_WriteData(0x19);
    LCD_WriteData(0x20);

    LCD_WriteCmd(0xe1);
    LCD_WriteData(0xd0);
    LCD_WriteData(0x00);
    LCD_WriteData(0x06);
    LCD_WriteData(0x09);
    LCD_WriteData(0x0b);
    LCD_WriteData(0x29);
    LCD_WriteData(0x36);
    LCD_WriteData(0x54);
    LCD_WriteData(0x4b);
    LCD_WriteData(0x0d);
    LCD_WriteData(0x16);
    LCD_WriteData(0x14);
    LCD_WriteData(0x21);
    LCD_WriteData(0x20);

    LCD_WriteCmd(0x29);
    LCD_WriteReg(0x36, 0x64);
    initGpioOut(19);
    initGpioOut(10);
    writeGPIO(10, 0);
    writeGPIO(19, 1);
    LCD_FillScreen(0xffff);
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t *color)
{
    LCD_SetCursor(x, y);
    LCD_WriteData((uint16_t)*color >> 8);
    LCD_WriteData((uint16_t)*color);
}

void LCD_DrawLine(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd, uint16_t color)
{
}

void LCD_DisplayOn()
{
    LCD_WriteCmd(0X29);
}

void LCD_DisplayOff()
{
    LCD_WriteCmd(0X28);
}

static void disp_init(void)
{
    ScreenInit();
};

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    LCD_Fill(area->x1, area->y1, area->x2, area->y2, (uint16_t *)color_p);
    lv_disp_flush_ready(disp_drv);
};

//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#define MY_DISP_HOR_RES (240)
#define MY_DISP_VER_RES (240)
void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 100];                             /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 100); /*Initialize the display buffer*/

    /* Example for 2) */
    //static lv_disp_draw_buf_t draw_buf_dsc_2;
    //static lv_color_t buf_2_1[MY_DISP_HOR_RES * 50 ];                                /*A buffer for 10 rows*/
    //static lv_color_t buf_2_2[MY_DISP_HOR_RES * 50];                                /*An other buffer for 10 rows*/
    //lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 50); /*Initialize the display buffer*/

    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    //static lv_disp_draw_buf_t draw_buf_dsc_3;
    //static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES/2];                               /*A screen sized buffer*/
    //static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES/2];                               /*Another screen sized buffer*/
    //lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_VER_RES * MY_DISP_HOR_RES); /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
