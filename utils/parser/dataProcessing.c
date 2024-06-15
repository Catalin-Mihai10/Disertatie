#include "dataProcessing.h"
#include "../../interface/constants.h"
#include "../../interface/basic/libraries.h"

#include <math.h>

void checkAndCleanData(pCicDataset mix)
{
    double32 epsilon = 1e-3;
    for(uint32 line = (uint32)ZERO; line < mix->rows; ++line)
    {
        for(uint32 column = (uint32)ZERO; column < mix->featuresColumns; ++column)
        {
            assert(!isnan(mix->features[(line * mix->featuresColumns) + column]));
            assert(!isinf(mix->features[(line * mix->featuresColumns) + column]));
        
            if(mix->features[(line * mix->featuresColumns) + column] == ZERO)
            {
                mix->features[(line * mix->featuresColumns) + column] += epsilon;
            }

            assert(mix->features[(line * mix->featuresColumns) + column] != ZERO);
        }
    }
    
    for(uint32 line = (uint32)ZERO; line < mix->rows; ++line)
    {
        if((mix->labels[(line * mix->labelsColumns)] == ZERO) && (mix->labels[(line * mix->labelsColumns) +  1] == ZERO))
        {
            mix->labels[(line * mix->labelsColumns)] = ONE;
        }
        for(uint32 column = (uint32)ZERO; column < mix->labelsColumns; ++column)
        {
            assert(!isnan(mix->labels[(line * mix->labelsColumns)+ column]));
            assert(!isinf(mix->labels[(line * mix->labelsColumns) + column]));
        }
    }


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

void processData(const pSChar8 source, pCicDataset dataset)
{
    tcpString data;
    uint32 read = fileReading(source, &data);
    if(read == READ_ERROR)
    {
        free(data.data);
        data.data = NULL;
        printf("Could not read data file!\n");
        return;
    }
    parseCsvData(&data, dataset);
    //parseCsvData_less_features(&data, dataset);
    //parseKDD(&data, dataset);
    free(data.data);
    data.data = NULL;
    normalizeCsvData(dataset, dataset->rows);
}
