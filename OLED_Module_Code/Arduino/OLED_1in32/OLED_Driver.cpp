/*****************************************************************************
* | File      	:   OLED_Driver.cpp
* | Author      :   Waveshare team
* | Function    :   1.32inch OLED Module Drive function
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2023-03-03
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "OLED_Driver.h"
#include "stdio.h"

/*******************************************************************************
function:
            Hardware reset
*******************************************************************************/
static void OLED_Reset(void)
{
    OLED_RST_1;
    Driver_Delay_ms(100);
    OLED_RST_0;
    Driver_Delay_ms(100);
    OLED_RST_1;
    Driver_Delay_ms(100);
}

/*******************************************************************************
function:
            Write register address and data
*******************************************************************************/
static void OLED_WriteReg(uint8_t Reg)
{
#if USE_SPI_4W
    OLED_DC_0;
    OLED_CS_0;
    SPI4W_Write_Byte(Reg);
    OLED_CS_1;
#elif USE_IIC
    I2C_Write_Byte(Reg,IIC_CMD);
#endif
}

static void OLED_WriteData(uint8_t Data)
{   
#if USE_SPI_4W
    OLED_DC_1;
    OLED_CS_0;
    SPI4W_Write_Byte(Data);
    OLED_CS_1;
#elif USE_IIC   
    I2C_Write_Byte(Data,IIC_RAM);
#endif
}

/*******************************************************************************
function:
        Common register initialization
*******************************************************************************/
static void OLED_InitReg(void)
{
    OLED_WriteReg(0xae);  // turn off oled panel

    OLED_WriteReg(0xa0);  // set re-map       
    OLED_WriteReg(0x51);          

    OLED_WriteReg(0xa1);  // set display start line 
    OLED_WriteReg(0x00);

    OLED_WriteReg(0xa2);  // set display offset 
    OLED_WriteReg(0x20);    

    OLED_WriteReg(0xa4);  // normal display

    OLED_WriteReg(0xa8);  // set multiplex ratio 
    OLED_WriteReg(0x5f);

    OLED_WriteReg(0xab);  // function selection A 
    OLED_WriteReg(0x01);  // enable internal VDD regulator

    OLED_WriteReg(0x81);  // set contrast
    OLED_WriteReg(0x77);    

    OLED_WriteReg(0xb1);  // set phase length 
    OLED_WriteReg(0x31);  

    OLED_WriteReg(0xb3);  // set front clock divider/oscillator frequency
    OLED_WriteReg(0xb1);

    OLED_WriteReg(0xb4);  // For brightness enhancement
    OLED_WriteReg(0xb5);          

    OLED_WriteReg(0xb6);  // Set Second pre-charge Period
    OLED_WriteReg(0X0d);      

    OLED_WriteReg(0xbc);  // Set Pre-charge voltage
    OLED_WriteReg(0x07);      

    OLED_WriteReg(0xbe);  // set vcomh
    OLED_WriteReg(0x07);       

    OLED_WriteReg(0xd5);  // Function Selection B
    OLED_WriteReg(0x02);  // Enable second pre-charge       

}

/********************************************************************************
function:
            initialization
********************************************************************************/
void OLED_1in32_Init(void)
{
    //Hardware reset
    OLED_Reset();

    //Set the initialization register
    OLED_InitReg();
    Driver_Delay_ms(200);

    //Turn on the OLED display
    OLED_WriteReg(0xAF);
}

/********************************************************************************
function:   Set the display Window(Xstart, Ystart, Xend, Yend)
parameter:
        xStart :   X direction Start coordinates
        Ystart :   Y direction Start coordinates
        Xend   :   X direction end coordinates
        Yend   :   Y direction end coordinates
********************************************************************************/
static void OLED_SetWindow(UBYTE Xstart, UBYTE Ystart, UBYTE Xend, UBYTE Yend)
{
    if((Xstart > OLED_1in32_WIDTH) || (Ystart > OLED_1in32_HEIGHT) ||
       (Xend > OLED_1in32_WIDTH) || (Yend > OLED_1in32_HEIGHT))
        return;

    OLED_WriteReg(0x15);
    OLED_WriteReg(Xstart/2);
    OLED_WriteReg(Xend/2 - 1);

    OLED_WriteReg(0x75);
    OLED_WriteReg(Ystart);
    OLED_WriteReg(Yend - 1);
}

/********************************************************************************
function:
            Clear screen
********************************************************************************/
void OLED_1in32_Clear(UBYTE color)
{
    UWORD i;
    OLED_SetWindow(0, 0, 128, 96);
    for(i=0; i<OLED_1in32_WIDTH*OLED_1in32_HEIGHT/2; i++){
        OLED_WriteData(color | (color << 2) | (color << 4) | (color << 6));
    }
}

/********************************************************************************
function:   Update all memory to OLED
********************************************************************************/
void OLED_1in32_Display(const UBYTE *Image)
{
    UWORD i, j, temp;
    OLED_SetWindow(0, 0, 128, 96);
    for(i=0; i<OLED_1in32_HEIGHT; i++)
        for(j=0; j<OLED_1in32_WIDTH/2; j++)
        {
            temp = pgm_read_byte(&Image[j + i*64]);
            OLED_WriteData(temp);
        }
}

/********************************************************************************
function:   Update all memory to OLED
********************************************************************************/
void OLED_1in32_Display_Part(const UBYTE *Image, UBYTE Xstart, UBYTE Ystart, UBYTE Xend, UBYTE Yend)
{
    UWORD i, j, temp;
    OLED_SetWindow(Xstart, Ystart, Xend, Yend);
    for(i=0; i<Yend-Ystart; i++)
        for(j=0; j<(Xend-Xstart)/2; j++)
        {
            temp = Image[j + i*(Xend-Xstart)/2];
            OLED_WriteData(temp);
        }
}
