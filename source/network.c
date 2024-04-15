#include "../interface/network.h"
#include "../utils/logger/NNLogger.h"
#include "../utils/parser/parser.h"

#include "math.h"
#include <unistd.h>

/* ALLOCATION/DEALOCATION OPERATIONS */
double32 sigmoid(double32 dataPoint)
{
    return ONE / (ONE + expf((-ONE) * dataPoint));
}

double32 relu(double32 data)
{
    return (data > 0.0)? data:ZERO;
}

double32 leakyRelu(double32 data)
{
    double32 param = 0.01;
    return (data > 0.0)? data:param * data;
}

double32 leakyReluDerivative(double32 data)
{
    double param = 0.01;
    return (data > 0.0)? ONE:param;
}

double32 reluDerivative(double32 activation)
{
    return (activation > 0.0)? ONE:ZERO;
}

void softmax(pTensor activations) 
{
    double32 maximum = -9999.9999,
             sum = ZERO,
             scaling = ZERO;

    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        if(maximum < activations->data[iterator]) maximum = activations->data[iterator];
    }
    
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        sum += expf(activations->data[iterator] - maximum);
    }

    scaling = maximum + log(sum);
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        activations->data[iterator] = expf(activations->data[iterator] - scaling);
        assert(!isnan(activations->data[iterator]));   
    }
}

double32 neuronCost(pNeuron neuron)
{
    double32 result = dotProduct(neuron->inputs, neuron->weights);
    return result + neuron->bias;
}

double32 activationDerivative(double32 value, activations activation)
{
    double32 derivative = ZERO;
    switch(activation)
    {
        case RELU:
        {
            derivative = reluDerivative(value);
        } break;
        case LRELU:
        case PRELU:
        {
            derivative = leakyReluDerivative(value);
        } break;
        default: break;
    }

    return derivative;
}

void activation(pTensor values, activations activation)
{
    switch(activation)
    {
        case RELU:
        {
            for(uint32 iterator = (uint32)ZERO; iterator < values->size; ++iterator)
            {
                values->data[iterator] = relu(values->data[iterator]);
                assert(!isnan(values->data[iterator]));
            }
        } break;
        case LRELU:
        case PRELU:
        {
            for(uint32 iterator = (uint32)ZERO; iterator < values->size; ++iterator)
            {
                values->data[iterator] = leakyRelu(values->data[iterator]);
                assert(!isnan(values->data[iterator]));
            }
        } break;
        case SOFTMAX:
        {
            softmax(values);
        } break;
        default: break;
    }
}

void clipGradient(pTensor gradient)
{
    double norm = ZERO;
    for(uint32 iterator = (uint32)ZERO; iterator < gradient->size; ++iterator)
    {
        norm += gradient->data[iterator] * gradient->data[iterator];
    }
    norm = sqrt(norm);

    if(norm > ONE)
    {
        double32 scale = ONE / norm;
        for(uint32 iterator = (uint32)ZERO; iterator < gradient->size; ++iterator)
        {
            gradient->data[iterator] *= scale;
        }
    }
}

double32 computeLoss(pTensor activations, pTensor truthValues)
{
    double loss = ZERO;
    
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        loss += truthValues->data[iterator] * log(activations->data[iterator]); 
    }

    return -loss;
}

void feedforward(pLayer layer, pTensor data, activations layerActivation)
{
    for(uint32 neuron = (uint32)ZERO; neuron < layer->size; ++neuron)
    {
        copyTensorData(layer->neurons[neuron].inputs, data);
        layer->activations->data[neuron] = neuronCost(&layer->neurons[neuron]);
        
        assert(!isnan(layer->activations->data[neuron]));

        NNDEBUG("activation value: %lf", layer->activations->data[neuron]);
    }
    activation(layer->activations, layerActivation);
}

