volatile int pixel_buffer_start; // global variable
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// function defintions 
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int color);
void swap(int* val1, int* val2);
void wait();

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    int x[8], y[8], x_diag[8], y_diag[8];

    /* set front pixel buffer to start of FPGA On-chip memory */
    for (int i=0; i < 8; i++) {
        x[i] = rand() % 319;
        y[i] = rand() % 239;
        x_diag[i] = rand() % 2 * 2 - 1;
        y_diag[i] = rand() % 2 * 2 - 1;
    }

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
        clear_screen();

        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)
        for (int i=0; i<8; i++) {
            //draw circle
            plot_pixel(x[i], y[i], 0xF81F);
            plot_pixel(x[i] +1 , y[i], 0xF81F);
            plot_pixel(x[i], y[i] + 1, 0xF81F);
            plot_pixel(x[i] + 1, y[i] + 1, 0xF81F);

            if (i<7) 
                draw_line(x[i], y[i], x[i+1], y[i+1], 0xF81F);
            else 
                draw_line(x[i], y[i], x[0], y[0], 0xF81F);


            // switch if theyre at the boundary
            if (x[i] == 0) {
                x_diag[i] = 1;
            }
            else if (x[i] == 319) {
                x_diag[i] = -1;
            }
            if (y[i] == 0) {
                y_diag[i] = 1;
            }
            else if (y[i] == 239){
                y_diag[i] = -1;
            }

            // update location
            x[i]+=x_diag[i];
            y[i]+=y_diag[i];

        }
        wait(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// code for subroutines 
void clear_screen() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, 0x0000);
        }
    }
}

//Bresenham’s algorithm
void draw_line(int x0, int y0, int x1, int y1, short int color) {
    bool is_steep = abs(y1-y0) > abs(x1-x0);

    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    
    int diff_x = abs(x1 - x0);
    int diff_y = abs(y1-y0);
    int error = -(diff_x / 2);
    int y_step = 0;
    int y = y0;
    
    if (y0 < y1) {
        y_step = 1;
    }
    else {
        y_step = -1;
    }
    
    for (int x = x0; x < x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, color);
        }
        else {
            plot_pixel(x, y, color);
        }
        error+=diff_y;
        if (error >= 0) {
            y = y + y_step;
            error-=diff_x;
        }
    }
}

void swap(int* val1, int* val2) {
    int temp = *val1;
    *val1 = *val2;
    *val2 = temp;
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void wait(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // pixel buffer reg
    volatile int * status =(int *)0xFF20302C; // status register for pixel biffer

    *pixel_ctrl_ptr = 1;

    // keep reading status 
    while((*status & 0x01) != 0) {
        status = status; 
    }
    
    return;
}  
