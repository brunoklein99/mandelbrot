#include "window.h"
#include "dynarray.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>

DynArray jobs;
pthread_mutex_t mutex;
pthread_cond_t cond;

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

    int rendered;
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
        base = job->dy * job->hpmin + job->ymin;
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

    pthread_mutex_lock(&mutex);

    dynarray_insert(&jobs, job);

    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void render(Display* display, Window win, GC gc, struct job_t* job){
    int wdist = job->wpmax - job->wpmin;
    int hdist = job->hpmax - job->hpmin;

    for (int i = 0; i < wdist; i++) {
        for (int j = 0; j < hdist; j++) {
            //printf("accessing %d\n", i * wdist + j);
            if (job->output[i * wdist + j] == 0){
                int x = i + job->wpmin;
                int y = j + job->hpmin;
                XDrawPoint(display, win, gc, x, y);
            }
        }
    }
}

int main() {
    int width = 1000;
    int height = 1000;
    int maxiter = 10000;

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
    dynarray_init(&jobs, 100);

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
            job->rendered = False;

            pthread_create(&thread, NULL, &worker, job);
            dynarray_insert(&threads, (void *) thread);
        }
    }

    size_t to_render_count = 100;
    while (to_render_count > 0){
        pthread_mutex_lock(&mutex);

        int rendered = False;
        for (int j = 0; j < jobs.used; ++j) {
            struct job_t* job = jobs.array[j];
            if (!job->rendered){
                to_render_count--;
                job->rendered = True;
                rendered = True;
                render(display, win, gc, job);
            }
        }

        if (!rendered || jobs.used < 100){
            printf("waiting to_render_count: %zu \n", to_render_count);
            pthread_cond_wait(&cond, &mutex);
        }

        pthread_mutex_unlock(&mutex);

        XFlush(display);
    }

    for (int i = 0; i < threads.used; ++i) {
        thread = (pthread_t) threads.array[i];
        pthread_join(thread, NULL);
    }

    dynarray_free(&threads);
    dynarray_free(&jobs);

    XFlush(display);

    sleep(1000);

    XCloseDisplay(display);

    return 0;

}