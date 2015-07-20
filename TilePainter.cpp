
#include "TilePainter.h"
#include "VertexData.h"
#include "CurveInterpolation.h"

#include <stdlib.h>
#include <iostream>


TilePainter::TilePainter()
{
}

TilePainter::~TilePainter()
{
}

CornerCase TilePainter::GetCornerCase(int i, std::vector<std::vector<CCell>> cells, std::vector<AnIndex> traceList, bool isTracingDone)
{
    if(traceList.size() == 1)
    {
        // nothing
    }
    else if(traceList.size() == 2)
    {
        CCell c1 = cells[traceList[0].x][traceList[0].y];
        CCell c2 = cells[traceList[1].x][traceList[1].y];
        bool changeDir = c1._directionType != c2._directionType;

        if(i == 0 && changeDir )  // start ?
            { return CornerCase::COR_START; }
        else if(i == 1 && changeDir) // end ?
            { return CornerCase::COR_END; }
    }
    else if(traceList.size() >= 3)
    {
        size_t curI = i;
        size_t prevI = i - 1;
        size_t nextI = i + 1;

        if(curI == 0)
        {
            if(isTracingDone) prevI = traceList.size() - 2;
            else prevI = traceList.size() - 1;
        }
        if(curI == traceList.size() - 1)
        {
            nextI = 0;
        }

        CCell prevC = cells[traceList[prevI].x][traceList[prevI].y];
        CCell curC = cells[traceList[curI].x][traceList[curI].y];
        CCell nextC = cells[traceList[nextI].x][traceList[nextI].y];

        // new code
        if(nextC._straightness  != Straightness::ST_DIAGONAL &&
           curC._straightness   == Straightness::ST_DIAGONAL &&
           prevC._straightness  != Straightness::ST_DIAGONAL)
        {
            return CornerCase::COR_MIDDLE_STRAIGHT;
        }
        else if(nextC._straightness != Straightness::ST_DIAGONAL &&
           curC._straightness  == Straightness::ST_DIAGONAL)
        {
            return CornerCase::COR_START_STRAIGHT;
        }
        else if(prevC._straightness != Straightness::ST_DIAGONAL &&
                curC._straightness  == Straightness::ST_DIAGONAL)
        {
            return CornerCase::COR_END_STRAIGHT;
        }
        else if(curC._straightness != Straightness::ST_DIAGONAL)
        {
            return CornerCase::COR_STRAIGHT;
        }

        // old code
        // middle
        else if(curC._directionType != prevC._directionType && curC._directionType != nextC._directionType)
        {
            //std::cout << "1 ";
            return CornerCase::COR_MIDDLE;
        }

        // start
        else if(curC._directionType == prevC._directionType &&
                curC._directionType != nextC._directionType)
        {
            //std::cout << "2 " << " (" << prevI << "," << curI << "," << nextI << ") ";
            return CornerCase::COR_START;
        }

        // end
        else if(curC._directionType != prevC._directionType &&
                curC._directionType == nextC._directionType)
        {
            //std::cout << "3 ";
            return CornerCase::COR_END;
        }
    }

    //std::cout << "4 ";
    return CornerCase::COR_NORMAL;
}

AVector TilePainter::GetMiddlePoint(AVector a, AVector b, AVector c)
{
    AVector ba = b - a;
    AVector cb = c - b;
    return (((ba + cb) * 0.5f) - ba) * 0.5f;
}

bool TilePainter::IsEven(int num)
{
    return (num % 2) == 0;
}

