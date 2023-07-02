#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "img_generic_queue.h"

struct image_generic_queue_structure
{
    int32_t unit_size;
    int32_t total_size;
    int32_t capacity;
    int32_t len;

    uint8_t *begin;
    int32_t head_offset;
    int32_t tail_offset;

    pthread_mutex_t mutex;
};

img_generic_queue_t *img_generic_queue_create(int32_t unit_size, int32_t capacity)
{
    img_generic_queue_t *handle = NULL;

    if((0 < unit_size) && (0 < capacity))
    {
        handle = (img_generic_queue_t *)malloc(sizeof(img_generic_queue_t));
        if(handle != NULL)
        {
            handle->unit_size = unit_size;
            handle->capacity = capacity;
            handle->total_size = unit_size * capacity;
            handle->len = 0;

            handle->begin = (uint8_t *)malloc(handle->total_size);
            if(handle->begin != NULL)
            {
                handle->head_offset = 0;
                handle->tail_offset = 0;
                pthread_mutex_init(&handle->mutex, NULL);
            }
            else
            {
                free(handle);
                handle = NULL;
            }
        }
    }

    return handle;
}

void img_generic_queue_destroy(img_generic_queue_t *handle)
{
    if(handle != NULL)
    {
        pthread_mutex_destroy(&handle->mutex);
        free(handle->begin);
        memset(handle, 0, sizeof(handle));
        free(handle);
    }
}

int32_t img_generic_queue_enqueue(img_generic_queue_t *handle, uint8_t *data)
{
    int32_t ret = 0;
    int32_t capacity = 0;
    int32_t total_size = 0;
    uint8_t *temp = NULL;
    uint8_t *ptr = NULL;

    if((handle != NULL) && (data != NULL))
    {
        pthread_mutex_lock(&handle->mutex);

        if(handle->capacity <= handle->len)
        {
            capacity = handle->capacity * 2;
            total_size = handle->unit_size * capacity;
            temp = (uint8_t *)malloc(total_size);
            if(temp != NULL)
            {
                memcpy(temp, handle->begin, handle->total_size);
                free(handle->begin);
                handle->begin = temp;
                handle->capacity = capacity;
                handle->total_size = total_size;
            }
            else
            {
                ret = -1;
            }
        }

        if(ret == 0)
        {
            ptr = handle->begin + handle->head_offset;
            memcpy(ptr, data, handle->unit_size);
            handle->head_offset += handle->unit_size;
            if(handle->total_size <= handle->head_offset)
            {
                handle->head_offset = 0;
            }
            ++handle->len;
        }

        pthread_mutex_unlock(&handle->mutex);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int32_t img_generic_queue_dequeue(img_generic_queue_t *handle, uint8_t *data)
{
    int32_t ret = 0;
    uint8_t *ptr = NULL;

    if((handle != NULL) && (data != NULL))
    {
        pthread_mutex_lock(&handle->mutex);

        if(0 < handle->len)
        {
            ptr = handle->begin + handle->tail_offset;
            memcpy(data, ptr, handle->unit_size);
            handle->tail_offset -= handle->unit_size;
            if(handle->tail_offset < 0)
            {
                handle->tail_offset = handle->total_size - handle->unit_size;
            }
            --handle->len;
        }
        else
        {
            ret = -1;
        }

        pthread_mutex_unlock(&handle->mutex);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int32_t img_generic_queue_peek(img_generic_queue_t *handle, uint8_t *data)
{
    int32_t ret = 0;
    uint8_t *ptr = NULL;

    if((handle != NULL) && (data != NULL))
    {
        pthread_mutex_lock(&handle->mutex);

        if(0 < handle->len)
        {
            ptr = handle->begin + handle->tail_offset;
            memcpy(data, ptr, handle->unit_size);
        }
        else
        {
            ret = -1;
        }

        pthread_mutex_unlock(&handle->mutex);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int32_t img_generic_queue_is_empty(img_generic_queue_t *handle)
{
    int32_t ret = 0;

    if(handle != NULL)
    {
        pthread_mutex_lock(&handle->mutex);
        if(0 < handle->len)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }
        pthread_mutex_unlock(&handle->mutex);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int32_t img_generic_queue_resize(img_generic_queue_t *handle, int32_t capacity)
{
    int32_t ret = 0;
    int32_t total_size = 0;
    uint8_t *temp = NULL;
    uint8_t *ptr = NULL;
    
    if((handle != NULL) && (0 < capacity))
    {
        if(handle->len < capacity)
        {
            total_size = handle->unit_size * capacity;
            temp = (uint8_t *)malloc(total_size);
            if(temp != NULL)
            {
                memcpy(temp, handle->begin, handle->total_size);
                free(handle->begin);
                handle->begin = temp;
                handle->capacity = capacity;
                handle->total_size = total_size;
            }
            else
            {
                ret = -1;
            }
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}
