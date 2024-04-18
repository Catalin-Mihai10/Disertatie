#include "../interface/tensor.h"
#include "../utils/logger/NNLogger.h"
#include <string.h>
#include <math.h>

/* DEFINE MACROS */
#define GetVarName(var) #var

/* INITIALIZATION OPERATIONS */

uint8 initializeTensor(pTensor T)
{
    if(T->size == (uint32)ZERO)
    {
        NNERROR("ERROR: Invalid Tensor size: %u!\n", T->size);
        return TENSOR_WRONG_SIZE;
    }

    T->data = (pDouble32) malloc((sizeof T->data) * T->size);
    
    if(T->data == NULL)
    {
        NNERROR("ERROR: Could not initialize tensor!\n");
        return UNINITIALIZED_POINTER;
    }

    return SUCCESS;
}

void zeroTensor(pTensor T)
{
    for(uint32 iterator = (uint32)ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ZERO;
    }

}

void onesTensor(pTensor T)
{
    for(uint32 iterator = (uint32)ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ONE;
    }
}

void randomizeTensor(pTensor T)
{
    srand((unsigned) time(NULL));

    for(uint32 iterator = (uint32)ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ((double32) rand() / (double32) (RAND_MAX)) * 2 - 1; 
    }
}

void heUniformInitialization(pTensor T)
{
    srand((unsigned) time(NULL));

    double32 positiveThreshold = sqrt(6.0/T->size),
             negativeThreshold = -positiveThreshold;
    for(uint32 iterator = (uint32)ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ((double32) rand() / (double32) (RAND_MAX)) * fabs(negativeThreshold - positiveThreshold) + negativeThreshold; 
    }
}

/* TENSOR OPERATIONS */
void copyTensorData(pTensor T1, pTensor T2)
{
    if(T1->size != T2->size)
    {
        NNERROR("Tensors of different size, %d -- %d!", T1->size, T2->size);
        return;
    }
 
    memcpy(T1->data, T2->data, (sizeof *T2->data) * T2->size);
}

void copyTensor(pTensor T1, pTensor T2)
{
    if((T1->size != (uint32)ZERO) && (T1->size != T2->size))
    {
        NNERROR("Tensors of different size!");
        return;
    }
    
    T1->size = T2->size;
    initializeTensor(T1);
    zeroTensor(T1);
    memcpy(T1->data, T2->data, (sizeof *T2->data) * T2->size);
}

void addScalar(pTensor T1, double32 scalar)
{
    if(T1->size == (uint32)ZERO)
    {
        NNERROR("Tensor invalid size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] += scalar;
    }
}

void substractScalar(pTensor T1, double32 scalar)
{
    if(T1->size == (uint32)ZERO)
    {
        NNERROR("Tensor invalid size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] -= scalar;
    }
}

void multiplyScalar(pTensor T1, double32 scalar)
{
    if(T1->size == (uint32)ZERO)
    {
        NNERROR("Tensor invalid size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] *= scalar;
    }
}

void divideScalar(pTensor T1, double32 scalar)
{
    if(T1->size == (uint32)ZERO)
    {
        NNERROR("Tensor invalid size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] /= scalar;
    }
}

void addTensor(pTensor T1, pTensor T2)  
{
    if((T1->size != T2->size))
    {
        NNERROR("Tensors of different size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] += T2->data[iterator];
    }
}

void substractTensor(pTensor T1, pTensor T2)
{
    if(T1->size != T2->size)
    {
        NNERROR("Tensors of different size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] -= T2->data[iterator];
    }
}

void multiplyTensor(pTensor T1, pTensor T2)
{
    if(T1->size != T2->size)
    {
        NNERROR("Tensors of different size!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        T1->data[iterator] *= T2->data[iterator];
    }
}

void squareTensor(pTensor T)
{
    if(T->size == (uint32)ZERO)
    {
        NNERROR("Tensor empty!");
        return;
    }

    for(uint32 iterator = (uint32)ZERO; ++iterator < T->size; ++iterator)
    {
        T->data[iterator] *= T->data[iterator];
    }
}

pTensor substractTensors(pTensor T1, pTensor T2)
{

    pTensor result = NULL;
    if((T1->size == ZERO) || (T2->size == ZERO) || (T1->size != T2->size))
    {
        NNERROR("Tensors are of different size: %d - %d!", T1->size, T2->size);
        return result;
    }

    result = malloc((sizeof *result));

    if(result == NULL)
    {
        NNERROR("Could not create new tensor!");
        return result;
    }

    result->size = T1->size;
    initializeTensor(result);
    zeroTensor(result);

    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        result->data[iterator] = T1->data[iterator] - T2->data[iterator];
    }
    
    return result;
}

double32 sumTensor(pTensor T)
{
    double32 result = ZERO;

    if(T->size == ZERO)
    {
        NNERROR("ERROR: Invalid tensor size!\n");
        return result;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < T->size; ++iterator)
    {
        result += T->data[iterator];
    }

    return result;
}

double32 sumTensors(pTensor T1, pTensor T2)
{
    double32 result = ZERO;

    if((T1->size == ZERO) || (T2->size == ZERO) || (T1->size != T2->size))
    {
        NNERROR("ERROR: Invalid tensors size!\n");
        return result;
    }
    
    for(uint32 iterator = (uint32)ZERO; iterator < T1->size; ++iterator)
    {
        result += T1->data[iterator] + T2->data[iterator];
    }

    return result;
}

double32 dotProduct(pTensor T1, pTensor T2)
{
    double32 result = ZERO;

    if((T1->size == ZERO) || (T2->size == ZERO) || (T1->size != T2->size))
    {
        NNERROR("ERROR: Invalid tensors size!\n");
        return result;
    }
    
    for(uint32 iterator = ZERO; iterator < T1->size; ++iterator)
    {
        result += T1->data[iterator] * T2->data[iterator];
    }

    return result;
}

/* HELPING FUNCTIONS */
void printTensor(pTensor T)
{
    printf("[ "); 
    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        printf("%*.6lf ", 5, T->data[iterator]);
    }
    printf("]\n");
}

/* TENSOR CLEANUP */
void freeTensor(pTensor T)
{
    T->size = ZERO;

    free(T->data);
    T->data = NULL;
    free(T);
    T = NULL;
}

/* TEST FUNCTIONS */
void testTensor(void)
{
    printf("--- CREATE UNIDIMENSIONAL TENSOR ---\n\n");
    tensor uT = {3, NULL};
    initializeTensor(&uT);
    assert(uT.size != ZERO);
    assert(uT.data != NULL);

    zeroTensor(&uT);
    printTensor(&uT);

    onesTensor(&uT);
    printTensor(&uT);

    randomizeTensor(&uT);
    
    printTensor(&uT);
    
    printf("\n--- UNIDIMENSIONAL TENSOR CREATED ---\n\n\n");
}

/*
void main()
{
    testTensor();
    return 0;
}
*/
