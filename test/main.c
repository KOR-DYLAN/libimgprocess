#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "img_common.h"
#include "img_resize.h"

#define NANOS           (1000000000)
#define PER_MICROSEC    (1000)
#define SRC_PATH        ("/home/dylan7h/workspace/libimgprocess/gpx1920x1080.rgb")

#ifndef DEFFILEMODE
#define DEFFILEMODE     (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif  /* !DEFFILEMODE */

int main(int argc, char *argv[])
{
    int fd = 0;
    off_t file_size = 0;
    ssize_t ret = 0;
    char name[256];

    struct timespec begin, end;
    uint64_t elapsed = 0;

    int32_t src_width = 0, src_height = 0;
    int32_t src_len = 0;
    uint8_t *src = NULL;

    int32_t dst_width = 0, dst_height = 0;
    int32_t dst_len = 0;
    uint8_t *dst = NULL;

    
    fd = open(SRC_PATH, O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr, "%s file is not opend...(%d, %s)\n", SRC_PATH, errno, strerror(errno));
        exit(-1);
    }

    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    src_width = 1920;
    src_height = 1080;
    src_len = image_get_size(IMG_TYPE_ID_BGR24, src_width, src_height);
    assert(src_len == file_size);

    src = (uint8_t *)malloc(file_size);
    assert(src != NULL);

    ret = read(fd, src, src_len);
    assert(ret == src_len);

    close(fd);

    dst_width = 672;
    dst_height = 474;
    dst_len = image_get_size(IMG_TYPE_ID_BGR24, dst_width, dst_height);
    dst = (uint8_t *)malloc(dst_len);
    assert(dst != NULL);

    /* resize by nearest neighbor */
    memset(dst, 0, dst_len);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    image_resize(IMG_TYPE_ID_BGR24, IMG_RESIZE_BY_NEAREST_NEIGHBOR, src, src_width, src_height, dst, dst_width, dst_height);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = ((end.tv_sec - begin.tv_sec) * (uint64_t)NANOS) + (end.tv_nsec - begin.tv_nsec);
    printf("resize by nearest neighbor - %dx%d to %dx%d, %lu [us]\n", src_width, src_height, dst_width, dst_height, elapsed / PER_MICROSEC);
    sprintf(name, "bgr24_%dx%d_to_%dx%d_by_nearest_neighbor.rgb\n", src_width, src_height, dst_width, dst_height);
    fd = open(name, O_CREAT | O_TRUNC | O_WRONLY, DEFFILEMODE);
    if(fd < 0)
    {
        fprintf(stderr, "%s file is not opend...(%d, %s)\n", name, errno, strerror(errno));
        exit(-1);
    }
    ret = write(fd, dst, dst_len);
    assert(ret == dst_len);
    close(fd);

    /* resize by bilinear */
    memset(dst, 0, dst_len);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    image_resize(IMG_TYPE_ID_BGR24, IMG_RESIZE_BY_BILINEAR, src, src_width, src_height, dst, dst_width, dst_height);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = ((end.tv_sec - begin.tv_sec) * (uint64_t)NANOS) + (end.tv_nsec - begin.tv_nsec);
    printf("resize by bilinear - %dx%d to %dx%d, %lu [us]\n", src_width, src_height, dst_width, dst_height, elapsed / PER_MICROSEC);
    sprintf(name, "bgr24_%dx%d_to_%dx%d_by_bilinear.rgb\n", src_width, src_height, dst_width, dst_height);
    fd = open(name, O_CREAT | O_TRUNC | O_WRONLY, DEFFILEMODE);
    if(fd < 0)
    {
        fprintf(stderr, "%s file is not opend...(%d, %s)\n", name, errno, strerror(errno));
        exit(-1);
    }
    ret = write(fd, dst, dst_len);
    assert(ret == dst_len);
    close(fd);

    /* paraller resize by nearest neighbor */
    memset(dst, 0, dst_len);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    image_resize_using_paraller(IMG_TYPE_ID_BGR24, IMG_RESIZE_BY_NEAREST_NEIGHBOR, src, src_width, src_height, dst, dst_width, dst_height);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = ((end.tv_sec - begin.tv_sec) * (uint64_t)NANOS) + (end.tv_nsec - begin.tv_nsec);
    printf("paraller resize by nearest neighbor - %dx%d to %dx%d, %lu [us]\n", src_width, src_height, dst_width, dst_height, elapsed / PER_MICROSEC);
    sprintf(name, "paraller_bgr24_%dx%d_to_%dx%d_by_nearest_neighbor.rgb\n", src_width, src_height, dst_width, dst_height);
    fd = open(name, O_CREAT | O_TRUNC | O_WRONLY, DEFFILEMODE);
    if(fd < 0)
    {
        fprintf(stderr, "%s file is not opend...(%d, %s)\n", name, errno, strerror(errno));
        exit(-1);
    }
    ret = write(fd, dst, dst_len);
    assert(ret == dst_len);
    close(fd);

    /* paraller resize by bilinear */
    memset(dst, 0, dst_len);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    image_resize_using_paraller(IMG_TYPE_ID_BGR24, IMG_RESIZE_BY_BILINEAR, src, src_width, src_height, dst, dst_width, dst_height);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = ((end.tv_sec - begin.tv_sec) * (uint64_t)NANOS) + (end.tv_nsec - begin.tv_nsec);
    printf("paraller resize by bilinear - %dx%d to %dx%d, %lu [us]\n", src_width, src_height, dst_width, dst_height, elapsed / PER_MICROSEC);
    sprintf(name, "paraller_bgr24_%dx%d_to_%dx%d_by_bilinear.rgb\n", src_width, src_height, dst_width, dst_height);
    fd = open(name, O_CREAT | O_TRUNC | O_WRONLY, DEFFILEMODE);
    if(fd < 0)
    {
        fprintf(stderr, "%s file is not opend...(%d, %s)\n", name, errno, strerror(errno));
        exit(-1);
    }
    ret = write(fd, dst, dst_len);
    assert(ret == dst_len);
    close(fd);

    free(dst);
    free(src);

    return 0;
}