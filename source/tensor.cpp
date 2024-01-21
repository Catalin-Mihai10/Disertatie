#include "../interface/tensor.hpp"
#include <cstdlib>

/* DEFINE MACROS */
#define GetVarName(var) #var

/* INITIALIZATION OPERATIONS */

uint8 initializeTensor(pUniTensor T)
{
    if(T->size == ZERO)
    {
        printf("ERROR: Invalid Tensor size: %u!\n", T->size);
        return TENSOR_WRONG_SIZE;
    }

    T->data = (pDouble32) malloc((sizeof T->data) * T->size);
    
    if(T->data == NULL)
    {
        printf("ERROR: Could not initialize tensor!\n");
        return UNINITIALIZED_POINTER;
    }

    return SUCCESS;
}

uint8 initializeTensor(pBiTensor T)
{
    if((T->rows == ZERO) || (T->columns == ZERO))
    {
        printf("ERROR: Invalid Tensor size!\n");
        return TENSOR_WRONG_SIZE;
    }

    T->tensor.size = T->rows * T->columns;
    
    uint8 result = initializeTensor(&T->tensor);
    if((result != SUCCESS) && (result == TENSOR_WRONG_SIZE))
    {
           return TENSOR_WRONG_SIZE;
    
    } else return UNINITIALIZED_POINTER;

    return SUCCESS;
}

uint8 initializeTensor(pTensor T)
{
    if((T->slices == ZERO) || (T->rows == ZERO) || (T->columns == ZERO))
    {
        printf("ERROR: Invalid Tensor size!\n");
        return TENSOR_WRONG_SIZE;
    }
    
    T->size = T->slices * T->rows * T->columns;
    T->offsets = (pUInt32) malloc((sizeof T->offsets) * T->slices);
    T->data = (pDouble32) malloc((sizeof T->data) * T->slices * T->rows * T->columns);

    if((T->offsets == NULL) || (T->data == NULL))
    {
        printf("ERROR: Could not initialize tensor!\n");
        return UNINITIALIZED_POINTER;
    }
    
    T->offsets[0] = ZERO;
    for(uint32 slice = ONE; slice < T->slices; ++slice)
    {
        T->offsets[slice] = T->offsets[slice - 1] + (T->rows * T->columns); 
    }

    return SUCCESS;
}


/*
 * TODO: memset may not assure that the values of the 
 * tensor will be initialized to floating point 0.0,
 * try to see if there is a better approach to this
 * and not less efficient then memset.
*/
  void zeroTensor(pUniTensor T)
{
    memset(T->data, ZERO, (sizeof T->data) * T->size);
}

void zeroTensor(pBiTensor T)
{
    zeroTensor(&T->tensor);
}

void zeroTensor(pTensor T)
{
    memset(T->data, ZERO, (sizeof T->data) * T->slices * T->rows * T->slices);
}

void onesTensor(pUniTensor T)
{
    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ONE;
    }
}

void onesTensor(pBiTensor T)
{
    onesTensor(&T->tensor);
}

void onesTensor(pTensor T)
{
    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ONE;
    }
}

void randomizeTensor(pUniTensor T)
{
    srand((unsigned) time(NULL));

    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = (double32) rand() / (double32) (RAND_MAX / 1.0); 
    }
}

void randomizeTensor(pBiTensor T)
{
    randomizeTensor(&T->tensor);
}

void randomizeTensor(pTensor T)
{
    srand((unsigned) time(NULL));
    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        T->data[iterator] = (double32) rand() / (double32) (RAND_MAX / 1.0);
    }
}

/* TENSOR OPERATIONS */
double32 dotProduct(pUniTensor T1, pUniTensor T2)
{
    double32 result = ZERO;

    if((T1->size == ZERO) || (T2->size == ZERO) || (T1->size != T2->size))
    {
        printf("ERROR: Invalid tensors size!\n");
        return result;
    }
    
    for(uint32 iterator = ZERO; iterator < T1->size; ++iterator)
    {
        result += T1->data[iterator] + T2->data[iterator];
    }

    return result;
}

