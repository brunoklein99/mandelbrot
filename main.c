#include "window.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>


int mandelbrot(double creal, double cimag, int maxiter) {
    double real = creal, imag = cimag;
    int n;
    for(n = 0; n < maxiter; ++n) {
        double real2 = real*real;
        double imag2 = imag*imag;
        if (real2 + imag2 > 4.0)
            return n;
        imag = 2* real*imag + cimag;
        real = real2 - imag2 + creal;
    }
    return 0;
}

int* mandelbrot_set(double xmin, double xmax,
                    double ymin, double ymax,
                    int width, int height,
                    int maxiter,
                    int *output) {

    int i,j;

    double *xlin = (double*) malloc (width*sizeof(double));
    double *ylin = (double*) malloc (width*sizeof(double));

    double dx = (xmax - xmin)/width;
    double dy = (ymax - ymin)/height;

    for (i = 0; i < width; i++){
        xlin[i] = xmin + i * dx;
    }

    for (j = 0; j < height; j++){
        ylin[j] = ymin + j * dy;
    }

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            output[i*width + j] = mandelbrot(xlin[i], ylin[j], maxiter);
        }
    }

    free(xlin);
    free(ylin);

    return output;
}

int main() {
    int width = 1000;
    int height = 1000;
    int maxiter = 80;

    Display* display = XOpenDisplay(":1");
    if (display == NULL) {
        printf("cannot open display");
    }

    Window win = create_simple_window(display, width, height, 0, 0);

    GC gc = create_gc(display, win, 0);
    XSync(display, False);

    int *output = (int *) malloc ((width*height)*sizeof(int));

    output = mandelbrot_set(-2.0, 0.05, -1.25, 1.25, width, height, maxiter, output);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (output[i*width + j] > 0){
                XDrawPoint(display, win, gc, i, j);
            }
        }
    }

    free(output);

    XFlush(display);

    sleep(10);

    XCloseDisplay(display);

    return 0;
}