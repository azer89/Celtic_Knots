#ifndef TILEPAINTER_H
#define TILEPAINTER_H

#include "CCell.h"
#include "AnIndex.h"
#include "DirectionType.h"
#include "ALine.h"
#include "AVector.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <vector>

class TilePainter
{
public:
    TilePainter();
    ~TilePainter();

    void SetTiles(std::vector<std::vector<CCell>> cells, float gridSpacing);
    void DrawTiles();

private:

    AVector RotatePoint(float cx,float cy,float angle, AVector p);

    void CreateCorner(CCell cell, AnIndex idx, float gridSpacing);
    void CreateCross(CCell cell, AnIndex idx, float gridSpacing);
    void CreateSlash(CCell cell, AnIndex idx, float gridSpacing);
    void CreateStraight(CCell cell, AnIndex idx, float gridSpacing);

private:
    QOpenGLBuffer               _vbo;
    QOpenGLVertexArrayObject    _vao;

    std::vector<ALine>          _lines;

};

#endif // TILEPAINTER_H
