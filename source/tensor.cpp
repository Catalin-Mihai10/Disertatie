#include "../interface/tensor.hpp"
#include <cstdlib>

/* DEFINE MACROS */
#define GetVarName(var) #var

/* INITIALIZATION OPERATIONS */

void initializeTensor(pUniTensor T)
{
    if(T->size == 0)
    {
        printf("ERROR: Invalid Tensor size: %u!\n", T->size);
        exit(1);
    }

    T->data = (pDouble32) malloc((sizeof T->data) * T->size);
    
    if(T->data == NULL)
    {
        printf("ERROR: Could not initialize tensor!\n");
        exit(1);
    }
}

void initializeTensor(pBiTensor T)
{
    if((T->rows == 0) || (T->columns == 0))
    {
        printf("ERROR: Invalid Tensor size!\n");
        exit(1);
    }

    T->tensor.size = T->rows * T->columns;
    initializeTensor(&T->tensor);
}

void initializeTensor(pTensor T)
{
    if((T->slices == 0) || (T->rows == 0) || (T->columns == 0))
    {
        printf("ERROR: Invalid Tensor size!\n");
        exit(1);
    }
    
    T->size = T->slices * T->rows * T->columns;
    T->offsets = (pUInt32) malloc((sizeof T->offsets) * T->slices);
    T->data = (pDouble32) malloc((sizeof T->data) * T->slices * T->rows * T->columns);

    if((T->offsets == NULL) || (T->data == NULL))
    {
        printf("ERROR: Could not initialize tensor!\n");
        exit(1);
    }
    
    T->offsets[0] = 0;
    for(uint32 slice = 1; slice < T->slices; ++slice)
    {
        T->offsets[slice] = T->offsets[slice - 1] + (T->rows * T->columns); 
    }
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
    for(uint32 iterator = 0; iterator < T->size; ++iterator)
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
    for(uint32 iterator = 0; iterator < T->size; ++iterator)
    {
        T->data[iterator] = ONE;
    }
}

/* TENSOR OPERATIONS */
void randomizeTensor(pUniTensor T)
{
    srand((unsigned) time(NULL));

    for(uint32 iterator = 0; iterator < T->size; ++iterator)
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
    for(uint32 iterator = 0; iterator < T->size; ++iterator)
    {
        T->data[iterator] = (double32) rand() / (double32) (RAND_MAX / 1.0);
    }
}


/* HELPING FUNCTIONS */
void printTensor(pUniTensor T)
{
    printf("%s = [ ", GetVarName(T));
    for(uint32 iterator = 0; iterator < T->size; ++iterator)
    {
        printf("%*.3lf ", 5, T->data[iterator]);
    }
    printf("]\n");
}

void printTensor(pBiTensor T)
{
    printf("%s = [\n", GetVarName(T));
    for(uint32 row = 0; row < T->rows; ++row)
    {
        for(uint32 column = 0; column < T->columns; ++column)
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
    for(uint32 slice = 0; slice < T->slices; ++slice)
    {
        printf("    [\n");
        for(uint32 row = 0; row < T->rows; ++row)
        {
            for(uint32 column = 0; column < T->columns; ++column)
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

    assert(uT.size != 0);
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

   assert(bT.rows != 0);
   assert(bT.columns != 0);
   assert(bT.tensor.size != 0);
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

   assert(T.slices != 0);
   assert(T.rows != 0);
   assert(T.columns != 0);
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

int main()
{
    //testUniTensor();
    //testBiTensor();
    testMultiTensor();
    return 0;
}
