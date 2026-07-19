#ifndef SHRINCS_FRACTAL_H
#define SHRINCS_FRACTAL_H

#include <stddef.h>

int shrincs_fractal_ring_sign(const unsigned char *msg, size_t msg_len,
                               const unsigned char **pks, const unsigned char *sk,
                               size_t ring_size, size_t fractal_depth);

int shrincs_fractal_ring_verify(const unsigned char *msg, size_t msg_len,
                                 const unsigned char **pks,
                                 size_t ring_size, size_t fractal_depth);

#endif
