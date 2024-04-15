#ifndef NEURAL_NETWORK_HEADER
#define NEURAL_NETWORK_HEADER

#include "constants.h"
#include "tensor.h"

typedef struct neuron
{
    uint32  inputSize;
    double32 bias;
    pTensor inputs,
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
    //double32 (*activationFunction)  (pTensor inputs, pTensor weights);
    pTensor activations,
            gradients;
    pNeuron neurons;
}layer;

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

typedef enum activations 
{
    RELU = 0x00,
    LRELU,
    PRELU,
    SOFTMAX
} activations;

/* CREATE FUNCTIONS */
uint8 createNeuron(pNeuron neuron);
uint8 createLayer(pLayer layer, uint32 inputSize);
void  createNetwork(pNetwork network, uint32 size, pUInt32 layerSize, pUInt32 inputSizePerLayer);

void freeNeuron(pNeuron neuron);
void freeLayer(pLayer layer);
void freeNetwork(pNetwork network);

/* INITIALIZATION FUNCTIONS */
void initializeNeuron(pNeuron neuron);
void initializeLayer(pLayer layer);
void initializeNetwork(pNetwork network);

/*  */
void setLayerActivationFunction(uint8 type);

/* HELPFUL FUNCTIONS */
void printNetworkParameters(pNetwork network);
void printLayerParameters(pLayer layer);
void printNeuronParameters(pNeuron neuron);
#endif /* NEURAL_NETWORK_HEADER */
