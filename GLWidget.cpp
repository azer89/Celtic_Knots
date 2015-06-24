
#include "GLWidget.h"

#include <iostream>
#include <random>
#include <math.h>

#include <QGLFormat>
#include <QSvgGenerator>

#include "VertexData.h"

GLWidget::GLWidget(QGLFormat format, QWidget *parent) :
    QGLWidget(format, parent),
    _isMouseDown(false),
    _zoomFactor(10.0),
    _shaderProgram(0),
    _img_width(50),
    _img_height(50),
    _shouldUpdateScrolls(false)
{
}

GLWidget::~GLWidget()
{
    if(_shaderProgram) delete _shaderProgram;
}

void GLWidget::initializeGL()
{
    QGLFormat glFormat = QGLWidget::format();
    if (!glFormat.sampleBuffers()) { std::cerr << "Could not enable sample buffers." << std::endl; return; }

    glShadeModel(GL_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glEnable(GL_DEPTH_TEST);

    _shaderProgram = new QOpenGLShaderProgram();
    if (!_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../OrganicLabyrinth/shader.vert"))
        { std::cerr << "Cannot load vertex shader." << std::endl; return; }

    if (!_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../OrganicLabyrinth/shader.frag"))
        { std::cerr << "Cannot load fragment shader." << std::endl; return; }

    if ( !_shaderProgram->link() )
        { std::cerr << "Cannot link shaders." << std::endl; return; }

    _shaderProgram->bind();
    _mvpMatrixLocation = _shaderProgram->uniformLocation("mvpMatrix");
    _colorLocation = _shaderProgram->attributeLocation("vertexColor");
    _vertexLocation = _shaderProgram->attributeLocation("vert");
    _use_color_location = _shaderProgram->uniformLocation("use_color");

    //InitCurve();
    //CreateCurveVAO();

    InitCells();
    InitDots();
}

bool GLWidget::event( QEvent * event )
{
    return QGLWidget::event(event);
}


// This is an override function from Qt but I can't find its purpose
void GLWidget::resizeGL(int width, int height)
{
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, this->width(),  this->height());

    int current_width = width();
    int current_height = height();

    // Set orthographic Matrix
    QMatrix4x4 orthoMatrix;

    orthoMatrix.ortho(0.0 +  _scrollOffset.x(),
                      (float)current_width +  _scrollOffset.x(),
                      (float)current_height + _scrollOffset.y(),
                      0.0 + _scrollOffset.y(),
                      -100, 100);

    // Translate the view to the middle
    QMatrix4x4 transformMatrix;
    transformMatrix.setToIdentity();
    transformMatrix.scale(_zoomFactor);

    _shaderProgram->setUniformValue(_mvpMatrixLocation, orthoMatrix * transformMatrix);


    if(_breakScribbleVao.isCreated() && _isMouseDown)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _breakScribbleVao.bind();
        glDrawArrays(GL_LINES, 0, 2);
        _breakScribbleVao.release();
    }

    if(_dotsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        int verticesPerDot = 4 + 2;

        _dotsVao.bind();

        uint nDots = _gridSize.width() * _gridSize.height();
        nDots += (_gridSize.width() + 1) * (_gridSize.height() + 1);

        for(uint a = 0; a < nDots; a++)
        {
            glDrawArrays(GL_TRIANGLE_FAN, a * verticesPerDot, verticesPerDot);
        }
        _dotsVao.release();
    }


    if(_cellLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(0.5f);
        _cellLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _cellLines.size() * 2);
        _cellLinesVao.release();
    }
}

// Mouse is pressed
void GLWidget::mousePressEvent(int x, int y)
{
    _isMouseDown = true;

    double dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    double dy = y + _scrollOffset.y();
    dy /= _zoomFactor;


    // break markers
    AnIndex idx = GetIndex(AVector(dx, dy));

    if(_cells[idx.x][idx.y]._cellSign != CellSign::SIGN_EMPTY)
    {
        AVector gVec(idx.x * _gridSpacing, idx.y * _gridSpacing);

        _breakScribbleLine.XA = gVec.x;
        _breakScribbleLine.YA = gVec.y;
        _breakScribbleLine.XB = gVec.x;
        _breakScribbleLine.YB = gVec.y;

        _startIndex.x = idx.x;
        _startIndex.y = idx.y;
        _endIndex.x = idx.x;
        _endIndex.y = idx.y;

        if(_breakScribbleVao.isCreated())
            { _breakScribbleVao.destroy(); }
    }
    // update canvas
    this->repaint();
}

