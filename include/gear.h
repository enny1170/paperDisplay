
/*******************************************************************************
* image
* filename: unsaved
* name: Gear
*
* preset name: Monochrome
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Threshold Dither 128
* bits per pixel: 1
*
* preprocess:
*  main scan direction: left_to_right
*  line scan direction: forward
*  inverse: no
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
*/
#include <stdint.h>



static const uint8_t image_data_Gear[128] = {
    0xff, 0xfc, 0x3f, 0xff, 
    0xff, 0xf8, 0x1f, 0xff, 
    0xff, 0xfb, 0xcf, 0xff, 
    0xfc, 0x7b, 0xcf, 0x3f, 
    0xf8, 0x3b, 0xce, 0x1f, 
    0xf3, 0x93, 0xcc, 0xcf, 
    0xe7, 0xc3, 0xc1, 0xe7, 
    0xe7, 0xef, 0xf3, 0xe7, 
    0xf3, 0xff, 0xff, 0xc7, 
    0xf9, 0xff, 0xff, 0x8f, 
    0xfc, 0xfe, 0x3f, 0x1f, 
    0xfc, 0xf8, 0x0f, 0x3f, 
    0xc1, 0xf3, 0xe7, 0x83, 
    0x81, 0xe7, 0xf7, 0x80, 
    0xbf, 0xef, 0xf3, 0xfc, 
    0xbf, 0xef, 0xf3, 0xfc, 
    0xbf, 0xef, 0xf3, 0xfc, 
    0xbf, 0xef, 0xf7, 0xfc, 
    0x81, 0xe7, 0xe7, 0x80, 
    0xc1, 0xf3, 0xcf, 0x83, 
    0xfc, 0xf8, 0x1f, 0x3f, 
    0xfc, 0xfe, 0x7f, 0x9f, 
    0xf9, 0xff, 0xff, 0xcf, 
    0xf3, 0xff, 0xff, 0xe7, 
    0xe7, 0xcf, 0xf3, 0xf7, 
    0xe7, 0x83, 0xc1, 0xe7, 
    0xf3, 0x13, 0xcc, 0xcf, 
    0xf8, 0x3b, 0xce, 0x1f, 
    0xfc, 0x7b, 0xcf, 0x3f, 
    0xff, 0xfb, 0xcf, 0xff, 
    0xff, 0xf8, 0x0f, 0xff, 
    0xff, 0xfc, 0x1f, 0xff
};
// const tImage Gear = { image_data_Gear, 32, 32,
//     8 };

