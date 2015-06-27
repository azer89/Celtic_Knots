#ifndef RIBBONSEGMENT_H
#define RIBBONSEGMENT_H

#include "AVector.h"
#include "LayerType.h"

struct RibbonSegment
{
public:
    AVector _startMPt;
    AVector _endMPt;

    AVector _anchor1;
    AVector _anchor2;

    AVector _startRPt;
    AVector _endRPt;

    AVector _startLPt;
    AVector _endLPt;

    LayerType _layerType;
};

#endif // RIBBONSEGMENT_H