pBiTensor dotProduct(pBiTensor T1, pBiTensor T2)
{
    pBiTensor resultedTensor;

    if(T1->columns != T2->rows)
    {
        printf("ERROR: Invalid tensors size!\n");
        return resultedTensor;
    }
    
    resultedTensor = (pBiTensor) malloc((sizeof *resultedTensor));
    if(resultedTensor == NULL)
    {
        printf("ERROR: Could not create new tensor!\n");
        return resultedTensor;
    }

    resultedTensor->rows =    T1->rows;
    resultedTensor->columns = T2->columns;
    initializeTensor(resultedTensor);

    for(uint32 row = ZERO; row < resultedTensor->rows; ++row)
    {
        for(uint32 iterator = ZERO; iterator < T1->columns; ++iterator)
        {
            for(uint32 column = ZERO; column < resultedTensor->columns; ++column)
            {
                resultedTensor->tensor.data[(row * resultedTensor->columns) + column] += T1->tensor.data[(row * T1->columns) + iterator] * T2->tensor.data[(iterator * T2->columns) + column];
            }
        }
    }

    return resultedTensor;
}

/*
 * TODO: For now there is no need for multiplication
 * of tensor of rank 3, since we will work with mostly
 * rank 2 tensor. Furthermore first clarify the rules
 * of the dot product for higher rank tensors.
*/
pTensor dotProduct(pTensor T1, pTensor T2)
{
    pTensor resultedTensor = NULL;
    return resultedTensor;
}

/* HELPING FUNCTIONS */
void printTensor(pUniTensor T)
{
    printf("%s = [ ", GetVarName(T));
    for(uint32 iterator = ZERO; iterator < T->size; ++iterator)
    {
        printf("%*.3lf ", 5, T->data[iterator]);
    }
    printf("]\n");
}

void printTensor(pBiTensor T)
{
    printf("%s = [\n", GetVarName(T));
    for(uint32 row = ZERO; row < T->rows; ++row)
    {
        for(uint32 column = ZERO; column < T->columns; ++column)
        {
            printf("%*.3lf ", 11, T->tensor.data[(row * T->columns) + column]);
        }
        printf("\n");
    }
    printf("    ]\n");
}

void printTensor(pTensor T)
{
    printf("%s= [\n", GetVarName(T));
    for(uint32 slice = ZERO; slice < T->slices; ++slice)
    {
        printf("    [\n");
        for(uint32 row = ZERO; row < T->rows; ++row)
        {
            for(uint32 column = ZERO; column < T->columns; ++column)
            {
                printf("%*.3lf ", 12, T->data[T->offsets[slice] + ((row * T->columns) + column)]);
            }
            printf("\n");
        }
        printf("    ],\n");
    }
    printf("\n]\n");
}

/* TENSOR CLEANUP */
void freeTensor(pUniTensor T)
{
    T->size = ZERO;

    free(T->data);
    T->data = NULL;
}

void freeTensor(pBiTensor T)
{
    T->rows     = ZERO;
    T->columns  = ZERO;
    freeTensor(&T->tensor);
}

void freeTensor(pTensor T)
{
    T->slices   = ZERO;
    T->rows     = ZERO;
    T->columns  = ZERO;
    T->size     = ZERO;

    free(T->offsets);
    T->offsets = NULL;

    free(T->data);
    T->data = NULL;
}

