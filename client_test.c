#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>

// Initial main function from wikipedia article on XCB 
int main(void)
{
  xcb_connection_t    *c;
  xcb_screen_t        *s;
  xcb_window_t         w;
  xcb_gcontext_t       g;
  xcb_generic_event_t *e;
  uint32_t             mask;
  uint32_t             values[2];
  int                  done = 0;
  xcb_rectangle_t      r = { 20, 20, 60, 60 };
 
                        /* open connection with the server */
  c = xcb_connect(NULL,NULL);
  if (xcb_connection_has_error(c)) {
    printf("Cannot open display\n");
    exit(1);
  }

  const xcb_setup_t* setup = xcb_get_setup(c);
  printf( "XCB Setup:\n"
	"status : %i\n"
	"pro major : %i\n"
	"pro minor : %i\n"
	"len : %i\n"
	"rel# : %i\n"
	"res_id_base : %i\n"
	"res_id_mask : %i\n"
	"mot buf size : %i\n"
	"ven_len : %i\n"
	"max req len : %i\n"
	"roots_len : %i\n"
	"image_byte_order : %i\n"
	"bitmap bit order : %i\n"
	"bitmap scan unit : %i\n"
	"bitmap scan pad : %i\n", setup->status, setup->protocol_major_version,
		setup->protocol_minor_version, setup->length, setup->release_number,
		setup->resource_id_base, setup->resource_id_mask, setup->motion_buffer_size,
		setup->vendor_len, setup->maximum_request_length, setup->roots_len,
		setup->pixmap_formats_len, setup->image_byte_order, setup->bitmap_format_bit_order,
		setup->bitmap_format_scanline_unit, setup->bitmap_format_scanline_pad );

                        /* get the first screen */
  s = xcb_setup_roots_iterator( xcb_get_setup(c) ).data;
 
                       /* create black graphics context */
  g = xcb_generate_id(c);
  w = s->root;
  mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = s->black_pixel;
  values[1] = 0;
  xcb_create_gc(c, g, w, mask, values);
 
                       /* create window */
  w = xcb_generate_id(c);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = s->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
  xcb_create_window(c, s->root_depth, w, s->root,
                    10, 10, 100, 100, 1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual,
                    mask, values);
 
                        /* map (show) the window */
  xcb_map_window(c, w);
 
  xcb_flush(c);
 
                        /* event loop */
  while (!done && (e = xcb_wait_for_event(c))) {
    switch (e->response_type & ~0x80) {
    case XCB_EXPOSE:    /* draw or redraw the window */
      xcb_poly_fill_rectangle(c, w, g,  1, &r);
      xcb_flush(c);
      break;
    case XCB_KEY_PRESS:  /* exit on key press */
      done = 1;
      break;
    }
    free(e);
  }
                        /* close connection to server */
  xcb_disconnect(c);
 
  return 0;
}
