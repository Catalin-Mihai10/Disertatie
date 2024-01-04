#ifndef NEURAL_NETWORK_HEADER
#define NEURAL_NETWORK_HEADER

#include "types.hpp"
#include "tensor.hpp"

/**
 * @brief structure that represents a layer of a
 * neural network. The network is a fully conected FFN.
 *
 * size                - an unsigned 32 bit integer that represents the 
 *                       number of neurons in the layer;
 *
 * neurons             - a pointer to a tensor structure that represents
 *                       the neurons of the layer.
 *
 * weights             - a pointer to a tensor structure that represents
 *                       the weights of the layer.
 *
 * activationFunction  - a pointer to a function that return a 32 bit
 *                       floating point number representing the activation
 *                       of the layer. It can take values between [0.0, 1.0]
 * 
 * lossFunction        - a pointer to a function that returns a 32 bit 
 *                       floating point number representing the accuracy
 *                       of the prediction at the respective layer. It can
 *                       take values between [0.0, 1.0]
 */
typedef struct layer
{
    uint32 size;
    
    pUniTensor  neurons,
                weights;
    
    double32 (*activationFunction)  (pUniTensor neurons, pUniTensor weights);
    double32 (*lossFunction)        (pUniTensor neurons, pUniTensor weights);
} layer;

typedef layer* pLayer;

/**
 * @brief structure that represents a neural network.
 *
 * size     - an unsigned 32 bit integer that represents the number of
 *            hidden layers in the neural network.
 *
 * layers   - a pointer to layer structure.
 */

typedef struct network
{
    uint32 size;
    pLayer layers;
} network;



#endif /* NEURAL_NETWORK_HEADER */