/* TEST FUNCTIONS */
void testUniTensor()
{
    printf("--- CREATE UNIDIMENSIONAL TENSOR ---\n\n");
    uniTensor uT = {3, NULL};
    initializeTensor(&uT);

    assert(uT.size != ZERO);
    assert(uT.data != NULL);

    zeroTensor(&uT);
    printTensor(&uT);

    onesTensor(&uT);
    printTensor(&uT);

    randomizeTensor(&uT);
    printTensor(&uT);

    uniTensor t2;
    t2 = uT;

    printTensor(&t2);

    //zeroTensor(&uT);
    //printTensor(&uT);
    printTensor(&t2);

    t2 += uT;
    printTensor(&t2);

    t2 -= uT;
    printTensor(&t2);

    t2 -= ONE;
    printTensor(&t2);

    t2 += ONE;
    printTensor(&t2);

    t2 *= ONE;
    printTensor(&t2);

    t2 /= 0.5;
    printTensor(&t2);

    t2 *= ZERO;
    printTensor(&t2);
    
    //t2 /= ZERO;
    //printTensor(&t2);
    
    freeTensor(&uT);
    freeTensor(&t2);
    printf("\n--- UNIDIMENSIONAL TENSOR CREATED ---\n\n\n");
}

void testBiTensor()
{
   printf("--- CREATE BIDIMENSIONAL TENSOR ---\n\n");
   biTensor bT = {5, 3};
   initializeTensor(&bT);

   assert(bT.rows != ZERO);
   assert(bT.columns != ZERO);
   assert(bT.tensor.size != ZERO);
   assert(bT.tensor.data != NULL);
    
   zeroTensor(&bT);
   printTensor(&bT);

   onesTensor(&bT);
   printTensor(&bT);

   randomizeTensor(&bT);
   printTensor(&bT);

   biTensor bt2;
   bt2 = bT;
   printTensor(&bt2);

   bt2 += bT;
   printTensor(&bt2);

   bt2 -= bT;
   printTensor(&bt2);

   bt2 += ONE;
   printTensor(&bt2);

   bt2 -= ONE;
   printTensor(&bt2);

   bt2 *= ONE;
   printTensor(&bt2);

   bt2 /= 0.5;
   printTensor(&bt2);

   bt2 *= ZERO;
   printTensor(&bt2);

   bt2 /= ZERO;
   printTensor(&bt2);
    
   freeTensor(&bT);
   freeTensor(&bt2);
   printf("\n--- BIDIMENSIONAL TENSOR CREATED ---\n\n\n");
}

void testMultiTensor()
{
   printf("--- CREATE MULTIDIMENSIONAL TENSOR ---\n\n");
   tensor T = {3, 5, 3, 0, NULL, NULL};
   initializeTensor(&T);

   assert(T.slices != ZERO);
   assert(T.rows != ZERO);
   assert(T.columns != ZERO);
   assert((T.offsets != NULL) && (T.data != NULL));
    
   onesTensor(&T);
   printTensor(&T);

    zeroTensor(&T);
   printTensor(&T);

   randomizeTensor(&T);
   printTensor(&T);
    
   tensor t2;
   t2 = T;
   printTensor(&t2);

   t2 += T;
   printTensor(&t2);

    t2 -= T;
    printTensor(&t2);

   t2 += ONE;
   printTensor(&t2);

   t2 -= ONE;
    printTensor(&t2);

    t2 *= ONE;
    printTensor(&t2);

    t2 /= 0.5;
    printTensor(&t2);

    t2 *= ZERO;
    printTensor(&t2);

    //t2 /= ZERO;
    //printTensor(&t2);

    freeTensor(&T);
    freeTensor(&t2);
   printf("\n--- MULTIDIMENSIONAL TENSOR CREATED ---\n");
}

/*
int main()
{
    //testUniTensor();
    //testBiTensor();
    //testMultiTensor();
        
    time_t startTime,
           endTime;

    biTensor T1, T2;
    T1.rows = 1000;
    T1.columns = 1000;
    initializeTensor(&T1);
    randomizeTensor(&T1);
    //printTensor(&T1);

    T2.rows = 1000;
    T2.columns = 1000;
    initializeTensor(&T2);
    randomizeTensor(&T2);
    //printTensor(&T2);
    time(&startTime);
    pBiTensor result = dotProduct(&T1, &T2); 
    time(&endTime);

    double32 totalTime = (double32) (endTime - startTime);
    printf("Dot product execution time: %.3lf seconds\n", totalTime);
    //printTensor(result);
    return SUCCESS;
}
*/
