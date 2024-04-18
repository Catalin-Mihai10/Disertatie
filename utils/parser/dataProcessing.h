#ifndef DATAPROCESSING_HEADER
#define DATAPROCESSING_HEADER

#include "parser.h"

void checkAndCleanData(pCicDataset mix);
void suffleData(pCicDataset mix);
void copyData(pCicDataset mix, pCicDataset dataset);

void processData(const pSChar8 source, pCicDataset dataset);
#endif /* DATAPROCESSING_HEADER */
