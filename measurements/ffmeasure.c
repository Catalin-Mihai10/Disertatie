#include "../interface/network.h"
#include "../utils/logger/NNLogger.h"
#include "../interface/activations.h"

#include <time.h>

int main(void)
{
    NNINFO("Create dummy layer");
    layer testL;
    testL.size = 60;
    createLayer(&testL, 60);
    initializeLayer(&testL);
    
    NNINFO("Create dummy data!");
    tensor testData, temp;
    uint32 dataLines = 100000; 
    testData.size = 60;
    testData.data = malloc((sizeof *testData.data) * 60 * dataLines);
    temp.size = testData.size;
    randomizeTensor(&testData);

    network n;
    uint32 layerSize[3],
            inputSize[3];

    layerSize[0] = testData.size;
    layerSize[1] = testData.size / 4;
    layerSize[2] = 2;

    inputSize[0] = testData.size;
    inputSize[1] = testData.size;
    inputSize[2] = testData.size / 4;
    createNetwork(&n, 3, layerSize, inputSize);
    initializeNetwork(&n);

    NNINFO("make clocks");
    clock_t start, end, ffStart, ffEnd;
    uint32 iterationts = 100;
    long double interationsTime[iterationts];
    memset(&interationsTime, 0x00, (sizeof *interationsTime) * iterationts);
    pDouble64 ffTime;
    NNINFO("ALLOC space");
    //memset(&ffTime, 0x00, (sizeof *ffTime) * iterationts * dataLines);

    NNINFO("Start measurements\n");
    double64 averageFFTime = ZERO;
    uint32 index = 0;
    activations activation = LRELU;
    for(uint32 iteration = 0; iteration < iterationts; ++iteration)
    {
        NNINFO("Iteration %d", iteration);
        start = clock();
        for(uint32 line = 0; line < dataLines; ++line)
        {
            temp.data = &testData.data[line * 60];
            for(uint32 layer = (uint32)ONE; layer < n.size; ++layer)
            {
                if(layer == n.size - 1) activation = SOFTMAX;
                if(layer == (uint32)ZERO) feedforward(&n.layers[layer], &temp, activation);
                else feedforward(&n.layers[layer], n.layers[layer - 1].activations, activation);
            }
        }
        end = clock();
        interationsTime[iteration] = (long double) (end - start) / (long double) CLOCKS_PER_SEC;
    }
    NNINFO("End measurements");
    averageFFTime /= index;

    long double averageDatasetParseTime = ZERO;
    for(uint32 samples = 0; samples < iterationts; ++samples)
    {
        averageDatasetParseTime += interationsTime[samples];
    }
    averageDatasetParseTime /= iterationts;
    
    NNWARN("Average Feed Forward Execution Time: %.10Lf s\n", averageFFTime);
    NNWARN("Average Test Dataset Completition Time: %.10Lf s\n", averageDatasetParseTime);
    return 0;
}
