#ifndef __IMG_GENERIC_QUEUE__H__
#define __IMG_GENERIC_QUEUE__H__

typedef struct image_generic_queue_structure img_generic_queue_t;

extern img_generic_queue_t *img_generic_queue_create(int32_t unit_size, int32_t capacity);
extern void img_generic_queue_destroy(img_generic_queue_t *handle);
extern int32_t img_generic_queue_enqueue(img_generic_queue_t *handle, uint8_t *data);
extern int32_t img_generic_queue_dequeue(img_generic_queue_t *handle, uint8_t *data);
extern int32_t img_generic_queue_peek(img_generic_queue_t *handle, uint8_t *data);
extern int32_t img_generic_queue_is_empty(img_generic_queue_t *handle);
extern int32_t img_generic_queue_resize(img_generic_queue_t *handle, int32_t capacity);

#endif  //!__IMG_GENERIC_QUEUE__H__
