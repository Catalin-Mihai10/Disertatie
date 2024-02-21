#include "../interface/network.hpp"
#include "../utils/logger/NNLogger.hpp"
#include <math.h>

/* ALLOCATION/DEALOCATION OPERATIONS */
double32 max(double32 left, double32 right)
{
    return (left > right)? left:right;
}

double32 sigmoid(double32 dataPoint)
{
    return ONE / (ONE + expf((-ONE) * dataPoint));
}

double32 relu(double32 data)
{
    return max(ZERO, data);
}

double32 activation(pUniTensor inputs, pUniTensor weights)
{
    double32 result = ZERO;
    
    for(uint32 iterator = ZERO; iterator < inputs->size; ++iterator)
    {
        result += inputs->data[iterator] * weights->data[iterator];
    }

    return relu(result / inputs->size);
}

double32 cost(pNetwork network, pUniTensor data)
{
    double32 result = ZERO;

    for(uint32 iterator = ZERO; iterator < data->size; ++iterator)
    {
        for(uint32 neuron = 0; neuron < network->layers[(uint32)ZERO].size; ++neuron)
        {
            network->layers[(uint32)ZERO].neurons[neuron].inputs = data;
            result += network->layers[(uint32)ZERO].activationFunction(network->layers[(uint32)ZERO].neurons[neuron].inputs, 
                                                                       network->layers[(uint32)ZERO].neurons[neuron].weights);
        }
    }

    for(uint32 layer = ONE; layer < network->size; ++layer)
    {
        for(uint32 neuron = ZERO; neuron < network->layers[layer].size; ++neuron)
        {
            network->layers[layer].neurons[neuron].inputs = network->layers[layer - 1].neurons[neuron].inputs;
            result += network->layers[layer].activationFunction(network->layers[layer].neurons[neuron].inputs,
                                                                network->layers[layer].neurons[neuron].weights); 
        }
    }

    return result;
}

/* CREATION FUNCTIONS */
uint8 createNeuron(pNeuron neuron)
{
    neuron->inputs = (pUniTensor) malloc((sizeof *neuron->inputs));

    if(neuron->inputs == NULL)
    {
        NNERROR("ERROR: Could not create neuron!\n");
        return UNINITIALIZED_POINTER;
    }

    neuron->weights = (pUniTensor) malloc((sizeof *neuron->weights));

    if(neuron->weights == NULL)
    {
        NNERROR("ERROR: Could not create neuron weights!\n");
        return UNINITIALIZED_POINTER;
    }

    neuron->inputs->size    = neuron->inputSize;
    neuron->weights->size   = neuron->inputSize;

    uint8 initializeInputs  = initializeTensor(neuron->inputs),
          initializeWeights = initializeTensor(neuron->weights);

    return initializeInputs + initializeWeights;
}

uint8 createLayer(pLayer layer, uint32 inputSize)
{
    layer->neurons = (pNeuron) malloc((sizeof *layer->neurons) * layer->size);
    
    if(layer->neurons == NULL)
    {
        NNERROR("ERROR: Could not create layer!\n");
        return UNINITIALIZED_POINTER;
    }

    for(uint32 iterator = ZERO; iterator < layer->size; ++iterator)
    {
        layer->neurons[iterator].inputSize = inputSize;
        if(createNeuron(&layer->neurons[iterator]) != SUCCESS)
        {
            NNERROR("ERROR: Could not create neurons layer!\n");
            return GENERIC_ERROR_CODE;
        }
    }

    return SUCCESS;
}

void createNetwork(pNetwork network, uint32 size, uint32 layerSize, uint32 inputSize)
{
    network->size = size;
    network->layers = (pLayer) malloc((sizeof *network->layers) * size);
    
    if(network->layers == NULL)
    {
        NNERROR("ERROR: Could not create network!\n");
        freeNetwork(network);
    }

    for(uint32 iterator = 0; iterator < network->size; ++iterator)
    {
        network->layers[iterator].size = layerSize;
        if(createLayer(&network->layers[iterator], inputSize) != SUCCESS)
        {
            NNERROR("ERROR: Could not create network layers!\n");
            freeNetwork(network);
            return;
        }
    }

}

void freeNeuron(pNeuron neuron)
{
    freeTensor(neuron->inputs);
    neuron->inputs = NULL;
    freeTensor(neuron->weights);
    neuron->weights = NULL;

    neuron->inputSize = ZERO;
}

void freeLayer(pLayer layer)
{
    for(uint32 iterator = 0; iterator < layer->size; ++iterator)
    {
        freeNeuron(&layer->neurons[iterator]);
    }
    layer->neurons = NULL;

    layer->activationFunction = NULL;
    layer->size = ZERO;
}

void freeNetwork(pNetwork network)
{
    for(uint32 iterator = 0; iterator < network->size; ++iterator)
    {
        freeLayer(&network->layers[iterator]);
    }
    network->layers = NULL;
    
    network->size = ZERO;
}

