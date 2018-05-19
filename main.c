#include "window.h"
#include "dynarray.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>

struct job_t {
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
};

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

void mandelbrot_set(struct job_t* job) {

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

    job->output = malloc(wdist * hdist * sizeof(int));

    for (i = 0; i < wdist; ++i) {
        for (j = 0; j < hdist; ++j) {
            job->output[i * wdist + j] = mandelbrot(xlin[i], ylin[j], job->maxiter);
        }
    }
}

void *worker(void* p){
    struct job_t* job = p;
    mandelbrot_set(job);
    pthread_exit(0);
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

    double xmin = -1.0;
    double xmax = 1;
    double ymin = -1;
    double ymax = 1;

    double dx = (xmax - xmin) / width;
    double dy = (ymax - ymin) / height;

    DynArray threads;
    dynarray_init(&threads, 100);

    pthread_t thread;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            struct job_t* job;
            job = malloc(sizeof(struct job_t));
            job->xmin = xmin;
            job->ymin = ymin;
            job->maxiter = maxiter;
            job->dx = dx;
            job->dy = dy;
            job->output = NULL;
            job->hpmin = width / 10 * i;
            job->hpmax = job->hpmin + 100;
            job->wpmin = height / 10 * j;
            job->wpmax = job->wpmin + 100;

            pthread_create(&thread, NULL, &worker, job);
            dynarray_insert(&threads, (void *) thread);
        }
    }

    for (int i = 0; i < threads.used; ++i) {
        thread = (pthread_t) threads.array[i];
        pthread_join(thread, NULL);
    }

    dynarray_free(&threads);

//    for (int i = 0; i < width; i++) {
//        for (int j = 0; j < height; j++) {
//            if (output[i][j] > 0){
//                XDrawPoint(display, win, gc, i, j);
//            }
//        }
//    }

    XFlush(display);

    sleep(2222222);

    XCloseDisplay(display);

    return 0;
}