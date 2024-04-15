#ifndef TENSOR_HEADER
#define TENSOR_HEADER

#include "basic/libraries.h"
#include "constants.h"

/**
 * @brief   Structure that represents a rank 1 Tensor.
 * 
 * size -   An unsigned 32 bit integer that represents
 *          the tensor dimension.
 * 
 * data -   A 32 bit floating point pointer that represents
 *          the tensor data.
 */
typedef struct tensor
{
    uint32      size;
    pDouble32   data;
} tensor;

/* DEFINE POINTER TYPES */
typedef tensor*  pTensor;

/* INITIALIZATION OPERATIONS */
uint8 initializeTensor(pTensor T);

void zeroTensor(pTensor T);
void onesTensor(pTensor T);
void randomizeTensor(pTensor T);
void heUniformInitialization(pTensor T);

/* TENSOR OPERATIONS */
void copyTensorData(pTensor T1, pTensor T2);
void copyTensor(pTensor T1, pTensor T2);

void addScalar(pTensor T1, double32 scalar);
void substractScalar(pTensor T1, double32 scalar);
void multiplyScalar(pTensor T1, double32 scalar);
void divideScalar(pTensor T1, double32 scalar);

void addTensor(pTensor T1, pTensor T2);
void substractTensor(pTensor T1, pTensor T2);
void multiplyTensor(pTensor T1, pTensor T2);

void squareTensor(pTensor T);

pTensor substractVectorAndTensor(pDouble32 V, pTensor T);

double32 sumTensor(pTensor T);
double32 sumTensors(pTensor T1, pTensor T2);
pTensor substractTensors(pTensor T1, pTensor T2);

double32 dotProduct(pTensor T1, pTensor T2);

/* HELPING FUNCTIONS */
void printTensor(pTensor T);

/* TENSOR CLEANUP */
void freeTensor(pTensor T);

#endif /* TENSOR_HEADER */
