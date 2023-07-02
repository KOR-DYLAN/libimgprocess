#ifndef __IMG_COMMON__H__
#define __IMG_COMMON__H__

#define BITS_PER_BYTE   (8)
#define ALIGN4          (4)
#define MAX_OF_CPU      (16)

typedef struct image_information_structure img_info_t;
typedef void(*process_func_t)(img_info_t *info);

enum img_identifier_enum
{
    IMG_ID_SRC,
    IMG_ID_DST,
    MAX_OF_IMG_ID,
};

enum img_type_identifier_enum
{
    IMG_TYPE_ID_BGR24,
    IMG_TYPE_ID_BGRA,
    IMG_TYPE_ID_YUV444,
    IMG_TYPE_ID_YUYV422,
    IMG_TYPE_ID_NV12,
    MAX_OF_IMG_TYPE_ID,
};

struct image_information_structure
{
    process_func_t func;

    int32_t row;
    int32_t bpp;
    struct
    {
        uint8_t *buf;
        int32_t width;
        int32_t height;
        int32_t stride;
    } img[MAX_OF_IMG_ID];
};

typedef struct image_message_queue_structure img_msg_queue_t;
struct image_message_queue_structure
{
    long id;
    struct image_information_structure info;
};

extern int32_t image_get_bpp(enum img_type_identifier_enum img_type);
extern int32_t image_get_padding(int32_t bpp, int32_t width);
extern int32_t image_get_stride(int32_t bpp, int32_t width);
extern int32_t image_get_size(enum img_type_identifier_enum img_type, int32_t width, int32_t height);

#endif  //!__IMG_COMMON__H__
