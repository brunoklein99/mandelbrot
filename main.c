#include "window.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>

const int width = 1000;
const int height = 1000;

typedef struct {
    // where we are starting to compute the set
    double xmin;
    double ymin;

    // distance of pixel steps within set compute boundary
    double dx;
    double dy;

    // iterations
    int maxiter;

    // output buffer
    int* output;

    // boundary of pixels to compute
    int wpmin;
    int wpmax;
    int hpmin;
    int hpmax;
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

    int i, j;
    double base;

    int wdist = job->wpmax - job->wpmin;
    int hdist = job->hpmax - job->hpmin;

    double xlin[wdist];
    double ylin[hdist];

    for (i = 0; i < wdist; i++){
        base = job->dx * job->wpmin + job->xmin;
        xlin[i] = base + (i * job->dx);
    }

    for (i = 0; i < hdist; i++){
        base = job->dx * job->wpmin + job->ymin;
        ylin[i] = base + (i * job->dy);
    }

    for (i = job->wpmin; i < job->wpmax; i++) {
        for (j = job->hpmin; j < job->hpmax; j++) {
            job->output[i * width + j] = mandelbrot(xlin[i], ylin[j], job->maxiter);
        }
    }
}

int main() {
    int maxiter = 80;
    int nthread = 100;

    Display* display = XOpenDisplay(":1");
    if (display == NULL) {
        printf("cannot open display");
    }

    Window win = create_simple_window(display, width, height, 0, 0);

    GC gc = create_gc(display, win, 0);
    XSync(display, False);

    int output[width][height];

    double xmin = -1.0;
    double xmax = 1;
    double ymin = -1;
    double ymax = 1;

    double dx = (xmax - xmin) / width;
    double dy = (ymax - ymin) / height;

    job_t job;
    job.xmin = xmin;
    job.ymin = ymin;
    job.maxiter = maxiter;
    job.dx = dx;
    job.dy = dy;
    job.hpmax = 500;
    job.hpmin = 0;
    job.wpmax = 500;
    job.wpmin = 0;
    job.output = (int *) output;

    mandelbrot_set(&job);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (output[i][j] > 0){
                XDrawPoint(display, win, gc, i, j);
            }
        }
    }

    XFlush(display);

    sleep(2222222);

    XCloseDisplay(display);

    return 0;
}