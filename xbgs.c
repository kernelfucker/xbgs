/* See LICENSE file for license details */
/* xbgs - xcb-based background setter for x */
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/randr.h>
#include "lib/stb_image.h"

#define version "0.4"

typedef enum{
	mode_scale,
	mode_fill,
	mode_center,
	mode_zoom,
} bgmt;

bgmt pm(const char *arg){
	if(strcmp(arg, "-s") == 0) return mode_scale;
	if(strcmp(arg, "-f") == 0) return mode_fill;
	if(strcmp(arg, "-c") == 0) return mode_center;
	if(strcmp(arg, "-z") == 0) return mode_zoom;
	return mode_scale;
}

void phelp(){
	puts("usage: xbgs [mode] [file]");
	puts("modes:");
	puts("  -s	image fits appropriately to the screen size");
	puts("  -f	adjusts the picture to full view by cropping the screen");
	puts("  -c	centers the image in the center of the screen");
	puts("  -z	enlarges if the image is small");
	puts("options:");
	puts("  -v	show version information");
	puts("  -h	display this");
}

xcb_atom_t get_atom(xcb_connection_t *conn, const char *name){
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 0, strlen(name), name);
	xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);
	xcb_atom_t atom = reply ? reply->atom : XCB_ATOM_NONE;
	free(reply);
	return atom;
}

int main(int argc, char *argv[]){
	if(argc < 2 || strcmp(argv[1], "-h") == 0){
		phelp();
		return 0;
	}

	if(strcmp(argv[1], "-v") == 0){
		printf("xbgs-%s\n", version);
		return 0;
	}

	if(argc < 3){
		fprintf(stderr, "there is no such argument, type -h\n");
		return 1;
	}

	bgmt mode = pm(argv[1]);
	const char *filename = argv[2];

	int img_w, img_h, channels;
	unsigned char *image = stbi_load(filename, &img_w, &img_h, &channels, 3);
	if(!image){
		fprintf(stderr, "image not loaded: %s\n", filename);
		return 1;
	}

	int screen_n;
	xcb_connection_t *conn = xcb_connect(NULL, &screen_n);
	if(xcb_connection_has_error(conn)){
		fprintf(stderr, "unable to connect x server\n");
		stbi_image_free(image);
		return 1;
	}

	const xcb_setup_t *setup = xcb_get_setup(conn);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	for(int i = 0; i < screen_n; ++i) xcb_screen_next(&iter);
	xcb_screen_t *screen = iter.data;

	int screen_w = screen->width_in_pixels;
	int screen_h = screen->height_in_pixels;

	xcb_pixmap_t pixmap = xcb_generate_id(conn);
	xcb_create_pixmap(conn, 24, pixmap, screen->root, screen_w, screen_h);

	xcb_gcontext_t gc = xcb_generate_id(conn);
	uint32_t values[2] = {screen->black_pixel, screen->white_pixel};
	xcb_create_gc(conn, gc, pixmap, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values);

	xcb_rectangle_t full = {0, 0, screen_w, screen_h};
	xcb_poly_fill_rectangle(conn, pixmap, gc, 1, &full);

	float scale_x = (float)screen_w / img_w;
	float scale_y = (float)screen_h / img_h;

	float scale = 1.0f;

	switch(mode){
		case mode_scale:  scale = scale_x; break;
		case mode_fill:   scale = fmaxf(scale_x, scale_y); break;
		case mode_center: scale = 1.0f; break;
		case mode_zoom:   scale = fminf(scale_x, scale_y); break;
	}

	int dst_w = img_w * scale;
	int dst_h = img_h * scale;
	int offset_x = (screen_w - dst_w) / 2;
	int offset_y = (screen_h - dst_h) / 2;

	for(int y = 0; y < screen_h; ++y){
		for(int x = 0; x < screen_w; ++x){
			int src_x = (int)((x - offset_x) / scale + 0.5f);
			int src_y = (int)((y - offset_y) / scale + 0.5f);
			if(src_x >= 0 && src_x < img_w && src_y >= 0 && src_y < img_h){
				int idx = (src_y * img_w + src_x) * 3;
				uint32_t pixel = (image[idx] << 16) | (image[idx+1] << 8) | image[idx+2];
				xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &pixel);
				xcb_rectangle_t r = {x, y, 1, 1};
				xcb_poly_fill_rectangle(conn, pixmap, gc, 1, &r);
			}
		}
	}

	xcb_atom_t atom_root = get_atom(conn, "_XROOTPMAP_ID");
	xcb_atom_t atom_eset = get_atom(conn, "ESETROOT_PMAP_ID");

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_root,
		XCB_ATOM_PIXMAP, 32, 1, &pixmap);
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom_eset,
		XCB_ATOM_PIXMAP, 32, 1, &pixmap);

	uint32_t attrib[] = { pixmap };
	xcb_change_window_attributes(conn, screen->root, XCB_CW_BACK_PIXMAP, attrib);

	xcb_clear_area(conn, 0, screen->root, 0, 0, 0, 0);

	xcb_set_close_down_mode(conn, XCB_CLOSE_DOWN_RETAIN_PERMANENT);
	xcb_flush(conn);

	stbi_image_free(image);

	xcb_free_gc(conn, gc);
	xcb_disconnect(conn);

	return 0;
}
