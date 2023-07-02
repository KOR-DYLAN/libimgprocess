#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "img_common.h"

int32_t image_get_bpp(enum img_type_identifier_enum img_type)
{
    const int32_t bpp[MAX_OF_IMG_TYPE_ID] = {
        [IMG_TYPE_ID_BGR24]     = 24, 
        [IMG_TYPE_ID_BGRA]      = 32, 
        [IMG_TYPE_ID_YUV444]    = 24, 
        [IMG_TYPE_ID_YUYV422]   = 16, 
        [IMG_TYPE_ID_NV12]      = 12, 
    };

    return bpp[img_type];
}

int32_t image_get_padding(int32_t bpp, int32_t width)
{
    int32_t more = ((bpp * width) / BITS_PER_BYTE) % ALIGN4;
    int32_t padding = 0;

    if(more != 0)
    {
        padding = ALIGN4 - more;
    }
    
    return padding;
}

int32_t image_get_stride(int32_t bpp, int32_t width)
{
    int32_t padding = image_get_padding(bpp, width);
    int32_t stride = ((bpp * width) / BITS_PER_BYTE) + padding;

    return stride;
}

int32_t image_get_size(enum img_type_identifier_enum img_type, int32_t width, int32_t height)
{
    int32_t bpp = image_get_bpp(img_type);
    int32_t stride = image_get_stride(bpp, width);
    int32_t size = stride * height;

    return size;
}
