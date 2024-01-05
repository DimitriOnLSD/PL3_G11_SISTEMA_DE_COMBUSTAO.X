/*
 * File:   my_lib_ili9341.c
 * Author: lcond
 *
 * Created on September 23, 2020, 6:45 PM
 */

#include <stdlib.h>
#include "lib_ili9341.h"
#include "mcc_generated_files/mcc.h"

uint16_t global_bg_color = BLACK;

void msDelay(uint8_t ms)
{
    uint8_t k;

    for (k = 0; k < ms; k++)
    {
        __delay_ms(1);
    }
}
uint8_t sendSPI(uint8_t byte)
{
    SSPBUF = byte;
    while (!SSPSTATbits.BF)
        ;

    return SSPBUF;
}

void lcd_init(void)
{

    lcd_send(LCD_REG, ILI9341_SWRESET);
    msDelay(150);

    lcd_send(LCD_REG, 0xEF);
    lcd_send(LCD_DATA, 0x03);
    lcd_send(LCD_DATA, 0x80);
    lcd_send(LCD_DATA, 0x02);
    lcd_send(LCD_REG, 0xCF);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0XC1);
    lcd_send(LCD_DATA, 0X30);
    lcd_send(LCD_REG, 0xED);
    lcd_send(LCD_DATA, 0x64);
    lcd_send(LCD_DATA, 0x03);
    lcd_send(LCD_DATA, 0X12);
    lcd_send(LCD_DATA, 0X81);
    lcd_send(LCD_REG, 0xE8);
    lcd_send(LCD_DATA, 0x85);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0x78);
    lcd_send(LCD_REG, 0xCB);
    lcd_send(LCD_DATA, 0x39);
    lcd_send(LCD_DATA, 0x2C);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0x34);
    lcd_send(LCD_DATA, 0x02);
    lcd_send(LCD_REG, 0xF7);
    lcd_send(LCD_DATA, 0x20);
    lcd_send(LCD_REG, 0xEA);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_REG, ILI9341_PWCTR1);
    lcd_send(LCD_DATA, 0x23);
    lcd_send(LCD_REG, ILI9341_PWCTR2);
    lcd_send(LCD_DATA, 0x10);
    lcd_send(LCD_REG, ILI9341_VMCTR1);
    lcd_send(LCD_DATA, 0x3e);
    lcd_send(LCD_DATA, 0x28);
    lcd_send(LCD_REG, ILI9341_VMCTR2);
    lcd_send(LCD_DATA, 0x86); // --
    lcd_send(LCD_REG, ILI9341_MADCTL);
    lcd_send(LCD_DATA, 0x48);
    lcd_send(LCD_REG, ILI9341_PIXFMT);
    lcd_send(LCD_DATA, 0x55);
    lcd_send(LCD_REG, ILI9341_FRMCTR1);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0x18);
    lcd_send(LCD_REG, ILI9341_DFUNCTR);
    lcd_send(LCD_DATA, 0x08);
    lcd_send(LCD_DATA, 0x82);
    lcd_send(LCD_DATA, 0x27);
    lcd_send(LCD_REG, 0xF2);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_REG, ILI9341_GAMMASET);
    lcd_send(LCD_DATA, 0x01);
    lcd_send(LCD_REG, ILI9341_GMCTRP1);
    lcd_send(LCD_DATA, 0x0F);
    lcd_send(LCD_DATA, 0x31);
    lcd_send(LCD_DATA, 0x2B);
    lcd_send(LCD_DATA, 0x0C);
    lcd_send(LCD_DATA, 0x0E);
    lcd_send(LCD_DATA, 0x08);
    lcd_send(LCD_DATA, 0x4E);
    lcd_send(LCD_DATA, 0xF1);
    lcd_send(LCD_DATA, 0x37);
    lcd_send(LCD_DATA, 0x07);
    lcd_send(LCD_DATA, 0x10);
    lcd_send(LCD_DATA, 0x03);
    lcd_send(LCD_DATA, 0x0E);
    lcd_send(LCD_DATA, 0x09);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_REG, ILI9341_GMCTRN1);
    lcd_send(LCD_DATA, 0x00);
    lcd_send(LCD_DATA, 0x0E);
    lcd_send(LCD_DATA, 0x14);
    lcd_send(LCD_DATA, 0x03);
    lcd_send(LCD_DATA, 0x11);
    lcd_send(LCD_DATA, 0x07);
    lcd_send(LCD_DATA, 0x31);
    lcd_send(LCD_DATA, 0xC1);
    lcd_send(LCD_DATA, 0x48);
    lcd_send(LCD_DATA, 0x08);
    lcd_send(LCD_DATA, 0x0F);
    lcd_send(LCD_DATA, 0x0C);
    lcd_send(LCD_DATA, 0x31);
    lcd_send(LCD_DATA, 0x36);
    lcd_send(LCD_DATA, 0x0F);
    lcd_send(LCD_REG, ILI9341_SLPOUT);
    msDelay(120);
    lcd_send(LCD_REG, ILI9341_DISPON);
    msDelay(120);

    lcd_fill(global_bg_color);
}

