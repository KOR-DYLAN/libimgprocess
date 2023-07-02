#ifndef __IMG_RESIZE__H__
#define __IMG_RESIZE__H__

enum image_resize_method_enum
{
    IMG_RESIZE_BY_NEAREST_NEIGHBOR,
    IMG_RESIZE_BY_BILINEAR,
    IMG_RESIZE_BY_CUBIC,
    MAX_OF_IMG_RESIZE_METHOD,
};

extern void image_resize(enum img_type_identifier_enum img_type,
                  enum image_resize_method_enum method,
                  uint8_t *src, int32_t src_width, int32_t src_height, 
                  uint8_t *dst, int32_t dst_width, int32_t dst_height);

extern void image_resize_using_paraller(enum img_type_identifier_enum img_type,
                                        enum image_resize_method_enum method,
                                        uint8_t *src, int32_t src_width, int32_t src_height, 
                                        uint8_t *dst, int32_t dst_width, int32_t dst_height);

#endif  //!__IMG_RESIZE__H__
