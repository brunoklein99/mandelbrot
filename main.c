#include "window.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int maxiter;
    int* output;
    int width;
    int height;
} job_t;

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

void mandelbrot_set(job_t* job) {

    int i,j;

    double *xlin = (double*) malloc (job->width*sizeof(double));
    double *ylin = (double*) malloc (job->width*sizeof(double));

    double dx = (job->xmax - job->xmin) / job->width;
    double dy = (job->ymax - job->ymin) / job->height;

    for (i = 0; i < job->width; i++){
        xlin[i] = job->xmin + i * dx;
    }

    for (j = 0; j < job->height; j++){
        ylin[j] = job->ymin + j * dy;
    }

    for (i = 0; i < job->width; i++) {
        for (j = 0; j < job->height; j++) {
            job->output[i* job->width + j] = mandelbrot(xlin[i], ylin[j], job->maxiter);
        }
    }

    free(xlin);
    free(ylin);
}

int main() {
    int width = 1000;
    int height = 1000;
    int maxiter = 80;
    int nthread = 100;

    Display* display = XOpenDisplay(":1");
    if (display == NULL) {
        printf("cannot open display");
    }

    Window win = create_simple_window(display, width, height, 0, 0);

    GC gc = create_gc(display, win, 0);
    XSync(display, False);

    int *output = (int *) malloc ((width*height)*sizeof(int));

    job_t job;
    job.maxiter = maxiter;
    job.xmin = -2.0;
    job.xmax = 0.05;
    job.ymin = 1.25;
    job.ymax = -1.25;
    job.width = width;
    job.height = height;
    job.output = output;

    mandelbrot_set(&job);

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