void lcd_send(bool dc, uint8_t value)
{
    LCD_DC = dc;

    LCD_CS = 0;
    // sendSPI(value);
    SPI2_ExchangeByte(value);
    LCD_CS = 1;
}

uint8_t lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcd_set_cursor_x(x) || lcd_set_cursor_y(y))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

uint8_t lcd_set_cursor_x(uint16_t x)
{
    if (x >= LCD_WIDTH)
    {
        return EXIT_FAILURE;
    }

    lcd_send(LCD_REG, 0x2B);
    lcd_send(LCD_DATA, x >> 8);
    lcd_send(LCD_DATA, x & 0xFF);
    lcd_send(LCD_REG, 0x2c);

    return EXIT_SUCCESS;
}

uint8_t lcd_set_cursor_y(uint16_t y)
{
    if (y >= LCD_HEIGHT)
    {
        return EXIT_FAILURE;
    }

    lcd_send(LCD_REG, 0x2A);
    lcd_send(LCD_DATA, y >> 8);
    lcd_send(LCD_DATA, y & 0xFF);
    lcd_send(LCD_REG, 0x2c);

    return EXIT_SUCCESS;
}

uint8_t lcd_draw_pixel(uint16_t color)
{
    lcd_send(LCD_DATA, color >> 8);
    lcd_send(LCD_DATA, color & 0xFF);

    return EXIT_SUCCESS;
}

void lcd_fill(uint16_t bg_color)
{
    uint16_t width = LCD_WIDTH, height = LCD_HEIGHT;

    if (lcd_set_cursor(0, 0))
    {
        return;
    }

    while (height--)
    {
        while (width--)
        {
            lcd_draw_pixel(bg_color);
        }
        width = LCD_WIDTH;
    }
}

void lcd_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx = x1 - x0, sx = x0 < x1 ? 1 : -1;
    int16_t dy = y1 - y0, sy = y0 < y1 ? 1 : -1;
    int16_t err, e2;

    if (x0 > x1)
    {
        dx = -dx;
    }

    if (y1 > y0)
    {
        dy = -dy;
    }

    err = dx + dy;

    while (true)
    {
        lcd_draw_pixel_at(x0, y0, color);

        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }

        e2 = (err << 1);

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void lcd_draw_pixel_at(uint16_t x, uint16_t y, uint16_t color)
{
    if (!lcd_set_cursor(x, y))
    {
        lcd_draw_pixel(color);
    }
}

void lcd_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t y_start = y0;

    if ((y0 >= LCD_HEIGHT) || (x0 >= LCD_WIDTH))
    {
        return;
    }

    if (x1 >= LCD_WIDTH)
    {
        x1 = LCD_WIDTH;
    }

    if (y1 >= LCD_HEIGHT)
    {
        y1 = LCD_HEIGHT;
    }

    while (x0 <= x1)
    {
        lcd_set_cursor(x0++, y0);

        while (y0++ <= y1)
        {
            lcd_draw_pixel(color);
        }
        y0 = y_start;
    }
}

