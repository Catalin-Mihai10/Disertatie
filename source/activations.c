#include "../interface/activations.h"
#include "../interface/constants.h"
#include <math.h>
#include <float.h>

/* ACTIVATIONS IMPLEMENTATION */
double32 sigmoid(double32 data)
{
    return ONE / (ONE + expf((-ONE) * data));
}

double32 relu(double32 data)
{
    return (data > ZERO)? data:ZERO;
}

double32 leakyRelu(double32 data)
{
    double32 param = 0.01;
    return (data > ZERO)? data:param * data;
}

void softmax(pTensor activations) 
{
    double32 maximum = -DBL_MAX,
             sum = ZERO,
             scaling = ZERO,
             epsilon = 1e-3;

    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        if(maximum < activations->data[iterator]) maximum = activations->data[iterator];
    }
    
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        sum += expf(activations->data[iterator] - maximum);
    }
    
    if(sum == ZERO)
    {
        sum += epsilon;
    }

    scaling = maximum + log(sum);
    for(uint32 iterator = (uint32)ZERO; iterator < activations->size; ++iterator)
    {
        activations->data[iterator] = expf(activations->data[iterator] - scaling);
        assert(!isnan(activations->data[iterator]));   
    }
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

/* DERIVATIVES IMPLEMENTATION */
double32 leakyReluDerivative(double32 data)
{
    double param = 0.01;
    return (data > ZERO)? ONE:param;
}

double32 reluDerivative(double32 activation)
{
    return (activation > ZERO)? ONE:ZERO;
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


