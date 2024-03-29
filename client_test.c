#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Initial main function from wikipedia article on XCB 
int main(void)
{
  xcb_connection_t    *c;
  xcb_window_t         w;
  xcb_gcontext_t       g;
  xcb_generic_event_t *e;
  uint32_t             mask;
  uint32_t             values[2];
  int                  done = 0;
  xcb_rectangle_t      r = { 20, 20, 60, 60 };
 
  // open connection with the server
  int screenNum;
  c = xcb_connect(NULL,&screenNum);
  if (xcb_connection_has_error(c)) {
    printf("Cannot open display\n");
    exit(1);
  }

  printf( "We are screen %i\n", screenNum );
  const xcb_setup_t* setup = xcb_get_setup(c);
  printf( "XCB Setup:\n"
	"status : %i\t\tpro major : %i\n"
	"pro minor : %i\t\tlen : %i\n"
	"rel# : %i\t\tres_id_base : %x\n"
	"res_id_mask : %x\tmot buf size : %i\n"
	"ven_len : %i\t\tmax req len : %i\n"
	"roots_len : %i\t\timage_byte_order : %i\n"
	"bitmap bit order : %i\tbitmap scan unit : %i\n"
	"bitmap scan pad : %i\n", setup->status, setup->protocol_major_version,
		setup->protocol_minor_version, setup->length, setup->release_number,
		setup->resource_id_base, setup->resource_id_mask, setup->motion_buffer_size,
		setup->vendor_len, setup->maximum_request_length, setup->roots_len,
		setup->pixmap_formats_len, setup->image_byte_order, setup->bitmap_format_bit_order,
		setup->bitmap_format_scanline_unit, setup->bitmap_format_scanline_pad );

  ///* get the first screen
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator( xcb_get_setup(c) );
  for( int i = 0; i < screenNum; ++i ) xcb_screen_next(&iter);

  xcb_screen_t* s = iter.data;
  printf( "\nScreen stats:\n"
    "width: %i\t\theight: %i\n"
    "minMaps: %i\t\tmaxMaps: %i\n"
    "backingStores: %i\tsaveUnders: %i\n"
    "root_depth: %i\t\tallowedDepths: %i\n"
    "root_visual: %i\t\troot: %i\n"
    "def cmap: %i\t\tcurMask: %x\n"
    "white: %x\t\tblack: %x\n\n", s->width_in_pixels, s->height_in_pixels,
      s->min_installed_maps, s->max_installed_maps, s->backing_stores,
      s->save_unders, s->root_depth, s->allowed_depths_len, s->root_visual,
      s->root, s->default_colormap, s->current_input_masks, s->white_pixel, 
      s->black_pixel );

  xcb_depth_iterator_t depthIterator = xcb_screen_allowed_depths_iterator(s);
  for( int i = 0; i < s->allowed_depths_len; ++i )
  {
    xcb_depth_t *depth = depthIterator.data;
    printf( "Depth entry %i\n"
      "depth: %i\t\t\tvisuals_len: %i\n", i, depth->depth, depth->visuals_len );
    xcb_depth_next(&depthIterator);

    xcb_visualtype_iterator_t visIterator = xcb_depth_visuals_iterator(depth);
    for( int j = 0; j < depth->visuals_len; ++j )
    {
      xcb_visualtype_t* vis = visIterator.data;
      printf( "\tVisual Entry %i\n"
        "\tvisID : %i\t\tclass : %i\n"
        "\tbpp : %i\t\t\tcolormap_entries : %i\n"
        "\tredMask : %x\tgreenMask : %x\n"
        "\tblueMask : %x\n", j, vis->visual_id, vis->_class, vis->bits_per_rgb_value, vis->colormap_entries, vis->red_mask, vis->green_mask, vis->blue_mask );
      xcb_visualtype_next(&visIterator);
    } 
  }

  ///* create black graphics context
  g = xcb_generate_id(c);
  w = s->root;
  mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = 0xFFFF00;//s->white_pixel;//black_pixel;
  values[1] = 0;
  xcb_create_gc(c, g, w, mask, values);
 
  ///* create window
  w = xcb_generate_id(c);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = s->black_pixel;//white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
  xcb_create_window(c, s->root_depth, w, s->root,
                    10, 10, 100, 100, 1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual,
                    mask, values);
 
  ///* map (show) the window 
  xcb_map_window(c, w);
  xcb_flush(c);
  // event loop
  while (!done && (e = xcb_wait_for_event(c))) {
    printf( "Got event %i (%x)\n", e->response_type, e->response_type );
    switch (e->response_type & ~0x80) {
    case XCB_EXPOSE:    // draw or redraw the window
      printf( "Exposing window!\n" );
        xcb_poly_fill_rectangle(c, w, g,  1, &r);
        xcb_flush(c);
//      done = 1;
      break;
    case XCB_KEY_PRESS:  // exit on key press
      done = 1;
      break;
    }
    free(e);
  }
  
  printf( "Sleeping 4 sec...\n" );
  sleep(4);
  
  ///* close connection to server */
  xcb_disconnect(c);
 
  return 0;
}
