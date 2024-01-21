#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

#include "types.hpp"

static const double32   ZERO    = 0x00,
                        ONE     = 0x01;

/* ERROR CODES */
static const uint8 SUCCESS                  = 0x00,
                   UNINITIALIZED_POINTER    = 0x01,
                   TENSOR_WRONG_SIZE        = 0x02,
                   GENERIC_ERROR_CODE       = 0x07; // We use this error code when we stumble upon an error
                                                    // but we don't care about the returned error code;

#endif /* CONSTANTS_HEADER */
