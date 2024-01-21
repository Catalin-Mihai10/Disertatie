#include "../interface/network.hpp"
#include <math.h>

/* ALLOCATION/DEALOCATION OPERATIONS */
double32 max(double32 left, double32 right)
{
    return (left > right)? left:right;
}

double32 sigmoid(double dataPoint)
{
    return ONE / (ONE + expf((-ONE) * dataPoint));
}

double32 cost(pUniTensor neurons, pUniTensor weights)
{
    double32 result = ZERO;

    for(uint32 iterator = 0; iterator < neurons->size; ++iterator)
    {
        result += neurons->data[iterator] * weights->data[iterator];
    }

    return result;
}

double32 activation(pUniTensor neurons, pUniTensor weights)
{
    return max(ZERO, cost(neurons, weights));
}

uint8 createNeuron(pNeuron neuron, uint32 inputSize)
{
    neuron->inputVolume = inputSize;
    return SUCCESS;
}

uint8 createLayer(pLayer layer, uint32 layerSize, uint32 inputSize)
{
    layer->neurons = (pNeuron) malloc((sizeof *layer->neurons) * layerSize);
    
    if(layer->neurons == NULL) 
    {
        printf("ERROR: Could not create layer!\n");
        return UNINITIALIZED_POINTER;
    }   
    
    for(uint32 iterator = 0; iterator < layerSize; ++iterator)
    {
        if(createNeuron(&layer->neurons[iterator], inputSize))
        {
            printf("ERROR: Could not create layer!\n");
            return UNINITIALIZED_POINTER;
        }
    }

    layer->size = layerSize;
   
    return SUCCESS;
}

void createNetwork(pNetwork network, uint32 size, uint32 layersSize, uint32 inputSize)
{
    network->layers = (pLayer) malloc((sizeof *network->layers) * size);

    if(network->layers == NULL)
    {
        printf("ERROR: Could not create neural network!\n");
        return;
    }

    network->size = size;
    
    for(uint32 iterator = ZERO; iterator < network->size; ++iterator)
    {
        if(createLayer(&network->layers[iterator], layersSize, inputSize) != SUCCESS)
        {
            printf("ERROR: Could not create network layers!\n");
            freeNetwork(network);
            return;
        }
    }
}

void freeNeuron(pNeuron neuron)
{
    free(neuron->inputs);
    neuron->inputs = NULL;
    free(neuron->weights);
    neuron->weights = NULL;

    neuron->inputVolume = ZERO;
}

void freeLayer(pLayer layer)
{
    for(uint32 iterator = ZERO; iterator < layer->size; ++iterator)
    {
        freeNeuron(&layer->neurons[iterator]);
    }
    layer->neurons = NULL;

    layer->size                 = ZERO;
    layer->costFunction         = NULL;
    layer->activationFunction   = NULL;
}

void freeNetwork(pNetwork network)
{
    for(uint32 iterator = ZERO; iterator < network->size; ++iterator)
    {
        freeLayer(&network->layers[iterator]);
    }

    free(network->layers);
    network->layers = NULL;

    network->size = ZERO;
}


/* INITIALIZATION FUNCTIONS */
uint8 initializeLayer(pLayer layer)
{
    layer->activationFunction = activation;
    layer->costFunction = cost;
 
    uint8 neuronsResult = initializeTensor(layer->neurons),
          weightsResult = initializeTensor(layer->weights);

    return neuronsResult + weightsResult;
}

void initializeNetwork(pNetwork network)
{
    for(uint32 iterator = ZERO; iterator < network->size; ++iterator)
    {
        if( initializeLayer(&network->layers[iterator]) != SUCCESS )
        {
            printf("ERROR: Could not initialize network layers!\n");
            freeNetwork(network);
            return;
        }
    }
}


/* TEST FUNCTIONS */
void testLayer()
{
    printf("--- BUILD LAYER WITH 5 NEURONS ---\n");
    layer l;
    createLayer(&l, 5);
    assert(l.size != ZERO);
    assert(l.activationFunction == NULL);
    assert(l.activationFunction == NULL);
    assert(l.neurons != NULL);
    assert(l.weights != NULL);
    printf("--- LAYER SUCCESSFULY BUILT ---\n\n");

    printf("---  INITIALIZE LAYER ---\n");
    initializeLayer(&l);
    assert(l.activationFunction != NULL);
    assert(l.costFunction != NULL);
    assert(l.neurons != NULL);
    assert(l.weights != NULL);
    printf("--- LAYER INITIALIZED ---\n\n");
    
    printf("--- FREE LAYER ---\n");
    freeLayer(&l);
    assert(l.size == ZERO);
    assert(l.weights == NULL);
    assert(l.neurons == NULL);
    assert(l.costFunction == NULL);
    assert(l.activationFunction == NULL);
    printf("--- LAYER FREED ---\n\n");
}

void testNetwork()
{
    printf("--- CREATE NEURAL NETWORK ---\n");
    network n;
    createNetwork(&n, 3, 3);
    printf("returned!\n");
    assert(n.size != ZERO);
    assert(n.layers != NULL);
    printf("--- NEURAL NETWORK CREATED ---\n\n");

    printf("--- INITIALIZE NEURAL NETWORK ---\n");
    initializeNetwork(&n);
    assert(n.layers->size != ZERO);
    assert(n.layers->activationFunction != NULL);
    assert(n.layers->costFunction != NULL);
    assert(n.layers->neurons != NULL);
    assert(n.layers->weights != NULL);
    printf("--- NETWORK INITIALIZED ---\n\n");

    double32 learningRate = 0.001;

    printf("--- FREE NEURAL NETWORK ---\n");
    freeNetwork(&n);
    assert(n.size == ZERO);
    assert(n.layers == NULL);
    printf("--- NEURAL NETWORK FREED ---\n\n");
}

int main()
{
    testLayer();
    testNetwork();
    return 0;
}