std::pair<LayerType, LayerType> TilePainter::GetLayerTypes(CCell curCel, AnIndex curIdx)
{
    // determine layer types (start end)
    std::pair<LayerType, LayerType> lTypes;
    if (curCel._directionType == DirectionType::DIR_UPRIGHT)
    {
        if(IsEven(curIdx.y) && IsEven(curIdx.y))    // UO
            { lTypes.first = LayerType::LAYER_UNDER; lTypes.second = LayerType::LAYER_OVER; }
        else /*if(!IsEven(curIdx.x) && !IsEven(curIdx.y))*/ // OU
            { lTypes.first = LayerType::LAYER_OVER; lTypes.second = LayerType::LAYER_UNDER; }
    }
    else if (curCel._directionType == DirectionType::DIR_DOWNRIGHT)
    {
        if(!IsEven(curIdx.x) && IsEven(curIdx.y))   // UO
            { lTypes.first = LayerType::LAYER_UNDER; lTypes.second = LayerType::LAYER_OVER; }
        else /*if(IsEven(curIdx.x) && !IsEven(curIdx.y))*/  // OU
            { lTypes.first = LayerType::LAYER_OVER; lTypes.second = LayerType::LAYER_UNDER; }
    }
    else if (curCel._directionType == DirectionType::DIR_DOWNLEFT)
    {
        if(IsEven(curIdx.y) && IsEven(curIdx.y)) // OU
            { lTypes.first = LayerType::LAYER_OVER; lTypes.second = LayerType::LAYER_UNDER; }
        else /*if(!IsEven(curIdx.x) && !IsEven(curIdx.y))*/ // UO
            { lTypes.first = LayerType::LAYER_UNDER; lTypes.second = LayerType::LAYER_OVER; }
    }
    else if (curCel._directionType == DirectionType::DIR_UPLEFT)
    {
        if(!IsEven(curIdx.x) && IsEven(curIdx.y))   // OU
            { lTypes.first = LayerType::LAYER_OVER; lTypes.second = LayerType::LAYER_UNDER; }
        else /*if(IsEven(curIdx.x) && !IsEven(curIdx.y))*/  // UO
            { lTypes.first = LayerType::LAYER_UNDER; lTypes.second = LayerType::LAYER_OVER; }
    }
    return lTypes;
}

ALine TilePainter::GetLineInACell(CCell curCel, float gridSpacing, AnIndex curIdx)
{
    float halfGridSpacing = gridSpacing * 0.5f;

    ALine ln;
    if (curCel._directionType == DirectionType::DIR_UPRIGHT)
        { ln = ALine(curIdx.x * gridSpacing, (curIdx.y + 1) * gridSpacing,
                    (curIdx.x + 1) * gridSpacing, curIdx.y * gridSpacing); }
    else if (curCel._directionType == DirectionType::DIR_DOWNRIGHT)
        { ln = ALine(curIdx.x * gridSpacing, curIdx.y * gridSpacing,
                    (curIdx.x + 1) * gridSpacing, (curIdx.y + 1) * gridSpacing); }
    else if (curCel._directionType == DirectionType::DIR_DOWNLEFT)
        { ln = ALine((curIdx.x + 1) * gridSpacing, curIdx.y * gridSpacing,
                     curIdx.x * gridSpacing, (curIdx.y + 1) * gridSpacing); }
    else if (curCel._directionType == DirectionType::DIR_UPLEFT)
        { ln = ALine((curIdx.x + 1) * gridSpacing, (curIdx.y + 1) * gridSpacing,
                      curIdx.x * gridSpacing, curIdx.y * gridSpacing); }

    else if (curCel._directionType == DirectionType::DIR_RIGHT)
    {
        ln = ALine((curIdx.x)     * gridSpacing,
                   (curIdx.y)     * gridSpacing + halfGridSpacing,
                   (curIdx.x + 1) * gridSpacing,
                   (curIdx.y)     * gridSpacing + halfGridSpacing);
    }
    else if (curCel._directionType == DirectionType::DIR_LEFT)
    {
        ln = ALine((curIdx.x + 1) * gridSpacing,
                   (curIdx.y)     * gridSpacing + halfGridSpacing,
                   (curIdx.x)     * gridSpacing,
                   (curIdx.y)     * gridSpacing + halfGridSpacing);
    }
    else if (curCel._directionType == DirectionType::DIR_UP)
    {
        ln = ALine((curIdx.x)     * gridSpacing + halfGridSpacing,
                   (curIdx.y + 1) * gridSpacing,
                   (curIdx.x)     * gridSpacing + halfGridSpacing,
                   (curIdx.y)     * gridSpacing);
    }
    else if (curCel._directionType == DirectionType::DIR_DOWN)
    {
        ln = ALine((curIdx.x)     * gridSpacing + halfGridSpacing,
                   (curIdx.y)     * gridSpacing,
                   (curIdx.x)     * gridSpacing + halfGridSpacing,
                   (curIdx.y + 1) * gridSpacing);
    }



    return ln;
}

