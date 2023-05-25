#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include <X11/extensions/Xinerama.h>


#define MAX(a, b)       ((a) > (b) ? (a) : (b))

static Display *dpy;

struct Monitor {
    int x, y, w, h;
};

static int sx, sy, sw, sh;
static struct Monitor monitor;
static Imlib_Image wallpaper;
static Window root;
static int nmonitor;
static Bool running = False;

void updategeometry () {
    nmonitor = 1;
    monitor.x = sx;
    monitor.y = sy;
    monitor.w = sw;
    monitor.h = sh;
}
void drawbg () {
    int i, w, h, nx, ny, nh, nw, tmp;
    double factor;
    Pixmap pm;
    Imlib_Image tmpImg, buffer;

    pm = XCreatePixmap(dpy, root, sw, sh, DefaultDepth(dpy, DefaultScreen(dpy)));
    if (!(buffer = imlib_create_image(sw,sh))) {
        printf("Error: Cannot allocate buffer.\n");
        exit(EXIT_FAILURE);
    }
    imlib_context_set_image(buffer);
    imlib_image_fill_rectangle(0, 0, sw, sh);
    imlib_context_set_blend(1);
    imlib_context_set_image(wallpaper);
    w = imlib_image_get_width();
	h = imlib_image_get_height();
	if (!(tmpImg = imlib_clone_image())) {
        printf("Error: Cannot clone image");
        exit(EXIT_FAILURE);
    }
	imlib_context_set_image(tmpImg);
	if (((monitor.w > monitor.h && w < h) ||
		(monitor.w < monitor.h && w > h))) {
		imlib_image_orientate(1);
		tmp = w;
		w = h;
		h = tmp;
	}
    imlib_context_set_image(buffer);

	factor = MAX((double)w / monitor.w,(double)h / monitor.h);
	nw = w / factor;
    nh = h / factor;
	nx = monitor.x + (monitor.w - nw) / 2;
	ny = monitor.y + (monitor.h - nh) / 2;

    imlib_blend_image_onto_image(tmpImg, 0, 0, 0, w, h,
					     nx, ny, nw, nh);
    imlib_context_set_image(tmpImg);
	imlib_free_image();    

    imlib_context_set_blend(0);
	imlib_context_set_image(buffer);
	imlib_context_set_drawable(root);
	imlib_render_image_on_drawable(0, 0);
	imlib_context_set_drawable(pm);
	imlib_render_image_on_drawable(0, 0);
	XSetWindowBackgroundPixmap(dpy, root, pm);
	imlib_context_set_image(buffer);
	imlib_free_image_and_decache();
	XFreePixmap(dpy, pm);
}

void run () {
    updategeometry();
    drawbg();
}

void cleanup () {
    imlib_context_set_image(wallpaper);
    imlib_free_image_and_decache();
}



void setup (int c, char* path) {
    Visual* vis;
    int screen;
    Colormap cm;
    XColor color;
    //verfificar entradas
    wallpaper = imlib_load_image_without_cache(path);
    screen = DefaultScreen(dpy);
    vis = DefaultVisual(dpy, screen);
    cm = DefaultColormap(dpy, screen);
    root = RootWindow(dpy, screen);
    XSelectInput(dpy, root, StructureNotifyMask);
    sx = sy = 0;
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);

    if(!XAllocNamedColor(dpy, cm, "#000000", &color, &color)) {
        printf("Color no reconocido\n");
        exit(EXIT_FAILURE);
    }

    imlib_context_set_display(dpy);
    imlib_context_set_visual(vis);
    imlib_context_set_colormap(cm);
    imlib_context_set_color(color.red, color.green, color.blue, 255);
}



int main (int argc, char *argv[]) {
    //agregar estructura para filtrar parametros
    if (argc < 2) {
        printf("Ingrese la ruta de la imagen\n");
        exit(EXIT_FAILURE);
    }
    if (!(dpy = XOpenDisplay(NULL))) {
        printf("wz: cannot open display\n");
        exit(EXIT_FAILURE);
    }
    setup(argc, argv[1]);
    run();
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}