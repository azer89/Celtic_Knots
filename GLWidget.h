#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QMatrix4x4>
#include <QtOpenGL/QGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "AVector.h"
#include "ALine.h"
#include "CCell.h"
#include "AnIndex.h"
#include "TilePainter.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

private:
    std::vector<std::vector<CCell>> _cells;
    QSize _gridSize;
    QSize _actualGridSize;
    float _gridSpacing;

    // cell lines
    std::vector<ALine>          _cellLines;
    QOpenGLBuffer               _cellLinesVbo;
    QOpenGLVertexArrayObject    _cellLinesVao;

    QOpenGLBuffer               _dotsVbo;
    QOpenGLVertexArrayObject    _dotsVao;

    // draw cells
    TilePainter* _tilePainter;


    // draw
    AnIndex                     _drawStartIndex;
    AnIndex                     _drawEndIndex;
    ALine                       _drawBreakLine;
    QOpenGLBuffer               _drawBreakVbo;
    QOpenGLVertexArrayObject    _drawBreakVao;

    std::vector<ALine>          _breakLines;
    QOpenGLBuffer               _breakLinesVbo;
    QOpenGLVertexArrayObject    _breakLinesVao;

    // trace cells
    std::vector<AnIndex>        _traceList;
    bool _isTracingDone;

    bool    _isMouseDown;
    float   _zoomFactor;
    QPoint  _scrollOffset;

    // image size
    int _img_width;
    int _img_height;

    // shader
    QOpenGLShaderProgram* _shaderProgram;

    // points
    std::vector<AVector>        _points;
    QOpenGLBuffer               _pointsVbo;
    QOpenGLVertexArrayObject    _pointsVao;

    // lines
    QOpenGLBuffer               _linesVbo;
    QOpenGLVertexArrayObject    _linesVao;

    // for rendering
    int         _mvpMatrixLocation;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;
    QMatrix4x4  _perspMatrix;
    QMatrix4x4  _transformMatrix;

    bool _shouldUpdateScrolls;

private:


    void InitCurve();        // demo
    void PaintCurve();       // demo
    void CreateCurveVAO();

    void SaveToSvg();

    void PreparePointsVAO(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
    void PrepareLinesVAO(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);

    AnIndex GetIndex(AVector vec);
    bool DoesHitAWall(AnIndex idx);
    bool IsACorner(AnIndex idx);
    bool IsValid(AnIndex idx);

protected:
    // qt event
    bool event( QEvent * event );
    // init opengl
    void initializeGL();
    // draw
    void paintGL();

    void resizeGL(int width, int height);

public:

    // constructor
    GLWidget( QGLFormat format, QWidget *parent = 0);
    // destructor
    ~GLWidget();

    QSize GetCanvasSize() { return QSize(_img_width, _img_height); }

    LineType GetLineIntersection(AVector pt);

    void UndoBreakMarkers();
    void InitCells();
    void InitDots();
    void ResetData();

    void GenerateAKnot();
    void TraceOneStep();
    double DistanceToFiniteLine(AVector v, AVector w, AVector p);

    // zoom in handle
    void ZoomIn();
    // zoom out handle
    void ZoomOut();
    // set zoom value
    void SetZoom(int val){this->_zoomFactor = val;}
    // get zoom value
    float GetZoomFactor() { return this->_zoomFactor; }

    // set horizontal scroll position
    void HorizontalScroll(int val);
    // set vertical scroll position
    void VerticalScroll(int val);
    // get scroll position (horizontal and vertical)
    QPoint GetScrollOffset() {return this->_scrollOffset;}

    // mouse press
    void mousePressEvent(int x, int y);
    // mouse move
    void mouseMoveEvent(int x, int y);
    // mouse release
    void mouseReleaseEvent(int x, int y);
    // mouse double click
    void mouseDoubleClick(int x, int y);

    bool GetShouldUpdateScrolls() { return _shouldUpdateScrolls; }
    void SetShouldUpdateScrolls(bool val) { _shouldUpdateScrolls = val; }
};

#endif // GLWIDGET_H