void TilePainter::RefineLines(std::vector<ALine>& lines, std::vector<CornerCase> ccs,  bool isTracingDone)
{
    std::vector<ALine> tempLines1(lines);
    for(size_t a = 0; a < lines.size() && lines.size() > 1; a++)
    {
        size_t curI = a;
        size_t prevI = a - 1;
        size_t nextI = (a + 1) % lines.size();
        if(curI == 0) { prevI = lines.size() - 1; }


        AVector offsetVec1 = GetMiddlePoint(tempLines1[prevI].GetPointA(),
                                            tempLines1[curI].GetPointA(),
                                            tempLines1[curI].GetPointB());

        AVector offsetVec2 = GetMiddlePoint(tempLines1[curI].GetPointA(),
                                            tempLines1[curI].GetPointB(),
                                            tempLines1[nextI].GetPointB());


        float normalFactor = 1.0;
        //float normalFactor = 0.8;


        // fix me: nasty code
        if(curI == 0 && ccs[prevI] == CornerCase::COR_STRAIGHT)
        {
            ALine prevLine = tempLines1[prevI];
            lines[curI].XA = prevLine.XB;
            lines[curI].YA = prevLine.YB;
        }

        // old code
        if(ccs[a] == CornerCase::COR_START)
        {
            lines[curI].XB += offsetVec2.x * normalFactor;
            lines[curI].YB += offsetVec2.y * normalFactor;
        }
        else if(ccs[a] == CornerCase::COR_END)
        {
            lines[curI].XA += offsetVec1.x * normalFactor;
            lines[curI].YA += offsetVec1.y * normalFactor;
        }
        else if(ccs[a] == CornerCase::COR_MIDDLE)
        {
            lines[curI].XA += offsetVec1.x * normalFactor;
            lines[curI].YA += offsetVec1.y * normalFactor;
            lines[curI].XB += offsetVec2.x * normalFactor;
            lines[curI].YB += offsetVec2.y * normalFactor;
        }

        // to do:
        // if nextI is COR_START_STRAIGHT
        // if prevI is COR_END_STRAIGHT

        else if(ccs[a] == CornerCase::COR_START_STRAIGHT)
        {
            lines[curI].XA += offsetVec1.x * normalFactor;
            lines[curI].YA += offsetVec1.y * normalFactor;

            if(nextI != 0)
            {
                ALine nextLine = tempLines1[nextI];
                lines[curI].XB = nextLine.XA;
                lines[curI].YB = nextLine.YA;
            }
        }
        else if(ccs[a] == CornerCase::COR_END_STRAIGHT)
        {
            if(prevI != tempLines1.size() - 1)
            {
                ALine prevLine = tempLines1[prevI];
                lines[curI].XA = prevLine.XB;
                lines[curI].YA = prevLine.YB;
            }

            lines[curI].XB += offsetVec2.x * normalFactor;
            lines[curI].YB += offsetVec2.y * normalFactor;
        }
        else if(ccs[a] == CornerCase::COR_MIDDLE_STRAIGHT)
        {
            if(nextI != 0)
            {
                ALine nextLine = tempLines1[nextI];
                lines[curI].XB = nextLine.XA;
                lines[curI].YB = nextLine.YA;
            }

            if(prevI != tempLines1.size() - 1)
            {
                ALine prevLine = tempLines1[prevI];
                lines[curI].XA = prevLine.XB;
                lines[curI].YA = prevLine.YB;
            }
        }
        //COR_MIDDLE_STRAIGHT


        // end code
        if(curI == lines.size() - 1 && !isTracingDone)
        {
            lines[curI].XB = tempLines1[curI].XB;
            lines[curI].YB = tempLines1[curI].YB;
        }

        if(curI == 0 && !isTracingDone)
        {
            lines[curI].XA = tempLines1[curI].XA;
            lines[curI].YA = tempLines1[curI].YA;
        }
    }
}