void lcd_draw_circle(uint16_t xm, uint16_t ym, uint16_t r, uint16_t color)
{
    int16_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2 * r);
    int16_t x = 0, y = r;

    lcd_draw_pixel_at(xm, ym + r, color);
    lcd_draw_pixel_at(xm, ym - r, color);
    lcd_draw_pixel_at(xm + r, ym, color);
    lcd_draw_pixel_at(xm - r, ym, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        lcd_draw_pixel_at(xm + x, ym + y, color);
        lcd_draw_pixel_at(xm - x, ym + y, color);
        lcd_draw_pixel_at(xm + x, ym - y, color);
        lcd_draw_pixel_at(xm - x, ym - y, color);
        lcd_draw_pixel_at(xm + y, ym + x, color);
        lcd_draw_pixel_at(xm - y, ym + x, color);
        lcd_draw_pixel_at(xm + y, ym - x, color);
        lcd_draw_pixel_at(xm - y, ym - x, color);
    }
}

void lcd_draw_filled_circle(uint16_t xm, uint16_t ym, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2 * r), x = 0, y = r;

    lcd_draw_line(xm - r, ym, xm + r, ym, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        lcd_draw_line(xm - x, ym + y, xm + x, ym + y, color);
        lcd_draw_line(xm - x, ym - y, xm + x, ym - y, color);
        lcd_draw_line(xm - y, ym + x, xm + y, ym + x, color);
        lcd_draw_line(xm - y, ym - x, xm + y, ym - x, color);
    }
}

void lcd_draw_char(uint16_t x, uint16_t y, uint16_t fIndex, uint16_t fg_color, uint16_t bg_color)
{
    uint8_t j, k, i;

    for (j = 0; j < font[fIndex]; j++)
    {
        lcd_set_cursor(x + font[fIndex] - j, y);

        for (k = 0; k < FONT_HEIGHT; k++)
        {
            for (i = 0; i < 8; i++)
            {
                if (font[(fIndex + ((font[fIndex]) << 1)) - (j << 1) - k] & (0x80 >> i))
                {
                    lcd_draw_pixel(fg_color);
                }
                else
                {
                    lcd_draw_pixel(bg_color);
                }
            }
        }
    }
}

void lcd_draw_string(uint16_t x, uint16_t y, const char *pS, uint16_t fg_color, uint16_t bg_color)
{
    uint16_t lIndex, k;

    while (*pS)
    {
        lIndex = 0;
        for (k = 0; k < (*pS - ' '); k++)
        {
            lIndex += ((font[lIndex]) << 1) + 1;
        }

        lcd_draw_char(x, y, lIndex, fg_color, bg_color);

        x += font[lIndex];

        pS++;
    }
}

/* Set current drawing window */
void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    if ((x0 > 239) || (y0 > 319) || (x1 > 239) || (y1 > 319))
        return; // Nothing if out of the screen

    lcd_send(LCD_REG, ILI9341_CASET);
    lcd_send(LCD_DATA, x0);
    lcd_send(LCD_DATA, x1);
    lcd_send(LCD_REG, ILI9341_PASET);
    lcd_send(LCD_DATA, y0);
    lcd_send(LCD_DATA, y1);
    lcd_send(LCD_REG, ILI9341_RAMWR);
}

void lcd_draw_image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Imagedata)
{
    uint16_t i, j;

    if ((y >= LCD_HEIGHT) || (x >= LCD_WIDTH))
    {
        return;
    }

    if (x + width >= LCD_WIDTH)
    {
        x = LCD_WIDTH;
    }

    if (y + height >= LCD_HEIGHT)
    {
        y = LCD_HEIGHT;
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            if (!lcd_set_cursor(x + i, y + height - j))
            {
                lcd_draw_pixel(*Imagedata++);
            }
        }
    }
}
