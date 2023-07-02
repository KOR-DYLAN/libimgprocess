#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "img_common.h"
#include "img_resize.h"

static void *image_resize_thread_pool(void *arg);
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
    img_msg_queue_t msg = {
        .id = 1,
    };

    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t thread_id[MAX_OF_CPU];
    key_t msg_key[MAX_OF_CPU];
    int msq_id[MAX_OF_CPU];
    int32_t i = 0;
    int result = 0;
    int32_t id = 0;

    if(MAX_OF_CPU < cpus)
    {
        cpus = MAX_OF_CPU;
    }

    for(i = 0; i < cpus; ++i)
    {
        msg_key[i] = ftok(".", i);
        msq_id[i] = msgget(msg_key[i], IPC_CREAT | 0666);
        if(msq_id[i] < 0)
        {
            fprintf(stderr, "create message queue key is failed...\n");
            exit(-1);
        }

        result = pthread_create(&thread_id[i], NULL, image_resize_thread_pool, (void*)&msg_key[i]);
        if (result != 0)
        {
            fprintf(stderr, "create pthread is failed...\n");
            exit(1);
        }
    }

    for(row = 0; row < dst_height; ++row)
    {
        info.row = row;

        id = row % cpus;
        memcpy(&msg.info, &info, sizeof(img_info_t));

        result = msgsnd(msq_id[id], &msg, sizeof(img_info_t), IPC_NOWAIT);
        if(result == -1)
        {
            fprintf(stderr, "msgsnd is failed...(%d, %s)\n", errno, strerror(errno));
            exit(1);
        }
        // info.func(&info);
    }

    info.row = -1;
    for(i = 0; i < cpus; ++i)
    {
        memcpy(&msg.info, &info, sizeof(img_info_t));
        result = msgsnd(msq_id[i], &msg, sizeof(img_info_t), IPC_NOWAIT);
        if(result == -1)
        {
            fprintf(stderr, "msgsnd is failed...(%d, %s)\n", errno, strerror(errno));
            exit(-1);
        }
    }

    for(i = 0; i < cpus; ++i)
    {
        pthread_join(thread_id[i], NULL);
        result = msgctl(msq_id[i], IPC_RMID, NULL);
        if (result == -1)
        {
            fprintf(stderr, "remove message queue key is failed...\n");
            exit(-1);
        }
    }
}

static void *image_resize_thread_pool(void *arg)
{
    key_t msg_key = *((key_t *)arg);
    int msg_id = msgget(msg_key, IPC_CREAT | 0666);
    int result = 0;
    img_msg_queue_t msg;
    
    while(1)
    {
        result = msgrcv(msg_id, &msg, sizeof(img_info_t), 0, 0);
        if (result== -1) 
        {
            fprintf(stderr, "msgrcv is failed...\n");
            exit(-1);
        }

        if(msg.info.row != -1)
        {
            msg.info.func(&msg.info);
        }
        else
        {
            break;
        }
    }

    // pthread_exit(NULL);
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