void TilePainter::SetTiles(std::vector<std::vector<CCell>> cells,
                           std::vector<AnIndex> traceList,
                           float gridSpacing,
                           bool isTracingDone)
{
    _cLines.clear();

    std::vector<std::pair<LayerType, LayerType>> layerTypeList1;
    std::vector<LayerType> layerTypeList2;
    std::vector<CornerCase> ccs;

    // because first and last are the same
    size_t lengthLimit = traceList.size();
    if(traceList.size() > 1)
    {
        AnIndex firstIdx = traceList[0];
        AnIndex lastIdx = traceList[traceList.size() - 1];
        if(firstIdx == lastIdx) { lengthLimit = traceList.size() - 1; }
    }

    for(size_t a = 0; a < lengthLimit; a++)
    {
        AnIndex curIdx = traceList[a];
        CCell curCel = cells[curIdx.x][curIdx.y];
        CornerCase cc = GetCornerCase(a, cells, traceList, isTracingDone);
        ccs.push_back(cc);

        // calculate an initial line
        ALine ln = GetLineInACell(curCel, gridSpacing, curIdx);
        ln.index1 = curIdx.x;
        ln.index2 = curIdx.y;

        // determine layer types (start end)
        std::pair<LayerType, LayerType> lTypes = GetLayerTypes(curCel, curIdx);
        layerTypeList1.push_back(lTypes);  //lTypes
        _cLines.push_back(ln);
    }

    // detection is at least four points
    // only when isTracingDone == true
    // DetectStraightLines(std::vector<CornerCase> ccs)

    // refine lines
    RefineLines(_cLines, ccs, isTracingDone);


    /*
    // to do: refine corner
    std::vector<ALine> tempLines;
    for(size_t a = 0; a < _cLines.size(); a++)
    {
        CornerCase cCase = ccs[a];
        ALine aLine = _cLines[a];

        if(cCase == CornerCase::COR_MIDDLE)
        {
            float xPos = gridSpacing * (float)(aLine.index1) + (gridSpacing / 2.0f);
            float yPos = gridSpacing * (float)(aLine.index2) + (gridSpacing / 2.0f);

            ALine lineA(aLine.XA, aLine.YA, xPos, yPos);
            ALine lineB(xPos, yPos, aLine.XB, aLine.YB);

            tempLines.push_back(lineA);
            tempLines.push_back(lineB);

        }
        else
        {
            tempLines.push_back(aLine);
        }
    }
    _cLines = tempLines;
    */

    // an example of acute corner
    /*
    ALine firstLine = _cLines[0];
    ALine lineA(firstLine.XA, firstLine.YA, gridSpacing / 4.0f, gridSpacing / 4.0f);
    ALine lineB(gridSpacing / 4.0f, gridSpacing / 4.0f, firstLine.XB, firstLine.YB);
    _cLines.erase(_cLines.begin());
    _cLines.insert(_cLines.begin(), lineB);
    _cLines.insert(_cLines.begin(), lineA);
    */


    // bezier curves
    if(isTracingDone)
    //if(false)
    {
        _urSegments.clear();
        _orSegments.clear();
        _anchorLines,clearenv();
        _points.clear();

        std::vector<std::pair<AVector, AVector>> anchors;
        // init
        for(size_t a = 0; a < _cLines.size(); a++)
        {
            int curIdx = a;
            int prevIdx = a - 1;
            int nextIdx = (a + 1) % _cLines.size();

            if(curIdx == 0) { prevIdx = _cLines.size() - 1; }

            AVector pt1 = _cLines[prevIdx].GetPointA();
            AVector pt2 = _cLines[curIdx].GetPointA();
            AVector pt3 = _cLines[curIdx].GetPointB();
            AVector pt4 = _cLines[nextIdx].GetPointB();

            AVector anchor1, anchor2;
            //CurveInterpolation::GetAnchors(pt1, pt2, pt3, pt4, anchor1, anchor2, 0.75);
            CurveInterpolation::GetAnchors(pt1, pt2, pt3, pt4, anchor1, anchor2, 0.9);

            /*
            // (Bx-Ax)*(Y-Ay) - (By-Ay)*(X-Ax)
            float val1 = (pt3.x - pt2.x) * (anchor1.y - pt2.y) - (pt3.y - pt2.y) * (anchor1.x - pt2.x);
            float val2 = (pt3.x - pt2.x) * (anchor2.y - pt2.y) - (pt3.y - pt2.y) * (anchor2.x - pt2.x);

            if((val1 < 0 && val2 > 0) || (val2 < 0 && val1 > 0))
            {
                anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0;
                anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0;
            }*/


            // code to fix anchor points


            float angle1 = AngleInBetween(anchor1 - pt2, pt3 - pt2);
            float angle2 = AngleInBetween(anchor2 - pt3, pt2 - pt3);


            CornerCase cCase = ccs[curIdx];
            if(cCase == CornerCase::COR_STRAIGHT)
            {
                anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0;
                anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0;
            }


            if(DistanceToFiniteLine(pt2, pt3, anchor1) < 0.01)
            {
                anchor1 = pt2;
                std::cout << "ok1\n";
            }
            if(DistanceToFiniteLine(pt2, pt3, anchor2) < 0.01)
            {
                anchor2 = pt3;
                std::cout << "ok2\n";
            }

            if(angle1 <= 0.15) { anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0; }
            if(angle2 <= 0.15) { anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0; }


            //if(angle1 <= 0.15 && angle2 <= 0.15)
            //{
            //    ccs[curIdx] = CornerCase::COR_STRAIGHT;
            //}

            std::pair<AVector, AVector> pAnchors(anchor1, anchor2);
            anchors.push_back(pAnchors);
        }

        // refine anchors (more straight)
        /*
        for(size_t a = 0; a < _cLines.size(); a++)
        {
            int curIdx = a;
            int prevIdx = a - 1;
            int nextIdx = (a + 1) % _cLines.size();

            if(curIdx == 0) { prevIdx = _cLines.size() - 1; }

            AVector pt2 = _cLines[curIdx].GetPointA();
            AVector pt3 = _cLines[curIdx].GetPointB();

            //AVector anchor1 = anchors[a].first;
            //AVector anchor2 = anchors[a].second;

            if(ccs[nextIdx] == CornerCase::COR_STRAIGHT)
                { anchors[a].second = pt3 + (pt2 - pt3).Norm() * 1.0; }

            if(ccs[prevIdx] == CornerCase::COR_STRAIGHT)
                { anchors[a].first = pt2 + (pt3 - pt2).Norm() * 1.0; }
        }
        */


        for(size_t a = 0; a < _cLines.size(); a++)
        {
            int curIdx = a;
            int prevIdx = a - 1;
            int nextIdx = (a + 1) % _cLines.size();

            if(curIdx == 0) { prevIdx = _cLines.size() - 1; }

            AVector pt2 = _cLines[curIdx].GetPointA();
            AVector pt3 = _cLines[curIdx].GetPointB();
            AVector anchor1 = anchors[a].first;
            AVector anchor2 = anchors[a].second;

            // layer types
            std::pair<LayerType, LayerType> lTypes = layerTypeList1[curIdx];

            /*
            CornerCase cCase = ccs[curIdx];
            if(cCase == CornerCase::COR_STRAIGHT)
            {
                anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0;
                anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0;
            }
            */

            /*
            // didn't work ???
            float angle1 = AngleInBetween(anchor1 - pt2, pt3 - pt2);
            float angle2 = AngleInBetween(anchor2 - pt3, pt2 - pt3);
            std::cout << "angles " << angle1 << " " << angle2 << "\n";
            if(angle1 <= 0.15) { anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0; }
            if(angle2 <= 0.15) { anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0; }
            */

            RibbonSegment segment1;
            RibbonSegment segment2;
            std::vector<AVector> segmentPoints1;
            std::vector<AVector> segmentPoints2;

            GeTwoSegments(pt2, anchor1, anchor2, pt3, &segment1, &segment2);

            // debug (please delete)
            _anchorLines.push_back(ALine(pt2, anchor1));
            _anchorLines.push_back(ALine(pt3, anchor2));

            CurveInterpolation::DeCasteljau(segmentPoints1, segment1._startMPt, segment1._anchor1, segment1._anchor2, segment1._endMPt, 1.0f);
            CurveInterpolation::DeCasteljau(segmentPoints2, segment2._startMPt, segment2._anchor1, segment2._anchor2, segment2._endMPt, 1.0f);

            for(size_t a = 0; a < segmentPoints1.size(); a++)
                { layerTypeList2.push_back(lTypes.first); }
            for(size_t a = 0; a < segmentPoints2.size(); a++)
                { layerTypeList2.push_back(lTypes.second); }

            _points.insert(_points.end(), segmentPoints1.begin(), segmentPoints1.end());
            _points.insert(_points.end(), segmentPoints2.begin(), segmentPoints2.end());

            segment1._layerType = lTypes.first;
            segment2._layerType = lTypes.second;
            segment1._cLines.clear();
            segment2._cLines.clear();
            for(size_t a = 0; a < segmentPoints1.size() - 1; a++)
            {
                int idx1 = a; int idx2 = a + 1;
                segment1._cLines.push_back(ALine(segmentPoints1[idx1], segmentPoints1[idx2]));
            }
            segment1._cLines.push_back(ALine(segmentPoints1[segmentPoints1.size() - 1], segment1._endMPt));
            for(size_t a = 0; a < segmentPoints2.size() - 1; a++)
            {
                int idx1 = a; int idx2 = a + 1;
                segment2._cLines.push_back(ALine(segmentPoints2[idx1], segmentPoints2[idx2]));
            }
            segment2._cLines.push_back(ALine(segmentPoints2[segmentPoints2.size() - 1], segment2._endMPt));
            CalculateRibbonLR(&segment1);
            CalculateRibbonLR(&segment2);

            if(segment1._layerType == LayerType::LAYER_OVER)
            {
                _orSegments.push_back(segment1);
                _urSegments.push_back(segment2);
            }
            else
            {
                _urSegments.push_back(segment1);
                _orSegments.push_back(segment2);
            }
        }

        //to do: uncomment this
        _cLines.clear();
        for(size_t a = 0; a < _points.size(); a++)
        {
            int idx1 = a;
            int idx2 = (a + 1) % _points.size();
            _cLines.push_back(ALine(_points[idx1], _points[idx2]));
        }

    }

    if(isTracingDone)
    //if(false)
    {
        _cLinesVao.destroy();
        std::cout << "layerTypeList2.size() " << layerTypeList2.size() << "\n";
        std::rotate(layerTypeList2.begin(), layerTypeList2.begin() + 1, layerTypeList2.end());  // move first to the end
        CalculateOverUnderRibbon(_cLines, layerTypeList2);
        PrepareLinesVAO2(_cLines, &_cLinesVbo, &_cLinesVao, layerTypeList2);
        PrepareLinesVAO1(_anchorLines, &_anchorLinesVbo, &_anchorLinesVao, QVector3D(1.0, 0.0, 0.0));
    }
    else
    {
        PrepareLinesVAO1(_cLines, &_cLinesVbo, &_cLinesVao, QVector3D(1.0, 0.0, 0.0));
        //if(isTracingDone)
        //{
        //    PrepareLinesVAO1(_anchorLines, &_anchorLinesVbo, &_anchorLinesVao, QVector3D(0.0, 0.0, 1.0));
        //}
    }
}

