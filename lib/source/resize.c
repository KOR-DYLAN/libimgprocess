#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "img_common.h"
#include "img_resize.h"

static void image_line_resize_by_nearest_neighbor_interpolation(img_info_t *info);
static void image_line_resize_by_bilinearinterpolation(img_info_t *info);

void image_resize(enum img_type_identifier_enum img_type,
                  enum image_resize_method_enum method,
                  uint8_t *src, int32_t src_width, int32_t src_height, 
                  uint8_t *dst, int32_t dst_width, int32_t dst_height)
{
    const process_func_t func[MAX_OF_IMG_RESIZE_METHOD] = {
        [IMG_RESIZE_BY_NEAREST_NEIGHBOR]    = image_line_resize_by_nearest_neighbor_interpolation,
        [IMG_RESIZE_BY_BILINEAR]            = image_line_resize_by_bilinearinterpolation,
        [IMG_RESIZE_BY_CUBIC]               = NULL,
    };
    int32_t row = 0;
    img_info_t info = {
        .bpp = image_get_bpp(img_type),
        .func = func[method],
        .img = {
            [IMG_ID_SRC] = {
                .buf = src,
                .width = src_width,
                .height = src_height,
                .stride = image_get_stride(image_get_bpp(img_type), src_width),
            },
            [IMG_ID_DST] = {
                .buf = dst,
                .width = dst_width,
                .height = dst_height,
                .stride = image_get_stride(image_get_bpp(img_type), dst_width),
            },
        },
    };

    for(row = 0; row < dst_height; ++row)
    {
        info.row = row;
        info.func(&info);
    }
}

void image_resize_using_paraller(enum img_type_identifier_enum img_type,
                                 enum image_resize_method_enum method,
                                 uint8_t *src, int32_t src_width, int32_t src_height, 
                                 uint8_t *dst, int32_t dst_width, int32_t dst_height)
{

}

static void image_line_resize_by_nearest_neighbor_interpolation(img_info_t *info)
{
    const int32_t channel = info->bpp / BITS_PER_BYTE;
    int32_t col = 0, i = 0;
    int32_t x = 0, y = 0;
    float rx = 0.0f, ry = 0.0f;
    uint8_t *src = NULL;
    uint8_t *dst = NULL;

    for(col = 0; col < info->img[IMG_ID_DST].width; ++col)
    {
        rx = (float)(info->img[IMG_ID_SRC].width - 1) * (float)col / (float)(info->img[IMG_ID_DST].width - 1);
        ry = (float)(info->img[IMG_ID_SRC].height - 1) * (float)info->row / (float)(info->img[IMG_ID_DST].height - 1);
        x = (int32_t)(rx + 0.5f);
        y = (int32_t)(ry + 0.5f);
        
        if(x >= info->img[IMG_ID_SRC].width)
        {
            x = info->img[IMG_ID_SRC].width - 1;
        }

        if(y >= info->img[IMG_ID_SRC].width)
        {
            y = info->img[IMG_ID_SRC].width - 1;
        }

        dst = info->img[IMG_ID_DST].buf + (info->row * info->img[IMG_ID_DST].stride) + (col * channel);
        src = info->img[IMG_ID_SRC].buf + (y * info->img[IMG_ID_SRC].stride) + (x * channel);
        for(i = 0; i < channel; ++i)
        {
            dst[i] = src[i];
        }
    }
}

static void image_line_resize_by_bilinearinterpolation(img_info_t *info)
{
    const int32_t channel = info->bpp / BITS_PER_BYTE;
    int32_t col = 0, i = 0;
    int32_t x1 = 0, y1 = 0;
    int32_t x2 = 0, y2 = 0;
    float rx = 0.0f, ry = 0.0f;
    float p = 0.0f, q = 0.0f;
    float value_f = 0.0f;
    uint8_t value_n = 0;
    uint8_t *src_a = NULL;
    uint8_t *src_b = NULL;
    uint8_t *src_c = NULL;
    uint8_t *src_d = NULL;
    uint8_t *dst = NULL;

    for(col = 0; col < info->img[IMG_ID_DST].width; ++col)
    {
        rx = (float)(info->img[IMG_ID_SRC].width - 1) * (float)col / (float)(info->img[IMG_ID_DST].width - 1);
        ry = (float)(info->img[IMG_ID_SRC].height - 1) * (float)info->row / (float)(info->img[IMG_ID_DST].height - 1);
        x1 = (int32_t)(rx);
        y1 = (int32_t)(ry);

        x2 =  x1 + 1;
        if(x2 == info->img[IMG_ID_SRC].width)
        {
            x2 = info->img[IMG_ID_SRC].width - 1;
        }

        y2 =  y1 + 1;
        if(y2 == info->img[IMG_ID_SRC].height)
        {
            y2 = info->img[IMG_ID_SRC].height - 1;
        }

        p = rx - x1;
        q = ry - y1;

        dst = info->img[IMG_ID_DST].buf + (info->row * info->img[IMG_ID_DST].stride) + (col * channel);
        src_a = info->img[IMG_ID_SRC].buf + (y1 * info->img[IMG_ID_SRC].stride) + (x1 * channel);
        src_b = info->img[IMG_ID_SRC].buf + (y1 * info->img[IMG_ID_SRC].stride) + (x2 * channel);
        src_c = info->img[IMG_ID_SRC].buf + (y2 * info->img[IMG_ID_SRC].stride) + (x1 * channel);
        src_d = info->img[IMG_ID_SRC].buf + (y2 * info->img[IMG_ID_SRC].stride) + (x2 * channel);
        for(i = 0; i < channel; ++i)
        {
            value_f = (1.0f - p) * (1.0f - q) * src_a[i] +
                      p * (1.0f - q) * src_b[i] +
                      (1.0 - p) * q * src_c[i] +
                      p * q * src_d[i];
            value_f += 0.5f;

            if(255.0f <= value_f)
            {
                value_n = 255;
            }
            else
            {
                value_n = (uint8_t)value_f;
            }

            dst[i] = value_n;
        }
    }
}
