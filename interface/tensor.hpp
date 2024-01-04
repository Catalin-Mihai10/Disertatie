#ifndef TENSOR_HEADER
#define TENSOR_HEADER

#include "basicLibraries.hpp"
#include "constants.hpp"
#include "types.hpp"

/**
 * @brief   Structure that represents a rank 1 Tensor.
 * 
 * size -   An unsigned 32 bit integer that represents
 *          the tensor dimension.
 * 
 * data -   A 32 bit floating point pointer that represents
 *          the tensor data.
 */
typedef struct uniTensor
{
    uint32      size;
    pDouble32   data;

    uniTensor operator=(const uniTensor& T)
    {
        this->size = T.size;
        this->data = (pDouble32) malloc((sizeof this->data) * this->size);
        memcpy(this->data, T.data, (sizeof this->data) * this->size);

        return *this;
    }
    
    void operator+=(const uniTensor& T)
    {
        if(this->size != T.size)
        {
            printf("ERROR: Tensors are not of same size!\n");
            exit(1);
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] += T.data[iterator];
        }
    }

    void operator+=(const uint32& scalar)
    {
        if(scalar == ZERO) return;

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] += scalar;
        }
    }

    void operator-=(const uniTensor& T)
    {
        if(this->size != T.size)
        {
            printf("ERROR: Tensors are not of same size!\n");
            exit(1);
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] -= T.data[iterator];
        }
    }

    void operator-=(const double32& scalar)
    {
        if(scalar == ZERO) return;

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] -= scalar;
        }
    }

    void operator*=(const double32& scalar)
    {

        if(scalar == ONE) return;
        
        if(scalar == ZERO)
        {
            memset(this->data, ZERO, (sizeof this->data) * this->size);
            return;
        }
        
        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] *= scalar;
        }
    }
    
    void operator/=(const double32& scalar)
    {

        if(scalar == ONE) return;
        
        if(scalar == ZERO)
        {
            printf("ERROR: Cannot divide by zero!");
            exit(1);
        }
        
        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] /= scalar;
        }
    }

} uniTensor;

/**
 * @brief   Structure that represents a rank 1 Tensor.
 * 
 * rows    -   An unsigned 32 bit integer that represents
 *             the tensor number of rows.
 * 
 * columns -   An unsigned 32 bit integer that represents
 *             the tensor number of columns.
 *
 * tensor  -   A rank 1 Tensor type structure in which we store
 *             our tensor data.
 */
typedef struct biTensor
{
    uint32      rows,
                columns;
    uniTensor   tensor;

    biTensor operator=(const biTensor& T)
    {
        this->rows =    T.rows;
        this->columns = T.columns;
        this->tensor =  T.tensor;

        return *this;
    }
    
    void operator+=(const biTensor& T)
    {
        if((this->rows != T.rows) || (this->columns != T.columns))
        {
            printf("ERROR: Tensor are not the same size!\n");
            exit(1);
        }

        this->tensor += T.tensor;
    }

    void operator+=(const double32& scalar)
    {
        if(scalar == ZERO) return;

        this->tensor += scalar;
    }

    void operator-=(const biTensor& T)
    {
        if((this->rows != T.rows) || (this->columns != T.columns))
        {
            printf("ERROR: Tensor are not the same size!\n");
            exit(1);
        }

        this->tensor -= T.tensor;
    }

    void operator-=(const double32& scalar)
    {
        if(scalar == ZERO) return;

        this->tensor -= scalar;
    }

    void operator*=(const double32& scalar)
    {
        this->tensor *= scalar;
    }

    void operator/=(const double32& scalar)
    {
        this->tensor /= scalar;
    }

} biTensor;

/**
 * @brief   Structure that represents a rank 1 Tensor.
 * 
 * slices  -  An unsigned 32 bit integer that represents
 *            the tensor number of rank 2 tensors.
 * 
 * rows    -  An unsigned 32 bit integer that represents
 *            each slice number of rows.
 *
 * columns -  An unsigned 32 bit integer that represents
 *            each slice number of columns.
 *
 * offsets -  A 32 bit unsigned integer pointer that stores
 *            the offsets of each slice in the tensor.
 *
 * data    -  A 32 bit floating point pointer that represents
 *            the tensor data.
 */
typedef struct tensor
{
    uint32  slices,
            rows,
            columns,
            size;
    pUInt32 offsets;
    pDouble32 data;

    tensor operator=(const tensor& T)
    {
        this->slices =  T.slices;
        this->rows =    T.rows;
        this->columns = T.columns;
        this->size =    T.size;

        this->offsets = (pUInt32) malloc((sizeof this->offsets) * this->slices);
        memcpy(this->offsets, T.offsets, (sizeof this->offsets) * this->slices);

        this->data = (pDouble32) malloc((sizeof this->data) * this->slices * this->rows * this->columns);
        memcpy(this->data, T.data, (sizeof this->data) * this->slices * this->rows * this->columns);
    
        return *this;
    }

    void operator+=(const tensor& T)
    {
        if((this->slices != T.slices) || (this->rows != T.rows) || (this->columns != T.columns))
        {
            printf("ERROR: Tensors are not of same size!\n");
            return;
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] += T.data[iterator];
        }
    }

    void operator+=(const double32& scalar)
    {
        if(scalar == ZERO) return;

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] += scalar;
        }
    }

    void operator-=(const tensor& T)
    {
        if((this->slices != T.slices) || (this->rows != T.rows) || (this->columns != T.columns))
        {
            printf("ERROR: Tensors are not of same size!\n");
            return;
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] -= T.data[iterator];
        }
    }

    void operator-=(const double32& scalar)
    {
        if(scalar == ZERO) return;

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] -= scalar;
        }
    }

    void operator*=(const double32& scalar)
    {
        if(scalar == ONE) return;

        if(scalar == ZERO)
        {
            memset(this->data, ZERO, (sizeof this->data) * this->size);
            return;
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] *= scalar;
        }
    }

    void operator/=(const double32& scalar)
    {
        if(scalar == ONE) return;

        if(scalar == ZERO)
        {
            printf("ERROR: Cannot divide by zero!");
            return;
        }

        for(uint32 iterator = 0; iterator < this->size; ++iterator)
        {
            this->data[iterator] /= scalar;
        }
    }

} tensor;

/* DEFINE POINTER TYPES */
typedef uniTensor*  pUniTensor;
typedef biTensor*   pBiTensor;
typedef tensor*     pTensor;

/* INITIALIZATION OPERATIONS */
void initializeTensor(pUniTensor T);
void initializeTensor(pBiTensor T);
void initializeTensor(pTensor T);

void zeroTensor(pUniTensor T);
void zeroTensor(pBiTensor T);
void zeroTensor(pTensor T);

void onesTensor(pUniTensor T);
void onesTensor(pBiTensor T);
void onesTensor(pTensor T);

void randomizeTensor(pUniTensor T);
void randomizeTensor(pBiTensor);
void randomizeTensor(pTensor T);

/* TENSOR OPERATIONS */

/* HELPING FUNCTIONS */
void printTensor(pUniTensor T);
void printTensor(pBiTensor T);
void printTensor(pTensor T);

/* TENSOR CLEANUP */
void freeTensor(pUniTensor T);
void freeTensor(pBiTensor T);
void freeTensor(pTensor T);

#endif /* TENSOR_HEADER */