void TilePainter::CalculateOverUnderRibbon(std::vector<ALine> cLines, std::vector<LayerType> layerTypeList)
{
    std::vector<ALine> rLines;
    std::vector<ALine> lLines;
    for(size_t a = 0; a < cLines.size(); a++)
    {
        int curIdx = a;
        int prevIdx = a - 1;
        int nextIdx = a + 1;

        if(curIdx == 0) { prevIdx = cLines.size() - 1; }
        else if(curIdx == cLines.size() - 1) { nextIdx = 0; }

        ALine prevLine = cLines[prevIdx];
        ALine curLine = cLines[curIdx];
        ALine nextLine = cLines[nextIdx];

        AVector d0Left, d0Right, d1Left, d1Right;
        GetSegmentPoints(curLine, prevLine, nextLine, 1.5,1.5, &d0Left, &d0Right, &d1Left, &d1Right);

        lLines.push_back(ALine(d0Left, d1Left));
        rLines.push_back(ALine(d0Right, d1Right));
    }

    _urLines.clear();
    _ulLines.clear();
    _orLines.clear();
    _olLines.clear();
    for(size_t a = 0; a < cLines.size(); a++)
    {
        if(layerTypeList[a] == LayerType::LAYER_UNDER)
        {
            _urLines.push_back(rLines[a]);
            _ulLines.push_back(lLines[a]);
        }
        else
        {
            _orLines.push_back(rLines[a]);
            _olLines.push_back(lLines[a]);
        }
    }

    PrepareQuadsVAO2(_urLines, _ulLines, &_uQuadsVbo, &_uQuadsVao, QVector3D(1, 1, 1));
    PrepareQuadsVAO2(_orLines, _olLines, &_oQuadsVbo, &_oQuadsVao, QVector3D(1, 1, 1));

    PrepareLinesVAO3(_urLines, _ulLines, &_uLinesVbo, &_uLinesVao, QVector3D(0, 0, 0));
    PrepareLinesVAO3(_orLines, _olLines, &_oLinesVbo, &_oLinesVao, QVector3D(0, 0, 0));
}