// Mouse is moved
void GLWidget::mouseMoveEvent(int x, int y)
{
    double dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    double dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    // break markers
    if(_isMouseDown)
    {
        AnIndex idx = GetIndex(AVector(dx, dy));
        AVector gVec(idx.x * _gridSpacing, idx.y * _gridSpacing);

        if( _cells[idx.x][idx.y]._cellSign != CellSign::SIGN_EMPTY && (_startIndex.x == idx.x || _startIndex.y == idx.y))
        {
            _breakScribbleLine.XB = gVec.x;
            _breakScribbleLine.YB = gVec.y;

            _endIndex.x = idx.x;
            _endIndex.y = idx.y;

            std::vector<ALine> linev;
            linev.push_back(_breakScribbleLine);
            PrepareLinesVAO(linev, &_breakScribbleVbo, &_breakScribbleVao, QVector3D(0.0, 0.0, 0.0));
        }
    }


    // update canvas
    this->repaint();
}


// Mouse is released
void GLWidget::mouseReleaseEvent(int x, int y)
{
    _isMouseDown = false;
    double dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    double dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    // your stuff

    this->repaint();
}

void GLWidget::mouseDoubleClick(int x, int y)
{
    double dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    double dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    // your stuff

    this->repaint();
}


void GLWidget::HorizontalScroll(int val) { _scrollOffset.setX(val); }
void GLWidget::VerticalScroll(int val) { _scrollOffset.setY(val); }
void GLWidget::ZoomIn() { this->_zoomFactor += 0.05f; }
void GLWidget::ZoomOut() { this->_zoomFactor -= 0.05f; if(this->_zoomFactor < 0.1f) _zoomFactor = 0.1f; }

AnIndex GLWidget::GetIndex(AVector vec)
{
    float floatx = vec.x / _gridSpacing;
    float floaty = vec.y / _gridSpacing;

    double intpartx, intparty;
    intpartx = round (floatx);
    intparty = round (floaty);

    return AnIndex(intpartx, intparty);
}

void GLWidget::InitCells()
{
    _cells.clear();
    _gridSpacing = 10;
    _gridSize = QSize(10, 5);

    // add one row and one column
    _actualGridSize = QSize((_gridSize.width() - 1) * 2 + 1, (_gridSize.height() - 1) * 2 + 1 );

    _img_width  = (_actualGridSize.width() - 1) * _gridSpacing;
    _img_height = (_actualGridSize.height() - 1) * _gridSpacing;

    for(uint a = 0; a < _actualGridSize.width(); a++)
    {
        _cells.push_back(std::vector<CCell>(_actualGridSize.height()));
    }


    // ONE
    for(size_t a = 0; a < _actualGridSize.width(); a += 2)
    {
        for(size_t b = 0; b < _actualGridSize.height(); b += 2)
        {
            _cells[a][b]._cellSign = CellSign::SIGN_ONE;
        }
    }

    // TWO
    for(size_t a = 1; a < _actualGridSize.width(); a += 2)
    {
        for(size_t b = 1; b < _actualGridSize.height(); b += 2)
        {
            _cells[a][b]._cellSign = CellSign::SIGN_TWO;
        }
    }

    // vao
    _cellLines.clear();
    for(uint a = 0; a < _actualGridSize.width() - 1; a++)
    {
        for(uint b = 0; b < _actualGridSize.height() - 1; b++)
        {
            AVector upLeftPt(a * _gridSpacing, b * _gridSpacing);
            AVector upRightPt((a + 1) * _gridSpacing, b * _gridSpacing);
            AVector bottomLeftPt(a * _gridSpacing, (b + 1) * _gridSpacing);
            AVector bottomRightPt((a + 1) * _gridSpacing, (b + 1) * _gridSpacing);

            _cellLines.push_back(ALine(upLeftPt, upRightPt));
            _cellLines.push_back(ALine(upLeftPt, bottomLeftPt));

            if(a == _actualGridSize.width() - 2)
                { _cellLines.push_back(ALine(upRightPt, bottomRightPt)); }

            if(b == _actualGridSize.height() - 2)
                { _cellLines.push_back(ALine(bottomLeftPt, bottomRightPt)); }
        }
    }
    PrepareLinesVAO(_cellLines, &_cellLinesVbo, &_cellLinesVao, QVector3D(0.0, 0.0, 0.0));

    _shouldUpdateScrolls = true;
}

