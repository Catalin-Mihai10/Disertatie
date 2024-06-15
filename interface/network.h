#ifndef NEURAL_NETWORK_HEADER
#define NEURAL_NETWORK_HEADER

#include "constants.h"
#include "tensor.h"
#include "activations.h"

/**
 * @brief structure that represents a neuron of a
 * layer in the neural network. 
 *
 * bias                - a 32 bit double that represents the bias of the
 *                       neuron;
 * 
 * inputs              - a pointer to a tensor structure that represents
 *                       the input values of the current neuron;
 *
 * weights             - a pointer to a tensor structure that represents
 *                       the of the current neuron.
 */
typedef struct neuron
{
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
 * activations         - a pointer to a tensor structure that represents
 *                       the values of the activation function of the neurons
 *                       on the current layer;
 * 
 * gradients           - a pointer to a tensor structure that represents
 *                       the gradient values of the current layer;
 *
 * neurons             - a pointer to a tensor structure that represents
 *                       the neurons of the layer.
 */
typedef struct layer
{
    uint32 size;
    pTensor activations,
            gradients;
    pNeuron neurons;
}layer;

typedef layer* pLayer;

/**
 * @brief structure that represents a neural network.
 *
 * size     - an unsigned 32 bit integer that represents the number of
 *            hidden layers in the neural network;
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
uint8 createNeuron(pNeuron neuron, uint32 inputSize);
uint8 createLayer(pLayer layer, uint32 inputSize);
void  createNetwork(pNetwork network, uint32 size, pUInt32 layerSize, pUInt32 inputSizePerLayer);

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

void feedforward(pLayer layer, pTensor data, activations layerActivation);
void backpropagation(pNetwork network, pTensor actualValue);

#endif /* NEURAL_NETWORK_HEADER */