void backpropagation(pNetwork network, pTensor actualValue)
{
    double32 learningRate = 0.00015;
    NNDEBUG("Start backpropagation process");
    pTensor gradient = substractTensors(network->layers[network->size - 1].activations, actualValue); //equates activation derivative
    //printf("gradient\n");
    //printTensor(gradient);
    clipGradient(gradient);
    copyTensorData(network->layers[network->size - 1].gradients, gradient);
    for(uint32 neuron = ZERO; neuron < network->layers[network->size - 1].size; ++neuron)
    {
        network->layers[network->size - 1].neurons[neuron].bias -= learningRate * gradient->data[neuron];
        for(uint32 iterator = (uint32)ZERO; iterator < network->layers[network->size - 1].neurons[neuron].weights->size; ++iterator)
        {
                network->layers[network->size - 1].neurons[neuron].weights->data[iterator] -= learningRate * gradient->data[neuron] * network->layers[network->size - 1].neurons[neuron].inputs->data[iterator];
        }
        //printf("gradient:%lf\n", gradient->data[neuron]);
        //printTensor(network->layers[network->size - 1].neurons[neuron].weights);
    }

    for(sint32 layer = network->size - 2; layer >= 0; --layer)
    {
        /*NOTE: First we compute the sum of the gradient for the
         *      weight which affects the output of the next layer neurons.
         * */ 
        for(uint32 neuron = ZERO; neuron < network->layers[layer].size; ++neuron)
        {
            double32 gradientSum = ZERO;
            for(uint32 iterator = (uint32)ZERO; iterator < network->layers[layer + 1].size; ++iterator)
            {
                gradientSum += network->layers[layer + 1].neurons[iterator].weights->data[neuron] * network->layers[layer + 1].gradients->data[iterator];
                //printf("weight:%lf * gradient:%lf\n", network->layers[network->size - 1].neurons[iterator].weights->data[neuron], gradient->data[iterator]);
            }
            //printf("gradientSum:%lf\nlearning rate:%lf\nactivation derivative:%lf\n", gradientSum, learningRate, activationDerivative(network->layers[layer].activations->data[neuron], LRELU));
            //Save the gradient sums into another vector in order to no deal with a pain in the ass refactoring.
            network->layers[layer].gradients->data[neuron] = gradientSum;
        }
        
        /*NOTE: After that we begin updadting the bias and the
         *      weights on the hidden layers.
         * */
        clipGradient(network->layers[layer].gradients);
        for(uint32 neuron = ZERO; neuron < network->layers[layer].size; ++neuron)
        {
            network->layers[layer].neurons[neuron].bias -= learningRate * (network->layers[layer].gradients->data[neuron] * activationDerivative(network->layers[layer].activations->data[neuron], LRELU));
            for(uint32 iterator = (uint32)ZERO; iterator < network->layers[layer].neurons[neuron].weights->size; ++iterator)
            {
                double32 grad = network->layers[layer].gradients->data[neuron] * activationDerivative(network->layers[layer].activations->data[neuron], LRELU);
                //printf("relu layer gradient: %lf\n", grad);
                network->layers[layer].neurons[neuron].weights->data[iterator] -= learningRate * (grad * network->layers[layer].neurons[neuron].inputs->data[iterator]);
                if(isnan(network->layers[layer].activations->data[neuron]))
                {
                    printf("inputs:\n");
                    printTensor(network->layers[layer].neurons[neuron].inputs);
                    printf("weights:\n");
                    printTensor(network->layers[layer].neurons[neuron].weights);
                    printf("bias:%lf\n", network->layers[layer].neurons[neuron].bias);
                }
                //printf("Updated weight: %lf\n", network->layers[layer].neurons[neuron].weights->data[iterator]);
            } 
        }
    }
}

double32 train(pNetwork network, pTensor data, pTensor actualValue)
{
    double32 loss = ZERO;

    NNDEBUG("Begin network training");
    
    activations activation = LRELU;
    for(uint32 layer = (uint32)ZERO; layer < network->size; ++layer)
    {
        if(layer == network->size - 1) activation = SOFTMAX;
        if(layer == (uint32)ZERO) feedforward(&network->layers[layer], data, activation);
        else feedforward(&network->layers[layer], network->layers[layer - 1].activations, activation);
    }
    //printTensor(network->layers[network->size - 1].activations);

    backpropagation(network, actualValue);

    loss = computeLoss(network->layers[network->size - 1].activations, actualValue);
    loss /= network->layers[(uint32)ZERO].size;
    
    NNDEBUG("Ended network training process");
    return loss; 
}