/* INITIALIZATION FUNCTIONS */
void initializeNeuron(pNeuron neuron)   
{
    zeroTensor(neuron->inputs);
    zeroTensor(neuron->weights);
}

void initializeLayer(pLayer layer)
{
    for(uint32 iterator = 0; iterator < layer->size; ++iterator)
    {
        initializeNeuron(&layer->neurons[iterator]);
    }

    layer->activationFunction   = activation;
}

void initializeNetwork(pNetwork network)
{
    for(uint32 iterator = 0; iterator < network->size; ++iterator)
    {
        initializeLayer(&network->layers[iterator]);
        NNINFO("Layer [%d] initialized!\n", iterator);
    }
    NNINFO("Neural Network initialized!\n");
}

/* HELPFUL FUNCTIONS */
void printNeuronParameters(pNeuron neuron)
{
    printTensor(neuron->weights);
}

void printLayerParameters(pLayer layer)
{
    printf("[\n\n");
    for(uint32 iterator = 0; iterator < layer->size; ++iterator)
    {
        printNeuronParameters(&layer->neurons[iterator]);
    }
    printf("\n]\n");
}

void printNetworkParameters(pNetwork network)
{
    printf("Printing Network parameters\n");
    for(uint32 iterator = 0; iterator < network->size; ++iterator)
    {
        printf("Layer[%d]", iterator);
        printLayerParameters(&network->layers[iterator]);
    }
    printf("\n");
}

/* TEST FUNCTIONS */
void testNeuron()
{
    NNDEBUG("--- BUILD NEURON ---\n");
    neuron n;
    n.inputSize = 10;
    createNeuron(&n);
    assert(n.inputSize  != ZERO);
    assert(n.inputs     != NULL);
    assert(n.weights    != NULL);
    NNDEBUG("--- NEURON SUCCESSFULY BUILT ---\n\n");

    NNDEBUG("--- INITIALIZE NEURON ---\n");
    initializeNeuron(&n);
    assert(n.inputSize  != ZERO);
    assert(n.inputs     != NULL);
    assert(n.weights    != NULL);
    NNDEBUG("--- NEURON SUCCESSFULY INITIALIZED ---\n\n");   
        
    printNeuronParameters(&n);

    NNDEBUG("--- FREE NEURON ---\n");
    freeNeuron(&n);
    assert(n.inputSize  == ZERO);
    assert(n.inputs     == NULL);
    assert(n.weights    == NULL);
    NNDEBUG("--- NEURON SUCCESSFULY FREED ---\n\n");
}

void testLayer()
{
    NNDEBUG("--- BUILD LAYER WITH 5 NEURONS ---\n");
    layer l;
    l.size = 5;
    uint32 inputSize = 10;
    createLayer(&l, inputSize);
    assert(l.size != ZERO);
    assert(l.activationFunction == NULL);
    assert(l.activationFunction == NULL);
    assert(l.neurons != NULL);
    NNDEBUG("--- LAYER SUCCESSFULY BUILT ---\n\n");

    NNDEBUG("---  INITIALIZE LAYER ---\n");
    initializeLayer(&l);
    //assert(l.activationFunction != NULL);
    //assert(l.lossFunction != NULL);
    assert(l.neurons != NULL);
    NNDEBUG("--- LAYER INITIALIZED ---\n\n");
    
    printLayerParameters(&l);

    NNDEBUG("--- FREE LAYER ---\n");
    freeLayer(&l);
    assert(l.size == ZERO);
    assert(l.neurons == NULL);
    assert(l.activationFunction == NULL);
    NNDEBUG("--- LAYER FREED ---\n\n");
}

void testNetwork()
{
    NNDEBUG("--- CREATE NEURAL NETWORK ---\n");
    network n;
    createNetwork(&n, 3, 5, 10);
    assert(n.size != ZERO);
    assert(n.layers != NULL);
    NNDEBUG("--- NEURAL NETWORK CREATED ---\n\n");

    NNDEBUG("--- INITIALIZE NEURAL NETWORK ---\n");
    initializeNetwork(&n);
    assert(n.layers != NULL);
    assert(n.size != ZERO);
    //assert(n.layers->activationFunction != NULL);
    //assert(n.layers->costFunction != NULL);
    //assert(n.layers->neurons != NULL);
    //assert(n.layers->weights != NULL);
    NNDEBUG("--- NETWORK INITIALIZED ---\n\n");

    printNetworkParameters(&n);

    NNDEBUG("--- FREE NEURAL NETWORK ---\n");
    freeNetwork(&n);
    assert(n.size == ZERO);
    assert(n.layers == NULL);
    NNDEBUG("--- NEURAL NETWORK FREED ---\n\n");
}

int main()
{
    testNeuron();
    testLayer();
    testNetwork();
    return 0;
}