void TilePainter::CalculateRibbonLR(RibbonSegment* segment)
{
    for(size_t a = 0; a < segment->_cLines.size(); a++)
    {
        int curIdx = a;
        int prevIdx = a - 1;
        int nextIdx = a + 1;

        if(curIdx == 0) { prevIdx = segment->_cLines.size() - 1; }
        else if(curIdx == segment->_cLines.size() - 1) { nextIdx = 0; }

        ALine prevLine = segment->_cLines[prevIdx];
        ALine curLine = segment->_cLines[curIdx];
        ALine nextLine = segment->_cLines[nextIdx];

        AVector d0Left, d0Right, d1Left, d1Right;
        GetSegmentPoints(curLine, prevLine, nextLine, 2, 2, &d0Left, &d0Right, &d1Left, &d1Right);

        segment->_lLines.push_back(ALine(d0Left, d1Left));
        segment->_rLines.push_back(ALine(d0Right, d1Right));
    }
}

void TilePainter::GeTwoSegments(AVector p0, AVector p1, AVector p2, AVector p3, RibbonSegment* segment1, RibbonSegment* segment2)
{
    double x0 = p0.x; double y0 = p0.y;
    double x1 = p1.x; double y1 = p1.y;
    double x2 = p2.x; double y2 = p2.y;
    double x3 = p3.x; double y3 = p3.y;

    double splitParam = 0.5;

    double x01 = (x1 - x0) * splitParam + x0;		double x12 = (x2 - x1) * splitParam + x1;		double x23 = (x3 - x2) * splitParam + x2;
    double y01 = (y1 - y0) * splitParam + y0;		double y12 = (y2 - y1) * splitParam + y1;		double y23 = (y3 - y2) * splitParam + y2;

    double x012 = (x12 - x01) * splitParam + x01;	double x123 = (x23 - x12) * splitParam + x12;
    double y012 = (y12 - y01) * splitParam + y01;	double y123 = (y23 - y12) * splitParam + y12;

    double x0123 = (x123 - x012) * splitParam + x012;
    double y0123 = (y123 - y012) * splitParam + y012;

    // AVector(x0, y0),        AVector(x01, y01),   AVector(x012, y012), AVector(x0123, y0123)
    segment1->_startMPt = AVector(x0, y0);
    segment1->_endMPt = AVector(x0123, y0123);
    segment1->_anchor1 = AVector(x01, y01);
    segment1->_anchor2 = AVector(x012, y012);

    // AVector(x0123,  y0123), AVector(x123, y123), AVector(x23, y23),   AVector(x3, y3)
    segment2->_startMPt = AVector(x0123,  y0123);
    segment2->_endMPt = AVector(x3, y3);
    segment2->_anchor1 = AVector(x123, y123);
    segment2->_anchor2 = AVector(x23, y23);
}

