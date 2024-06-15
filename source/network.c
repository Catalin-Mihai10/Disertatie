#include "../interface/network.h"
#include "../interface/activations.h"
#include "../utils/logger/NNLogger.h"
#include "../utils/parser/parser.h"
#include "../utils/parser/dataProcessing.h"
#include "../utils/loader/loader.h"

#include <float.h>
#include <math.h>
#include <unistd.h>

double32 neuronCost(pNeuron neuron)
{
    double32 result = dotProduct(neuron->inputs, neuron->weights);
    return result + neuron->bias;
}

void clipGradient(pTensor gradient)
{
    double norm = ZERO,
           epsilon = 1e-3;
    for(uint32 iterator = (uint32)ZERO; iterator < gradient->size; ++iterator)
    {
        norm += gradient->data[iterator] * gradient->data[iterator];
    }
    norm = sqrt(norm);
    
    if(norm == ZERO)
    {
        norm += epsilon;
    }


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
        loss -= truthValues->data[iterator] * log(activations->data[iterator]); 
    }

    return loss;
}

double32 computeBinaryCrossEntropyLoss(pTensor activations, pTensor truthValues)
{
    double32 loss = ZERO,
             epsilon = 1e-3;
    
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        loss = -((truthValues->data[iterator] * log(activations->data[iterator] + epsilon)) + ((ONE - truthValues->data[iterator]) * log((ONE - activations->data[iterator]) + epsilon)));
    }
    
    return loss;
}

void feedforward(pLayer layer, pTensor data, activations layerActivation)
{
    for(uint32 neuron = (uint32)ZERO; neuron < layer->size; ++neuron)
    {
        copyTensorData(layer->neurons[neuron].inputs, data);
        layer->activations->data[neuron] = neuronCost(&layer->neurons[neuron]);
        
        NNDEBUG("activation value: %lf", layer->activations->data[neuron]);
    }
    activation(layer->activations, layerActivation);
}

void voidForZeros(pTensor activations)
{
    double32 epsilon = 1e-3;
    if((activations->data[0] == 0) && (activations->data[1] == 0))
    {
        activations->data[0] += epsilon;
        activations->data[1] += epsilon;
    }
}

void backpropagation(pNetwork network, pTensor actualValue)
{
    double32 learningRate = 0.00015;
    NNDEBUG("Start backpropagation process");
    voidForZeros(network->layers[network->size - 1].activations);

    /*NOTE: Just subtracting the values of the truth and predicted
     *      values equate to the derivative of the loss function
     *      with regard to the weights.
     * */
    pTensor gradient = substractTensors(network->layers[network->size - 1].activations, actualValue);

    voidForZeros(gradient);
    clipGradient(gradient);
    
    copyTensorData(network->layers[network->size - 1].gradients, gradient);
    for(uint32 neuron = ZERO; neuron < network->layers[network->size - 1].size; ++neuron)
    {
        network->layers[network->size - 1].neurons[neuron].bias -= learningRate * gradient->data[neuron];
        for(uint32 weight = (uint32)ZERO; weight < network->layers[network->size - 1].neurons[neuron].weights->size; ++weight)
        {
                network->layers[network->size - 1].neurons[neuron].weights->data[weight] -= learningRate * (gradient->data[neuron] * network->layers[network->size - 1].neurons[neuron].inputs->data[weight]);
        }
    }

    for(sint32 layer = network->size - 2; layer >= 0; --layer)
    {
        /*NOTE: First we compute the sum of the gradient for the
         *      weight which affects the output of the next layer neurons.
         * */ 
        for(uint32 neuron = ZERO; neuron < network->layers[layer].size; ++neuron)
        {
            double32 gradientSum = ZERO;
            for(uint32 upperNeuron = (uint32)ZERO; upperNeuron < network->layers[layer + 1].size; ++upperNeuron)
            {
                gradientSum += network->layers[layer + 1].neurons[upperNeuron].weights->data[neuron] * network->layers[layer + 1].gradients->data[upperNeuron];
            }
            gradientSum += 1e-3;
     
            /*NOTE: Save the gradient sums into another vector in order to no deal with a pain in the ass refactoring.*/
            network->layers[layer].gradients->data[neuron] = gradientSum;
        }
        
        /*NOTE: After that we begin updadting the bias and the
         *      weights on the hidden layers.
         * */
        clipGradient(network->layers[layer].gradients);
        for(uint32 neuron = ZERO; neuron < network->layers[layer].size; ++neuron)
        {
            network->layers[layer].neurons[neuron].bias -= learningRate * (network->layers[layer].gradients->data[neuron] * activationDerivative(network->layers[layer].activations->data[neuron], LRELU));
            for(uint32 weight = (uint32)ZERO; weight < network->layers[layer].neurons[neuron].weights->size; ++weight)
            {
                double32 grad = network->layers[layer].gradients->data[neuron] * activationDerivative(network->layers[layer].activations->data[neuron], LRELU);
                network->layers[layer].neurons[neuron].weights->data[weight] -= learningRate * (grad * network->layers[layer].neurons[neuron].inputs->data[weight]);
            } 
        }
    }
    
    free(gradient->data);
    gradient->data = NULL;
    free(gradient);
    gradient = NULL;
}

