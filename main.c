#include <X11/Xlib.h>

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

Window create_simple_window(Display* display, int width, int height, int x, int y)
{
    int screen_num = DefaultScreen(display);
    int win_border_width = 2;
    Window win;

    /* create a simple window, as a direct child of the screen's */
    /* root window. Use the screen's black and white colors as   */
    /* the foreground and background colors of the window,       */
    /* respectively. Place the new window's top-left corner at   */
    /* the given 'x,y' coordinates.                              */
    win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                              x, y, width, height, win_border_width,
                              BlackPixel(display, screen_num),
                              WhitePixel(display, screen_num));

    /* make the window actually appear on the screen. */
    XMapWindow(display, win);

    /* flush all pending requests to the X server. */
    XFlush(display);

    return win;
}

GC
create_gc(Display* display, Window win, int reverse_video)
{
    GC gc;				/* handle of newly created GC.  */
    unsigned long valuemask = 0;		/* which values in 'values' to  */
    /* check when creating the GC.  */
    XGCValues values;			/* initial values for the GC.   */
    unsigned int line_width = 2;		/* line width for the GC.       */
    int line_style = LineSolid;		/* style for lines drawing and  */
    int cap_style = CapButt;		/* style of the line's edje and */
    int join_style = JoinBevel;		/*  joined lines.		*/
    int screen_num = DefaultScreen(display);

    gc = XCreateGC(display, win, valuemask, &values);
    if (gc < 0) {
        fprintf(stderr, "XCreateGC: \n");
    }

    /* allocate foreground and background colors for this GC. */
    if (reverse_video) {
        XSetForeground(display, gc, WhitePixel(display, screen_num));
        XSetBackground(display, gc, BlackPixel(display, screen_num));
    }
    else {
        XSetForeground(display, gc, BlackPixel(display, screen_num));
        XSetBackground(display, gc, WhitePixel(display, screen_num));
    }

    /* define the style of lines that will be drawn using this GC. */
    XSetLineAttributes(display, gc,
                       line_width, line_style, cap_style, join_style);

    /* define the fill style for the GC. to be 'solid filling'. */
    XSetFillStyle(display, gc, FillSolid);

    return gc;
}

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