void TilePainter::PrepareLinesVAO3(std::vector<ALine> rLines, std::vector<ALine> lLines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol)
{
    if(vao->isCreated()) { vao->destroy(); }

    vao->create();
    vao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < rLines.size(); a++)
    {
        data.append(VertexData(QVector3D(rLines[a].XA, rLines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(rLines[a].XB, rLines[a].YB,  0), QVector2D(), vecCol));

        data.append(VertexData(QVector3D(lLines[a].XA, lLines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lLines[a].XB, lLines[a].YB,  0), QVector2D(), vecCol));
    }

    vbo->create();
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    vao->release();
}

void TilePainter::PrepareQuadsVAO2(std::vector<ALine> rLines, std::vector<ALine> lLines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol)
{
    if(vao->isCreated()) { vao->destroy(); }

    vao->create();
    vao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < rLines.size(); a++)
    {
        data.append(VertexData(QVector3D(rLines[a].XA, rLines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lLines[a].XA, lLines[a].YA,  0), QVector2D(), vecCol));

        data.append(VertexData(QVector3D(lLines[a].XB, lLines[a].YB,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(rLines[a].XB, rLines[a].YB,  0), QVector2D(), vecCol));

    }

    vbo->create();
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    vao->release();
}

int TilePainter::PrepareQuadsVAO1(std::vector<RibbonSegment> ribbonSegments, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol)
{
    if(vao->isCreated()) { vao->destroy(); }

    vao->create();
    vao->bind();

    int nSize = 0;
    QVector<VertexData> data;
    for(uint a = 0; a < ribbonSegments.size(); a++)
    {
        /*
        std::vector<ALine> rLines = ribbonSegments[a]._rLines;
        std::vector<ALine> lLines = ribbonSegments[a]._lLines;
        nSize += cLines.size() * 4;
        for(uint b = 0; b < cLines.size(); b++)
        {
            data.append(VertexData(QVector3D(rLines[b].XA, rLines[b].YA,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(rLines[b].XB, rLines[b].YB,  0), QVector2D(), vecCol));

            data.append(VertexData(QVector3D(lLines[b].XA, lLines[b].YA,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(lLines[b].XB, lLines[b].YB,  0), QVector2D(), vecCol));
        }
        */


        std::vector<ALine> cLines = ribbonSegments[a]._cLines;
        nSize += cLines.size() * 2;
        for(uint b = 0; b < cLines.size(); b++)
        {
            data.append(VertexData(QVector3D(cLines[b].XA, cLines[b].YA,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(cLines[b].XB, cLines[b].YB,  0), QVector2D(), vecCol));
        }

    }

    vbo->create();
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    vao->release();

    return nSize;
}

void TilePainter::PrepareLinesVAO1(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
{
    if(linesVao->isCreated())
    {
        linesVao->destroy();
    }

    linesVao->create();
    linesVao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        data.append(VertexData(QVector3D(lines[a].XA, lines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lines[a].XB, lines[a].YB,  0), QVector2D(), vecCol));
    }

    linesVbo->create();
    linesVbo->bind();
    linesVbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    linesVao->release();
}

void TilePainter::PrepareLinesVAO2(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, std::vector<LayerType> layerTypeList)
{
    if(linesVao->isCreated())
    {
        linesVao->destroy();
    }

    linesVao->create();
    linesVao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        QVector3D vecCol(1, 1, 0);
        if(layerTypeList[a] == LayerType::LAYER_UNDER) {vecCol = QVector3D(0, 1, 1); }

        data.append(VertexData(QVector3D(lines[a].XA, lines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lines[a].XB, lines[a].YB,  0), QVector2D(), vecCol));
    }

    linesVbo->create();
    linesVbo->bind();
    linesVbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    linesVao->release();
}