/* CREATION FUNCTIONS */
uint8 createNeuron(pNeuron neuron)
{
    neuron->inputs = (pTensor) malloc((sizeof *neuron->inputs));

    if(neuron->inputs == NULL)
    {
        NNERROR("ERROR: Could not create neuron!\n");
        return UNINITIALIZED_POINTER;
    }

    neuron->weights = (pTensor) malloc((sizeof *neuron->weights));

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
    layer->activations = (pTensor) malloc((sizeof *layer->activations));
    layer->gradients = (pTensor) malloc((sizeof *layer->gradients));

    if((layer->neurons == NULL) || (layer->activations == NULL) || (layer->gradients == NULL))
    {
        NNERROR("ERROR: Could not create layer!\n");
        return UNINITIALIZED_POINTER;
    }

    layer->activations->size = layer->size;
    layer->gradients->size = layer->size;
    initializeTensor(layer->activations);
    initializeTensor(layer->gradients);

    for(uint32 iterator = (uint32)ZERO; iterator < layer->size; ++iterator)
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

void createNetwork(pNetwork network, uint32 size, pUInt32 layerSize, pUInt32 inputSizePerLayer)
{
    network->size = size;
    network->layers = (pLayer) malloc((sizeof *network->layers) * size);
    
    if(network->layers == NULL)
    {
        NNERROR("ERROR: Could not create network!\n");
        freeNetwork(network);
    }

    for(uint32 iterator = (uint32)ZERO; iterator < network->size; ++iterator)
    {

        network->layers[iterator].size = layerSize[iterator];
        uint32 neuronsInputSize = inputSizePerLayer[iterator]; 
    
        if(createLayer(&network->layers[iterator], neuronsInputSize) != SUCCESS)
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
}

void freeLayer(pLayer layer)
{
    for(uint32 iterator = 0; iterator < layer->size; ++iterator)
    {
        freeNeuron(&layer->neurons[iterator]);
    }
    free(layer->neurons);
    layer->neurons = NULL;
    
    freeTensor(layer->activations);
    layer->activations = NULL;

    freeTensor(layer->gradients);
    layer->gradients = NULL;

    //layer->activationFunction = NULL;
}

void freeNetwork(pNetwork network)
{
    for(uint32 iterator = 0; iterator < network->size; ++iterator)
    {
        freeLayer(&network->layers[iterator]);
    }
    free(network->layers);
    network->layers = NULL;
    
    network->size = ZERO;
}

/* INITIALIZATION FUNCTIONS */
void initializeNeuron(pNeuron neuron)   
{
    zeroTensor(neuron->inputs);
    randomizeTensor(neuron->weights);
    neuron->bias = (double32) rand() / (double32) (RAND_MAX / 0.2);
}

void initializeLayer(pLayer layer)
{
    for(uint32 iterator = 0; iterator < layer->size; ++iterator)
    {
        zeroTensor(layer->activations);
        initializeNeuron(&layer->neurons[iterator]);
    }

    //layer->activationFunction   = activation;
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
void testNeuron(void)
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

void testLayer(void)
{
    NNDEBUG("--- BUILD LAYER WITH 5 NEURONS ---\n");
    layer l;
    l.size = 5;
    uint32 inputSize = 10;
    createLayer(&l, inputSize);
    assert(l.size != ZERO);
    //assert(l.activationFunction == NULL);
    //assert(l.activationFunction == NULL);
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
    //assert(l.activationFunction == NULL);
    NNDEBUG("--- LAYER FREED ---\n\n");
}

void testNetwork(void)
{
    NNDEBUG("--- CREATE NEURAL NETWORK ---\n");
    network n;
    uint32 layerSizes[] = {3, 5, 1},
           inputSize[] = {3, 5, 1};
    createNetwork(&n, 3, layerSizes, inputSize);
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

void processData(const pSChar8 source, pCicDataset dataset)
{
    tcpString data;
    uint32 read = fileReading(source, &data);
    if(read == READ_ERROR)
    {
        free(data.data);
        data.data = NULL;
        NNERROR("Could not read data file!\n");
        return;
    }

    parseCsvData(&data, dataset);
    free(data.data);
    data.data = NULL;
    
    normalizeCsvData(dataset, dataset->rows);
}

void validationFeedForward(pNetwork network, pTensor data)
{
    activations activation = LRELU;
    for(uint32 layer = (uint32)ZERO; layer < network->size; ++layer)
    {
        if(layer == network->size - 1) activation = SOFTMAX;
        if(layer == (uint32)ZERO) feedforward(&network->layers[layer], data, activation);
        else feedforward(&network->layers[layer], network->layers[layer - 1].activations, activation);
    }

     NNDEBUG("Initialize input layer with training data");
}

void test(pCicDataset dataset, pNetwork network)
{
    tensor data, actualValue;
    data.size = dataset->featuresColumns;
    actualValue.size = dataset->labelsColumns;
    uint32 TP = ZERO,
           TN = ZERO,
           FP = ZERO,
           FN = ZERO;

    for(uint32 row = 0; row < dataset->rows; ++row)
    {
        data.data = &dataset->features[(row * dataset->featuresColumns)];
        actualValue.data = &dataset->labels[(row * dataset->labelsColumns)];
        validationFeedForward(network, &data);
        pTensor predictionError = substractTensors(network->layers[network->size - 1].activations, &actualValue); //equates activation derivative
        squareTensor(predictionError);
        
        double32 loss = computeLoss(network->layers[network->size - 1].activations, &actualValue);
        loss /= network->layers[(uint32)ZERO].size;

        NNINFO("Validation loss:%lf", loss);
        
        printTensor(network->layers[network->size - 1].activations);
        printTensor(&actualValue);
        printTensor(predictionError);

        if(actualValue.data[0] == ONE)
        {
            if(fabs(predictionError->data[0]) < 0.3)
            {
                ++TP;
            }else
            {
                ++FN;       
            }
        }else if(actualValue.data[1] == ONE)
        {
            if(fabs(predictionError->data[1]) > 0.6)
            {
                ++FP;
            }else 
            {
                ++TN;
            }
            
        }
    }
    
    uint32 total = TP + TN + FP + FN;
    printf("Network Accuracy:%lf\n", (double32)((double32)(TP + TN) / total));
    printf("True Positive: %d\nTrue Negative: %d\nFalse Positive: %d\nFalse Negative: %d\n", TP, TN, FP, FN);
}

void copyData(pCicDataset mix, pCicDataset dataset)
{
    uint32 MAX_DATASET_LIMT = 700000;

    if(dataset->rows < MAX_DATASET_LIMT)
    {
        MAX_DATASET_LIMT = dataset->rows;
    }

    for(uint32 iterator = (uint32)ZERO; iterator < MAX_DATASET_LIMT; ++iterator)
    {
        for(uint32 column = (uint32)ZERO; column < dataset->featuresColumns; ++column)
        {
            mix->features[((mix->rows + iterator) * mix->featuresColumns) + column] = dataset->features[(iterator * dataset->featuresColumns) + column];
        }

        for(uint32 column = (uint32)ZERO; column < dataset->labelsColumns; ++column)
        {
            mix->labels[((mix->rows + iterator) * mix->labelsColumns) + column] = dataset->labels[(iterator * dataset->labelsColumns) + column];
        }
    }

    mix->rows += MAX_DATASET_LIMT;
}

void suffleData(pCicDataset mix)
{

    srand((unsigned) time(NULL));
    for(uint32 row = (uint32)ZERO; row < mix->rows; ++row)
    {
        uint32 first = rand() / (RAND_MAX / mix->rows),
               second = rand() / (RAND_MAX / mix->rows); 
        for(uint32 column = (uint32)ZERO; column < mix->featuresColumns; ++column)
        {
           double32 temp = mix->features[(first * mix->featuresColumns) + column];
           mix->features[(first * mix->featuresColumns) + column] = mix->features[(second * mix->featuresColumns) + column];
           mix->features[(second * mix->featuresColumns) + column] = temp;
        }

        for(uint32 column = (uint32)ZERO; column < mix->labelsColumns; ++column)
        {
           double32 temp = mix->labels[(first * mix->labelsColumns) + column];
           mix->labels[(first * mix->labelsColumns) + column] = mix->labels[(second * mix->labelsColumns) + column];
           mix->labels[(second * mix->labelsColumns) + column] = temp;
        }
    }
}

int main(void)
{
  //  testNeuron();
  //  testLayer();
  //  testNetwork();

    pSChar8 source[11] = {"/home/catalin/Datasets/CIC_2019/03-11/Portmap.csv",
                        "/home/catalin/Datasets/CIC_2019/03-11/LDAP.csv",
                        "/home/catalin/Datasets/CIC_2019/03-11/MSSQL.csv",
                        "/home/catalin/Datasets/CIC_2019/03-11/NetBIOS.csv",
                        "/home/catalin/Datasets/CIC_2019/03-11/Syn.csv",
                        "/home/catalin/Datasets/CIC_2019/03-11/UDP.csv",
                        "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_DNS.csv",
                        "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_LDAP.csv",
                        "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_MSSQL.csv",
                        "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_NetBIOS.csv",
                        "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_NTP.csv",
                        };
    cic_dataset dataset, mix;
    network n;
    uint32 layerSizes[3],
           inputSize[3];
    
    uint32 datasetMixSize = 9800000;
    mix.featuresColumns = 60;
    mix.labelsColumns = 2;
    mix.rows = (uint32)ZERO;
    mix.features = malloc((sizeof *mix.features) * datasetMixSize * mix.featuresColumns);
    mix.labels = malloc((sizeof *mix.labels) * datasetMixSize * mix.labelsColumns);
        
    for(uint32 sources = 0; sources < 11; ++sources)
    {
        NNINFO("process: %s\n", source[sources]);
        processData(source[sources], &dataset);
            
        for(uint32 line = 0; line < dataset.rows; ++line)
        {
            for(uint32 column = 0; column < 60; ++column)
            {
                assert(!isnan(dataset.features[(line * 60) + column]));
                assert(!isinf(dataset.features[(line * 60) + column]));
            }
        }

        if(sources == 0)
        {
            layerSizes[0] = dataset.featuresColumns; 
            layerSizes[1] = dataset.featuresColumns / 4;
            layerSizes[2] = dataset.labelsColumns;
            
            inputSize[0] = dataset.featuresColumns;
            inputSize[1] = dataset.featuresColumns;
            inputSize[2] = dataset.featuresColumns / 4;
            NNDEBUG("Construct neural network");
            createNetwork(&n, 3, layerSizes, inputSize);
            initializeNetwork(&n);
        }
        copyData(&mix, &dataset);

        free(dataset.features);
        dataset.features = NULL;
        free(dataset.labels);
        dataset.labels = NULL;
    }

    suffleData(&mix);
    for(uint32 line = 0; line < mix.rows; ++line)
    {
        for(uint32 column = 0; column < 60; ++column)
        {
            assert(!isnan(mix.features[(line * 60) + column]));
            assert(!isinf(mix.features[(line * 60) + column]));
        }
    }

    NNINFO("Start Network training");

    printf("intra in loop\n");
    uint32 epochPatience = 5,
           noImprovement = (uint32)ZERO,
           epoch = (uint32)ZERO,
           maxEpochs = 10;
    double32 validationLoss = ZERO,
             epsilon = 1e-5,
             bestValue = 9999.9999;

    while(epoch < maxEpochs && noImprovement < epochPatience)
    {
 
        tensor data, actualValue;
        data.size = mix.featuresColumns;
        actualValue.size = mix.labelsColumns;   
        
        for(uint32 line = (uint32)ZERO; line < mix.rows; ++line)
        {
            NNDEBUG("Iteration: %d", line);
            data.data = &mix.features[(line * mix.featuresColumns)];
            actualValue.data = &mix.labels[(line * mix.labelsColumns)];
            double32 returnCost = train(&n, &data, &actualValue);
            NNINFO("Network cost: %lf", returnCost);
        }
        
        cic_dataset temp;
        processData("/home/catalin/Datasets/CIC_2019/test.csv", &temp);
        data.size = temp.featuresColumns;
        actualValue.size = temp.labelsColumns;
        for(uint32 row = 0; row < temp.rows; ++row)
        {
            data.data = &temp.features[(row * temp.featuresColumns)];
            actualValue.data = &temp.labels[(row * temp.labelsColumns)];
            validationFeedForward(&n, &data);
            pTensor predictionError = substractTensors(n.layers[n.size - 1].activations, &actualValue); //equates activation derivative
            squareTensor(predictionError);
        
            double32 loss = computeLoss(n.layers[n.size - 1].activations, &actualValue);
            loss /= n.layers[(uint32)ZERO].size;
            validationLoss += loss;
        }
        free(temp.features);
        temp.features = NULL;
        free(temp.labels);
        temp.labels = NULL;

        if(validationLoss + epsilon < bestValue)
        {
            bestValue = validationLoss;
            noImprovement = (uint)ZERO;
        }else ++noImprovement;
    
        ++epoch;
    }

    NNINFO("Network training completed!");

    free(mix.features);
    mix.features = NULL;
    free(mix.labels);
    mix.labels = NULL;
    
    processData("/home/catalin/Datasets/CIC_2019/test.csv", &dataset);
    test(&dataset, &n);
        
    free(dataset.features);
    dataset.features = NULL;
    free(dataset.labels);
    dataset.labels = NULL;
    freeNetwork(&n);
    return 0;
}
