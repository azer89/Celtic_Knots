
#include "TilePainter.h"
#include "VertexData.h"
#include "CurveInterpolation.h"
#include "LayerType.h"

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
    AnIndex curIdx = traceList[i];
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
        {
            //std::cout << "s ";
            return CornerCase::COR_START;
        }
        else if(i == 1 && changeDir) // end ?
        {
            //std::cout << "e ";
            return CornerCase::COR_END;
        }
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

        // middle
        if(curC._directionType != prevC._directionType && curC._directionType != nextC._directionType)
        {
            //std::cout << "1 ";
            return CornerCase::COR_MIDDLE;
        }

        // start
        else if(curC._directionType == prevC._directionType && curC._directionType != nextC._directionType)
        {
            //std::cout << "2 " << " (" << prevI << "," << curI << "," << nextI << ") ";
            return CornerCase::COR_START;
        }

        // end
        else if(curC._directionType != prevC._directionType && curC._directionType == nextC._directionType)
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

void TilePainter::SetTiles(std::vector<std::vector<CCell>> cells, std::vector<AnIndex> traceList, float gridSpacing, bool isTracingDone)
{
    _cLines.clear();
    std::vector<std::pair<LayerType, LayerType>> layerTypeList1;
    std::vector<std::pair<LayerType, LayerType>> layerTypeList2;
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
        ALine ln;
        if (curCel._directionType == DirectionType::DIR_UPRIGHT)
        {
            ln = ALine(curIdx.x * gridSpacing, (curIdx.y + 1) * gridSpacing,
                      (curIdx.x + 1) * gridSpacing, curIdx.y * gridSpacing);
        }
        else if (curCel._directionType == DirectionType::DIR_DOWNRIGHT)
        {
            ln = ALine(curIdx.x * gridSpacing, curIdx.y * gridSpacing,
                      (curIdx.x + 1) * gridSpacing, (curIdx.y + 1) * gridSpacing);
        }
        else if (curCel._directionType == DirectionType::DIR_DOWNLEFT)
        {
            ln = ALine((curIdx.x + 1) * gridSpacing, curIdx.y * gridSpacing,
                       curIdx.x * gridSpacing, (curIdx.y + 1) * gridSpacing);
        }
        else if (curCel._directionType == DirectionType::DIR_UPLEFT)
        {
            ln = ALine((curIdx.x + 1) * gridSpacing, (curIdx.y + 1) * gridSpacing,
                        curIdx.x * gridSpacing, curIdx.y * gridSpacing);
        }

        // determine layer types (start end)
        std::pair<LayerType, LayerType> lTypes;
        if (curCel._directionType == DirectionType::DIR_UPRIGHT)
        {
            if(IsEven(curIdx.y) && IsEven(curIdx.y))
            {
                // UO
                lTypes.first = LayerType::LAYER_UNDER;
                lTypes.second = LayerType::LAYER_OVER;
            }
            else /*if(!IsEven(curIdx.x) && !IsEven(curIdx.y))*/
            {
                // OU
                lTypes.first = LayerType::LAYER_OVER;
                lTypes.second = LayerType::LAYER_UNDER;
            }
        }
        else if (curCel._directionType == DirectionType::DIR_DOWNRIGHT)
        {
            if(!IsEven(curIdx.x) && IsEven(curIdx.y))
            {
                // UO
                lTypes.first = LayerType::LAYER_UNDER;
                lTypes.second = LayerType::LAYER_OVER;
            }
            else /*if(IsEven(curIdx.x) && !IsEven(curIdx.y))*/
            {
                // OU
                lTypes.first = LayerType::LAYER_OVER;
                lTypes.second = LayerType::LAYER_UNDER;
            }
        }
        else if (curCel._directionType == DirectionType::DIR_DOWNLEFT)
        {
            if(IsEven(curIdx.y) && IsEven(curIdx.y))
            {
                // OU
                lTypes.first = LayerType::LAYER_OVER;
                lTypes.second = LayerType::LAYER_UNDER;
            }
            else /*if(!IsEven(curIdx.x) && !IsEven(curIdx.y))*/
            {
                // UO
                lTypes.first = LayerType::LAYER_UNDER;
                lTypes.second = LayerType::LAYER_OVER;
            }
        }
        else if (curCel._directionType == DirectionType::DIR_UPLEFT)
        {
            if(!IsEven(curIdx.x) && IsEven(curIdx.y))
            {
                // OU
                lTypes.first = LayerType::LAYER_OVER;
                lTypes.second = LayerType::LAYER_UNDER;
            }
            else /*if(IsEven(curIdx.x) && !IsEven(curIdx.y))*/
            {
                // UO
                lTypes.first = LayerType::LAYER_UNDER;
                lTypes.second = LayerType::LAYER_OVER;
            }
        }
        layerTypeList1.push_back(lTypes);  //lTypes
        _cLines.push_back(ln);
    }


    // refine lines
    std::vector<ALine> tempLines1(_cLines);
    for(size_t a = 0; a < _cLines.size() && _cLines.size() > 1; a++)
    {
        size_t curI = a;
        size_t prevI = a - 1;
        size_t nextI = a + 1;
        if(curI == 0)
        {
            prevI = _cLines.size() - 1;
        }
        else if(curI == _cLines.size() - 1)
        {
            nextI = 0;
        }

        AVector offsetVec1 = GetMiddlePoint(tempLines1[prevI].GetPointA(), tempLines1[curI].GetPointA(), tempLines1[curI].GetPointB());
        AVector offsetVec2 = GetMiddlePoint(tempLines1[curI].GetPointA(), tempLines1[curI].GetPointB(), tempLines1[nextI].GetPointB());

        offsetVec1 *= 0.5;
        offsetVec2 *= 0.5;

        if(ccs[a] == CornerCase::COR_START)
        {
            _cLines[curI].XB += offsetVec2.x;
            _cLines[curI].YB += offsetVec2.y;
        }
        else if(ccs[a] == CornerCase::COR_END)
        {
            _cLines[curI].XA += offsetVec1.x;
            _cLines[curI].YA += offsetVec1.y;
        }
        else if(ccs[a] == CornerCase::COR_MIDDLE)
        {
            _cLines[curI].XA += offsetVec1.x;
            _cLines[curI].YA += offsetVec1.y;
            _cLines[curI].XB += offsetVec2.x;
            _cLines[curI].YB += offsetVec2.y;
        }

        if(curI == _cLines.size() - 1 && !isTracingDone)
        {
            _cLines[curI].XB = tempLines1[curI].XB;
            _cLines[curI].YB = tempLines1[curI].YB;
        }

        if(curI == 0 && !isTracingDone)
        {
            _cLines[curI].XA = tempLines1[curI].XA;
            _cLines[curI].YA = tempLines1[curI].YA;
        }
    }



    // divide lines
    /*
    std::vector<ALine> tempLines2;
    for(size_t a = 0; a < _cLines.size(); a++)
    {
        ALine ln = _cLines[a];
        AVector sPt = ln.GetPointA();
        AVector ePt = ln.GetPointB();
        AVector mPt = sPt + (ePt - sPt) * 0.5;
        tempLines2.push_back(ALine(sPt, mPt));
        tempLines2.push_back(ALine(mPt, ePt));
    }
    _cLines = std::vector<ALine>(tempLines2);
    */


    // bezier curves
    std::vector<LayerType> plTypeList;
    if(isTracingDone)
    {
        _points.clear();
        for(size_t a = 0; a < _cLines.size(); a++)
        {
            int curIdx = a;
            int prevIdx = a - 1;
            int nextIdx = a + 1;

            if(curIdx == 0) { prevIdx = _cLines.size() - 1; }
            else if(curIdx == _cLines.size() - 1) { nextIdx = 0; }

            AVector pt1 = _cLines[prevIdx].GetPointA();
            AVector pt2 = _cLines[curIdx].GetPointA();
            AVector pt3 = _cLines[curIdx].GetPointB();
            AVector pt4 = _cLines[nextIdx].GetPointB();

            AVector anchor1;
            AVector anchor2;
            CurveInterpolation::GetAnchors(pt1, pt2, pt3, pt4, anchor1, anchor2, 0.75);

            float angle1 = AngleInBetween(anchor1 - pt2, pt3 - pt2);
            float angle2 = AngleInBetween(anchor2 - pt3, pt2 - pt3);

            //std::cout << angle1 << "  -  " << angle2 << "\n";

            // layer types
            std::pair<LayerType, LayerType> lTypes = layerTypeList1[curIdx];

            if(angle1 <= 0.13) { anchor1 = pt2 + (pt3 - pt2).Norm() * 1.0; }
            if(angle2 <= 0.13) { anchor2 = pt3 + (pt2 - pt3).Norm() * 1.0; }

            std::vector<AVector> segmentPoints;
            CurveInterpolation::DeCasteljau(segmentPoints, pt2, anchor1, anchor2, pt3, 1.0f);

            for(size_t a = 0; a < segmentPoints.size() / 2; a++)
                { plTypeList.push_back(lTypes.first); }
            for(size_t a = segmentPoints.size() / 2; a < segmentPoints.size(); a++)
                { plTypeList.push_back(lTypes.second); }

            _points.insert(_points.end(), segmentPoints.begin(), segmentPoints.end());
        }

        _cLines.clear();
        for(size_t a = 0; a < _points.size(); a++)
        {
            int idx1 = a;
            int idx2 = a + 1;
            if(idx1 == _points.size() - 1) { idx2 = 0; }

            _cLines.push_back(ALine(_points[idx1].x, _points[idx1].y, _points[idx2].x, _points[idx2].y));
        }
    }

    // move first to the end
    // std::vector<std::pair<LayerType, LayerType>>
    layerTypeList2 =


    PrepareLinesVAO(_cLines, &_cLinesVbo, &_cLinesVao, QVector3D(1.0, 0.0, 0.0));

    // left and right
    _rLines.clear();
    _lLines.clear();
    for(size_t a = 0; a < _cLines.size(); a++)
    {
        int curIdx = a;
        int prevIdx = a - 1;
        int nextIdx = a + 1;

        if(curIdx == 0) { prevIdx = _cLines.size() - 1; }
        else if(curIdx == _cLines.size() - 1) { nextIdx = 0; }

        ALine prevLine = _cLines[prevIdx];
        ALine curLine = _cLines[curIdx];
        ALine nextLine = _cLines[nextIdx];

        AVector d0Left;
        AVector d0Right;
        AVector d1Left;
        AVector d1Right;

        GetSegmentPoints(curLine, prevLine, nextLine, 2, 2, &d0Left, &d0Right, &d1Left, &d1Right);

        _lLines.push_back(ALine(d0Left, d1Left));
        _rLines.push_back(ALine(d0Right, d1Right));
    }

    PrepareLinesVAO(_lLines, &_lLinesVbo, &_lLinesVao, QVector3D(1.0, 0.0, 0.0));
    PrepareLinesVAO(_rLines, &_rLinesVbo, &_rLinesVao, QVector3D(1.0, 0.0, 0.0));
}

void TilePainter::PrepareLinesVAO(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
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

void TilePainter::DrawTiles()
{

    if(_cLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _cLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _cLines.size() * 2);
        _cLinesVao.release();
    }

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

//void TilePainter::CreateCorner(CCell cell, AnIndex idx, float gridSpacing)
//{
//}

//void TilePainter::CreateCross(CCell cell, AnIndex idx, float gridSpacing)
//{
//}

//void TilePainter::CreateSlash(CCell cell, AnIndex idx, float gridSpacing)
//{
//}

//void TilePainter::CreateStraight(CCell cell, AnIndex idx, float gridSpacing)
//{
//}

/*
AVector TilePainter::RotatePoint(float cx,float cy,float angle, AVector p)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    p.x -= cx;
    p.y -= cy;

    // rotate point
    float xnew = p.x * c - p.y * s;
    float ynew = p.x * s + p.y * c;

    // translate point back:
    p.x = xnew + cx;
    p.y = ynew + cy;
    return p;
}
*/
