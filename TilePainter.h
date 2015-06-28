#ifndef TILEPAINTER_H
#define TILEPAINTER_H

#include "CCell.h"
#include "AnIndex.h"
#include "DirectionType.h"
#include "ALine.h"
#include "AVector.h"
#include "LayerType.h"
#include "RibbonSegment.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <vector>

enum CornerCase
{
    COR_NORMAL = 0,
    COR_START = 1,
    COR_MIDDLE = 2,
    COR_END = 3,
};

class TilePainter
{
public:
    TilePainter();
    ~TilePainter();

    void SetTiles(std::vector<std::vector<CCell>> cells, std::vector<AnIndex> traceList, float gridSpacing, bool isTracingDone);
    void DrawTiles();

public:
    QOpenGLShaderProgram* _shaderProgram;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;

private:

    bool IsEven(int num);
    ALine GetLineInACell(CCell curCel, float gridSpacing, AnIndex curIdx);
    AVector GetMiddlePoint(AVector a, AVector b, AVector c);
    CornerCase GetCornerCase(int i, std::vector<std::vector<CCell>> cells, std::vector<AnIndex> traceList, bool isTracingDone);

    std::pair<LayerType, LayerType> GetLayerTypes(CCell curCel, AnIndex curIdx);

    void GeTwoSegments(AVector p0, AVector p1, AVector p2, AVector p3, RibbonSegment* segment1, RibbonSegment* segment2);
    void CalculateRibbonLR(RibbonSegment* segment);
    void CalculateOverUnderRibbon(std::vector<ALine> cLines, std::vector<LayerType> layerTypeList);

    int PrepareQuadsVAO1(std::vector<RibbonSegment> ribbonSegments, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);
    void PrepareQuadsVAO2(std::vector<ALine> rLines, std::vector<ALine> lLines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);

    void PrepareLinesVAO1(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void PrepareLinesVAO2(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, std::vector<LayerType> layerTypeList);

    double AngleInBetween(AVector vec1, AVector vec2);
    void GetSegmentPoints(ALine curLine, ALine prevLine, ALine nextLine, double t0, double t1, AVector* pA, AVector* pB, AVector* pC, AVector* pD);
    //AVector RotatePoint(float cx,float cy,float angle, AVector p);

    //void CreateCorner(CCell cell, AnIndex idx, float gridSpacing);
    //void CreateCross(CCell cell, AnIndex idx, float gridSpacing);
    //void CreateSlash(CCell cell, AnIndex idx, float gridSpacing);
    //void CreateStraight(CCell cell, AnIndex idx, float gridSpacing);

private:

    // over under
    std::vector<RibbonSegment>  _urSegments;    // under
    std::vector<RibbonSegment>  _orSegments;    // over

    // under, left and right
    std::vector<ALine>          _uLines;
    QOpenGLBuffer               _uLinesVbo;
    QOpenGLVertexArrayObject    _uLinesVao;

    // over, left and right
    std::vector<ALine>          _oLines;
    QOpenGLBuffer               _oLinesVbo;
    QOpenGLVertexArrayObject    _oLinesVao;

    //std::vector<ALine>          _rLines;
    //std::vector<ALine>          _lLines;
    //std::vector<ALine>          _uRLines;
    //std::vector<ALine>          _uLLines;
    std::vector<ALine> _urLines;
    std::vector<ALine> _ulLines;
    std::vector<ALine> _orLines;
    std::vector<ALine> _olLines;

    // under segments
    //int _uQuadsSize;
    QOpenGLBuffer               _uQuadsVbo;
    QOpenGLVertexArrayObject    _uQuadsVao;

    // over segments
    //int _oQuadsSize;
    QOpenGLBuffer               _oQuadsVbo;
    QOpenGLVertexArrayObject    _oQuadsVao;

    QOpenGLBuffer               _cLinesVbo;
    QOpenGLVertexArrayObject    _cLinesVao;
    std::vector<ALine>          _cLines;

    //QOpenGLBuffer               _rLinesVbo;
    //QOpenGLVertexArrayObject    _rLinesVao;
    //std::vector<ALine>          _rLines;

    //QOpenGLBuffer               _lLinesVbo;
    //QOpenGLVertexArrayObject    _lLinesVao;
    //std::vector<ALine>          _lLines;

    QOpenGLBuffer               _debugPointsVbo;
    QOpenGLVertexArrayObject    _debugPointsVao;
    std::vector<AVector>        _debugPoints;

    std::vector<AVector>        _points;

};

#endif // TILEPAINTER_H