double32 train(pNetwork network, pTensor data, pTensor actualValue)
{
    double32 loss = ZERO;

    NNDEBUG("Begin network training");
    
    feedforward(&network->layers[(uint32)ZERO], data, LRELU);
    for(uint32 layer = (uint32)ONE; layer < network->size - 1; ++layer)
    {
        feedforward(&network->layers[layer], network->layers[layer - 1].activations, LRELU);
    }
    feedforward(&network->layers[network->size - 1], network->layers[network->size - 2].activations, SOFTMAX);

    backpropagation(network, actualValue);

    //loss = computeLoss(network->layers[network->size - 1].activations, actualValue);
    loss = computeBinaryCrossEntropyLoss(network->layers[network->size - 1].activations, actualValue);
    
    NNDEBUG("Ended network training process");
    return loss; 
}

/* CREATION FUNCTIONS */
uint8 createNeuron(pNeuron neuron, uint32 inputSize)
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
    
    neuron->inputs->size    = inputSize;
    neuron->weights->size   = inputSize;

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
        if(createNeuron(&layer->neurons[iterator], inputSize) != SUCCESS) 
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
    for(uint32 iterator = (uint32)ZERO; iterator < layer->size; ++iterator)
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
    for(uint32 iterator = (uint32)ZERO; iterator < network->size; ++iterator)
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
    for(uint32 iterator = (uint32)ZERO; iterator < layer->size; ++iterator)
    {
        zeroTensor(layer->activations);
        initializeNeuron(&layer->neurons[iterator]);
    }
}

void initializeNetwork(pNetwork network)
{
    for(uint32 iterator = (uint32)ZERO; iterator < network->size; ++iterator)
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
    for(uint32 iterator = (uint32)ZERO; iterator < layer->size; ++iterator)
    {
        printNeuronParameters(&layer->neurons[iterator]);
    }
    printf("\n]\n");
}

void printNetworkParameters(pNetwork network)
{
    printf("Printing Network parameters\n");
    for(uint32 iterator = (uint32)ZERO; iterator < network->size; ++iterator)
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
    createNeuron(&n, 10);
    assert(n.inputs     != NULL);
    assert(n.weights    != NULL);
    NNDEBUG("--- NEURON SUCCESSFULY BUILT ---\n\n");

    NNDEBUG("--- INITIALIZE NEURON ---\n");
    initializeNeuron(&n);
    assert(n.inputs     != NULL);
    assert(n.weights    != NULL);
    NNDEBUG("--- NEURON SUCCESSFULY INITIALIZED ---\n\n");   
        
    printNeuronParameters(&n);

    NNDEBUG("--- FREE NEURON ---\n");
    freeNeuron(&n);
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
    assert(l.neurons != NULL);
    NNDEBUG("--- LAYER SUCCESSFULY BUILT ---\n\n");

    NNDEBUG("---  INITIALIZE LAYER ---\n");
    initializeLayer(&l);
    assert(l.neurons != NULL);
    NNDEBUG("--- LAYER INITIALIZED ---\n\n");
    
    printLayerParameters(&l);

    NNDEBUG("--- FREE LAYER ---\n");
    freeLayer(&l);
    assert(l.size == ZERO);
    assert(l.neurons == NULL);
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

    NNDEBUG("--- NETWORK INITIALIZED ---\n\n");

    printNetworkParameters(&n);

    NNDEBUG("--- FREE NEURAL NETWORK ---\n");
    freeNetwork(&n);
    assert(n.size == ZERO);
    assert(n.layers == NULL);
    NNDEBUG("--- NEURAL NETWORK FREED ---\n\n");
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

    for(uint32 row = (uint32)ZERO; row < dataset->rows; ++row)
    {
        data.data = &dataset->features[(row * dataset->featuresColumns)];
        actualValue.data = &dataset->labels[(row * dataset->labelsColumns)];
        validationFeedForward(network, &data);
        pTensor predictionError = substractTensors(network->layers[network->size - 1].activations, &actualValue); //equates activation derivative
        squareTensor(predictionError);
        
        double32 loss = computeBinaryCrossEntropyLoss(network->layers[network->size - 1].activations, &actualValue);

        NNINFO("Validation loss:%lf", loss);
        
        printTensor(network->layers[network->size - 1].activations);
        printTensor(&actualValue);
        printTensor(predictionError);

        if(actualValue.data[1] == ONE)
        {
            if(fabs(predictionError->data[1]) < 0.9)
            {
                ++TP;
            }else
            {
                ++FN;       
            }
        }else if(actualValue.data[0] == ONE)
        {
            if(fabs(predictionError->data[0]) > 0.9)
            {
                ++FP;
            }else 
            {
                ++TN;
            }
            
        }
    }
    
    uint32 total = TP + TN + FP + FN;
    
    double32 accuracy = (double32)((double32)(TP + TN) / total),
             recall = (double32)((double32)(TP) / (TP + FN)),
             precision = (double32)((double32)(TP) / (TP + FP)),
             f1 = 2.0 * (double32)((precision * recall) / (precision + recall)),
             specificity = (double32)((double32)(TN) / (TN + FP));
    NNINFO("Precision:%lf\nRecall:%lf\nF1 Score:%lf\nSpecificity:%lf\nAccuracy:%lf\n", precision, recall, f1, specificity, accuracy);
    NNINFO("True Positive: %d\nTrue Negative: %d\nFalse Positive: %d\nFalse Negative: %d\n", TP, TN, FP, FN);
}


