#ifndef NEURAL_NETWORK_HEADER
#define NEURAL_NETWORK_HEADER

#include "types.hpp"
#include "tensor.hpp"

typedef struct neuron
{
    uint32 inputSize;

    pUniTensor  inputs,
                weights;
} neuron;

typedef neuron* pNeuron;

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
   
    pNeuron neurons;
   
    double32 (*activationFunction)  (pUniTensor inputs, pUniTensor weights);
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

typedef network* pNetwork;

/* CREATE FUNCTIONS */
uint8 createNeuron(pNeuron neuron);
uint8 createLayer(pLayer layer, uint32 inputSize);
void  createNetwork(pNetwork network, uint32 size, uint32 layerSize, uint32 inputSize);

void freeNeuron(pNeuron neuron);
void freeLayer(pLayer layer);
void freeNetwork(pNetwork network);

/* INITIALIZATION FUNCTIONS */
void initializeNeuron(pNeuron neuron);
void initializeLayer(pLayer layer);
void initializeNetwork(pNetwork network);

/* HELPFUL FUNCTIONS */
void printNetworkParameters(pNetwork network);
void printLayerParameters(pLayer layer);
void printNeuronParameters(pNeuron neuron);

#endif /* NEURAL_NETWORK_HEADER */