void GLWidget::InitDots()
{
    if(_dotsVao.isCreated())
    {
        _dotsVao.destroy();
    }
    _dotsVao.create();
    _dotsVao.bind();

    float radius = 1.0f;

    QVector<VertexData> vertices;

    // ONE
    for(uint a = 0; a < _actualGridSize.width(); a += 2)
    {
        for(uint b = 0; b < _actualGridSize.height(); b += 2)
        {
            int xCenter = a * _gridSpacing;
            int yCenter = b * _gridSpacing;
            QVector3D vecCol = QVector3D(1, 0, 0);

            vertices.append(VertexData(QVector3D(xCenter, yCenter,  0.0f), QVector2D(), vecCol));
            float addValue = (M_PI * 2.0 / 4);
            for(float a = 0.0; a < M_PI * 2.0; a += addValue)
            {
                float xPt = xCenter + radius * sin(a);
                float yPt = yCenter + radius * cos(a);
                vertices.append(VertexData(QVector3D(xPt, yPt,  0.0f), QVector2D(), vecCol));
            }
            float xPt = xCenter + radius * sin(M_PI * 2.0);
            float yPt = yCenter + radius * cos(M_PI * 2.0);
            vertices.append(VertexData(QVector3D(xPt, yPt,  0.0f), QVector2D(), vecCol));

        }
    }

    // TWO
    for(uint a = 1; a < _actualGridSize.width(); a += 2)
    {
        for(uint b = 1; b < _actualGridSize.height(); b += 2)
        {
            int xCenter = a * _gridSpacing;
            int yCenter = b * _gridSpacing;
            QVector3D vecCol = QVector3D(0, 0, 1);

            vertices.append(VertexData(QVector3D(xCenter, yCenter,  0.0f), QVector2D(), vecCol));
            float addValue = (M_PI * 2.0 / 4);
            for(float a = 0.0; a < M_PI * 2.0; a += addValue)
            {
                float xPt = xCenter + radius * sin(a);
                float yPt = yCenter + radius * cos(a);
                vertices.append(VertexData(QVector3D(xPt, yPt,  0.0f), QVector2D(), vecCol));
            }
            float xPt = xCenter + radius * sin(M_PI * 2.0);
            float yPt = yCenter + radius * cos(M_PI * 2.0);
            vertices.append(VertexData(QVector3D(xPt, yPt,  0.0f), QVector2D(), vecCol));

        }
    }

    _dotsVbo.create();
    _dotsVbo.bind();
    _dotsVbo.allocate(vertices.data(), vertices.size() * sizeof(VertexData));

    // reuse the variable
    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    _dotsVao.release();

}

void GLWidget::InitCurve()
{
    _points.clear();
    AVector centerPt(this->_img_width / 2, this->_img_height / 2);

    float addValue = (M_PI * 2.0 / (float)16);
    for(float a = 0.0; a < M_PI * 2.0; a += addValue)
    {
        float xPt = centerPt.x + 10 * sin(a);
        float yPt = centerPt.y + 10 * cos(a);
        _points.push_back(AVector(xPt, yPt));
    }
}

void GLWidget::CreateCurveVAO()
{
    // POINTS VAO
    QVector3D vecCol = QVector3D(1.0, 0.0, 0.0);
    PreparePointsVAO(_points, &_pointsVbo, &_pointsVao, vecCol);

    // LINES VAO
    vecCol = QVector3D(0.0, 0.5, 1.0);
    std::vector<ALine> lines;
    for(uint a = 0; a < _points.size(); a++)
    {
        if(a < _points.size() - 1) { lines.push_back(ALine(_points[a], _points[a + 1])); }
        else { lines.push_back(ALine(_points[a], _points[0])); }
    }
    PrepareLinesVAO(lines, &_linesVbo, &_linesVao, vecCol);
}

void GLWidget::PreparePointsVAO(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol)
{
    if(ptsVao->isCreated())
    {
        ptsVao->destroy();
    }

    ptsVao->create();
    ptsVao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < points.size(); a++)
    {
        data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol));
    }

    ptsVbo->create();
    ptsVbo->bind();
    ptsVbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    ptsVao->release();
}

void GLWidget::PrepareLinesVAO(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
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

void GLWidget::SaveToSvg()
{
}

void GLWidget::PaintCurve()
{
    if(_points.size() == 0) { return; }

    _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

    glPointSize(5.0f);
    _pointsVao.bind();
    glDrawArrays(GL_POINTS, 0, _points.size());
    _pointsVao.release();

    glLineWidth(2.0f);
    _linesVao.bind();
    glDrawArrays(GL_LINES, 0, _points.size() * 2);
    _linesVao.release();
}