int main(void)
{
  //  testNeuron();
  //  testLayer();
  //  testNetwork();

    pSChar8 source[11] = {  "/home/catalin/Datasets/CIC_2019/03-11/Portmap.csv",
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
    cic_dataset dataset, mix, validation;
    network n;
    uint32 layerSizes[3],
           inputSize[3];
    
    uint32 datasetMixSize = 9800000;
    mix.featuresColumns = 60;
    //mix.featuresColumns = 10;
    mix.labelsColumns = 2;
    mix.rows = (uint32)ZERO;
    mix.features = malloc((sizeof *mix.features) * datasetMixSize * mix.featuresColumns);
    mix.labels = malloc((sizeof *mix.labels) * datasetMixSize * mix.labelsColumns);
        
    for(uint32 sources = (uint32)ZERO; sources < 11; ++sources)
    {
        NNDEBUG("process: %s\n", source[sources]);
        processData(source[sources], &dataset);
        NNDEBUG("after process");
        checkAndCleanData(&dataset);
        copyData(&mix, &dataset);

        free(dataset.features);
        dataset.features = NULL;
        free(dataset.labels);
        dataset.labels = NULL;
    }

    suffleData(&mix);
    checkAndCleanData(&mix);

    NNDEBUG("Construct neural network");
    layerSizes[0] = dataset.featuresColumns; 
    layerSizes[1] = dataset.featuresColumns / 4;
    layerSizes[2] = dataset.labelsColumns;
            
    inputSize[0] = dataset.featuresColumns;
    inputSize[1] = dataset.featuresColumns;
    inputSize[2] = dataset.featuresColumns / 4;
    createNetwork(&n, 3, layerSizes, inputSize);
    initializeNetwork(&n);
    
    uint32 epochPatience = 5,
           noImprovement = (uint32)ZERO,
           epoch = (uint32)ZERO,
           maxEpochs = 5;
    //double32 validationLoss = ZERO,
    //         bestValue = DBL_MAX;

    NNINFO("Start Network training");
    while(epoch < maxEpochs && noImprovement < epochPatience)
    {
        NNINFO("Epoch:%d", epoch);
        tensor data, actualValue;
        data.size = mix.featuresColumns;
        actualValue.size = mix.labelsColumns;   
        
        for(uint32 line = (uint32)ZERO; line < mix.rows; ++line)
        {
            NNDEBUG("Iteration: %d", line);
            data.data = &mix.features[(line * mix.featuresColumns)];
            actualValue.data = &mix.labels[(line * mix.labelsColumns)];
            double32 returnCost = train(&n, &data, &actualValue);
            
            if((line % 500000) == 0) NNINFO("Network cost: %lf", returnCost);
        }
        /*Early stopping implementation. Uncomment to enable it.
        cic_dataset temp;
        processData("/home/catalin/Datasets/CIC_2019/test.csv", &temp);
        data.size = temp.featuresColumns;
        actualValue.size = temp.labelsColumns;
        double loss = ZERO;
        for(uint32 row = 0; row < temp.rows; ++row)
        {
            data.data = &temp.features[(row * temp.featuresColumns)];
            actualValue.data = &temp.labels[(row * temp.labelsColumns)];
            validationFeedForward(&n, &data);
            pTensor predictionError = substractTensors(n.layers[n.size - 1].activations, &actualValue); //equates activation derivative
            squareTensor(predictionError);
        
            loss += computeLoss(n.layers[n.size - 1].activations, &actualValue);
            validationLoss += loss;
        }
        
        loss /= n.layers[(uint32)ZERO].size;

        free(temp.features);
        temp.features = NULL;
        free(temp.labels);
        temp.labels = NULL;
       
        if(validationLoss + epsilon < bestValue)
        {
            bestValue = validationLoss;
            noImprovement = (uint)ZERO;
        }else ++noImprovement;
        */
        ++epoch;
    }

    NNINFO("Network training completed!");

    free(mix.features);
    mix.features = NULL;
    free(mix.labels);
    mix.labels = NULL;
   
    system("echo root | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches'");
    
    NNINFO("Begin Network inference!");
    processData("/home/catalin/Datasets/CIC_2019/test.csv", &validation);
    NNINFO("Clean validation data!");
    checkAndCleanData(&validation);
    
    NNINFO("Start validation process!");
    test(&validation, &n);

    free(validation.features);
    validation.features = NULL;
    free(validation.labels);
    validation.labels = NULL;
    
    printNetworkParameters(&n);

    freeNetwork(&n);
    return 0;
}
