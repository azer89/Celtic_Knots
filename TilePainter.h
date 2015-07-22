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
    COR_START_STRAIGHT = 4,
    COR_END_STRAIGHT = 5,
    COR_MIDDLE_STRAIGHT = 6,
    COR_STRAIGHT = 7,
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

    void RefineLines(std::vector<ALine>& lines, std::vector<CornerCase> ccs, bool isTracingDone);

    std::pair<LayerType, LayerType> GetLayerTypes(CCell curCel, AnIndex curIdx);

    void GeTwoSegments(AVector p0, AVector p1, AVector p2, AVector p3, RibbonSegment* segment1, RibbonSegment* segment2);
    void CalculateRibbonLR(RibbonSegment* segment);
    void CalculateOverUnderRibbon(std::vector<ALine> cLines, std::vector<LayerType> layerTypeList);

    int  PrepareQuadsVBO1(std::vector<RibbonSegment> ribbonSegments, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);
    void PrepareQuadsVBO2(std::vector<ALine> rLines, std::vector<ALine> lLines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);
    void PrepareLinesVBO1(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void PrepareLinesVBO2(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, std::vector<LayerType> layerTypeList);
    void PrepareLinesVBO3(std::vector<ALine> rLines, std::vector<ALine> lLines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);

    double AngleInBetween(AVector vec1, AVector vec2);
    void GetSegmentPoints(ALine curLine, ALine prevLine, ALine nextLine, double t0, double t1, AVector* pA, AVector* pB, AVector* pC, AVector* pD);

    double DistanceToFiniteLine(AVector v, AVector w, AVector p);

private:

    // over under
    std::vector<RibbonSegment>  _urSegments;    // under
    std::vector<RibbonSegment>  _orSegments;    // over

    std::vector<ALine> _urLines;
    std::vector<ALine> _ulLines;
    std::vector<ALine> _orLines;
    std::vector<ALine> _olLines;

    QOpenGLBuffer               _uQuadsVbo;
    QOpenGLVertexArrayObject    _uQuadsVao;

    QOpenGLBuffer               _oQuadsVbo;
    QOpenGLVertexArrayObject    _oQuadsVao;

    QOpenGLBuffer               _cLinesVbo;
    QOpenGLVertexArrayObject    _cLinesVao;
    std::vector<ALine>          _cLines;

    QOpenGLBuffer               _anchorLinesVbo;
    QOpenGLVertexArrayObject    _anchorLinesVao;
    std::vector<ALine>          _anchorLines;

    QOpenGLBuffer               _uLinesVbo;
    QOpenGLVertexArrayObject    _uLinesVao;

    QOpenGLBuffer               _oLinesVbo;
    QOpenGLVertexArrayObject    _oLinesVao;

    QOpenGLBuffer               _debugPointsVbo;
    QOpenGLVertexArrayObject    _debugPointsVao;
    std::vector<AVector>        _debugPoints;

    std::vector<AVector>        _points;

};

#endif // TILEPAINTER_H