void TilePainter::DrawTiles()
{
    /*
    if(_anchorLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _anchorLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _cLines.size() * 2);
        _anchorLinesVao.release();
    }*/

    if(_uQuadsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        _uQuadsVao.bind();
        glDrawArrays(GL_QUADS, 0, _urLines.size() * 4);
        _uQuadsVao.release();
    }

    if(_uLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        _uLinesVao.bind();
        glLineWidth(6.0f);
        glDrawArrays(GL_LINES, 0, _urLines.size() * 4);
        _uLinesVao.release();
    }

    if(_oQuadsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        _oQuadsVao.bind();
        glDrawArrays(GL_QUADS, 0, _orLines.size() * 4);
        _oQuadsVao.release();
    }

    if(_oLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        _oLinesVao.bind();
        glLineWidth(6.0f);
        glDrawArrays(GL_LINES, 0, _orLines.size() * 4);
        _oLinesVao.release();
    }


    if(_cLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _cLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _cLines.size() * 2);
        _cLinesVao.release();
    }



    /*
    if(_rLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(3.0f);
        _rLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _rLines.size() * 2);
        _rLinesVao.release();
    }

    if(_lLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(3.0f);
        _lLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _lLines.size() * 2);
        _lLinesVao.release();
    }
    */
}

double TilePainter::AngleInBetween(AVector vec1, AVector vec2)
{
    return acos(vec1.Dot(vec2) / (vec1.Length() * vec2.Length()));
}

void TilePainter::GetSegmentPoints(ALine curLine, ALine prevLine, ALine nextLine, double t0, double t1, AVector* pA, AVector* pB, AVector* pC, AVector* pD)
{
    // point
    AVector p0 = curLine.GetPointA();
    AVector p1 = curLine.GetPointB();

    // direction
    AVector prevDirNorm  = prevLine.Direction().Norm();
    AVector curDirNorm   = curLine.Direction().Norm();
    AVector nextDirNorm  = nextLine.Direction().Norm();

    AVector d0;
    AVector d1;

    if(!prevLine.Invalid() && !nextLine.Invalid())		// normal drawing
    {
        d0 = (prevDirNorm + curDirNorm) / 2.0;
        d1 = (nextDirNorm + curDirNorm) / 2.0;
    }
    else if(prevLine.Invalid() && nextLine.Invalid())	// line consists of only one line segment
    {
        d0 = curDirNorm;
        d1 = curDirNorm;
    }
    else if(prevLine.Invalid())							// draw starting segment
    {
        d0 = curDirNorm;
        d1 = (nextDirNorm + curDirNorm) / 2.0;
    }
    else if(nextLine.Invalid())							// draw ending segment
    {
        d0 = (prevDirNorm + curDirNorm) / 2.0;
        d1 = curDirNorm;
    }

    // thickness
    float p0HalfThickness = t0;
    float p1HalfThickness = t1;

    if(p0HalfThickness <= 0.0) p0HalfThickness = 0.5;
    if(p1HalfThickness <= 0.0) p1HalfThickness = 0.5;

    d0 = d0.Norm();
    d1 = d1.Norm();

    d0 *= p0HalfThickness;
    d1 *= p1HalfThickness;

    AVector d0Left (-d0.y,  d0.x);
    AVector d0Right( d0.y, -d0.x);
    AVector d1Left (-d1.y,  d1.x);
    AVector d1Right( d1.y, -d1.x);

    *pA = p0 + d0Left;
    *pB = p0 + d0Right;
    *pC = p1 + d1Left;
    *pD = p1 + d1Right;
}

double TilePainter::DistanceToFiniteLine(AVector v, AVector w, AVector p)
{
    float eps_val = std::numeric_limits<float>::epsilon() * 1000;
    double l2 = v.DistanceSquared(w);
    if (l2 > -eps_val && l2 < eps_val) return p.Distance(v);
    double t = (p - v).Dot(w - v) / l2;
    if (t < 0.0)	  { return  p.Distance(v); }
    else if (t > 1.0) { return  p.Distance(w); }
    AVector projection = v + (w - v) * t;
    return p.Distance(projection);
}
