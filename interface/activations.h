#ifndef ACTIVATIONS_HEADER
#define ACTIVATIONS_HEADER

#include "types.h"
#include "tensor.h"

typedef enum activations 
{
    RELU = 0x00,
    LRELU,
    PRELU,
    SOFTMAX
} activations;

/* ACTIVATION FUNCTIONS */
double32 sigmoid(double32 data);
double32 relu(double32 data);
double32 leakyRelu(double32 data);
void softmax(pTensor activations);
void activation(pTensor values, activations activation);

/* ACTIVATIONS FUNCTIONS DERIVATIVES */
double32 leakyReluDerivative(double32 data);
double32 reluDerivative(double32 activation);
double32 activationDerivative(double32 value, activations activation);

#endif /* ACTIVATIONS_HEADER */
