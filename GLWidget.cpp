
#include "GLWidget.h"

#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>

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
    _tilePainter(0),
    _shouldUpdateScrolls(false)
{
}

GLWidget::~GLWidget()
{
    if(_shaderProgram) delete _shaderProgram;
    if(_tilePainter) delete _tilePainter;
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

    _tilePainter = new TilePainter();
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

    if(_drawBreakVao.isCreated() && _isMouseDown)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _drawBreakVao.bind();
        glDrawArrays(GL_LINES, 0, 2);
        _drawBreakVao.release();
    }

    if(_breakLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _breakLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _breakLines.size() * 2);
        _breakLinesVao.release();
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

        _drawBreakLine.XA = gVec.x;
        _drawBreakLine.YA = gVec.y;
        _drawBreakLine.XB = gVec.x;
        _drawBreakLine.YB = gVec.y;

        _drawStartIndex.x = idx.x;
        _drawStartIndex.y = idx.y;
        _drawEndIndex.x = idx.x;
        _drawEndIndex.y = idx.y;

        if(_drawBreakVao.isCreated())
            { _drawBreakVao.destroy(); }
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

        if( _cells[idx.x][idx.y]._cellSign != CellSign::SIGN_EMPTY && (_drawStartIndex.x == idx.x || _drawStartIndex.y == idx.y))
        {
            _drawBreakLine.XB = gVec.x;
            _drawBreakLine.YB = gVec.y;

            _drawEndIndex.x = idx.x;
            _drawEndIndex.y = idx.y;

            std::vector<ALine> linev;
            linev.push_back(_drawBreakLine);
            PrepareLinesVAO(linev, &_drawBreakVbo, &_drawBreakVao, QVector3D(0.0, 0.0, 0.0));
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
    if(_drawBreakVao.isCreated())
    {
        _breakLines.push_back(_drawBreakLine);
        PrepareLinesVAO(_breakLines, &_breakLinesVbo, &_breakLinesVao, QVector3D(0.0, 0.0, 0.0));

        // mark cells
        int startx = std::min(_drawStartIndex.x, _drawEndIndex.x);
        int endx = std::max(_drawStartIndex.x, _drawEndIndex.x);
        int starty = std::min(_drawStartIndex.y, _drawEndIndex.y);
        int endy = std::max(_drawStartIndex.y, _drawEndIndex.y);

        for(size_t a = 0; a < _actualGridSize.width(); a++)
        {
            for(size_t b = 0; b < _actualGridSize.height(); b++)
            {
                if(a >= startx && a <= endx && b >= starty && b <= endy )
                    { _cells[a][b]._cellBreakMarker = CellBreakMarker::BREAK_MARKER_BREAK; }
            }
        }

        for(size_t b = 0; b < _actualGridSize.height(); b++)
        {
            for(size_t a = 0; a < _actualGridSize.width(); a++)
            {

                if(_cells[a][b]._cellBreakMarker == CellBreakMarker::BREAK_MARKER_BREAK)
                    { std::cout << "x"; }
                else
                    { std::cout << " "; }
            }
            std::cout << "\n";
        }
    }


     // update canvas
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

bool GLWidget::DoesHitAWall(AnIndex idx)
{
    CCell aCell = _cells[idx.x][idx.y];
    QSize gridDim(_actualGridSize.width() -1, _actualGridSize.height() -1);
    if(idx.x < 0 || idx.x >= gridDim.width() || idx.y < 0 || idx.y >= gridDim.height())
    {
        return true;
    }
    else if(aCell._cellBreakMarker == CellBreakMarker::BREAK_MARKER_BREAK)
    {
        return true;
    }
    return false;
}

bool GLWidget::IsACorner(AnIndex idx)
{
    AnIndex drIdx(idx.x + 1, idx.y + 1);    // down right
    AnIndex rIdx( idx.x + 1, idx.y    );     // right
    AnIndex dIdx( idx.x    , idx.y + 1);     // down

    if(DoesHitAWall(idx) && DoesHitAWall(rIdx) && DoesHitAWall(drIdx))   // up right
        { return true; }
    else if(DoesHitAWall(rIdx) && DoesHitAWall(drIdx) && DoesHitAWall(dIdx))  // downright
        { return true; }
    else if(DoesHitAWall(idx) && DoesHitAWall(dIdx) && DoesHitAWall(drIdx))  // down left
        { return true; }
    else if(DoesHitAWall(dIdx) && DoesHitAWall(idx) && DoesHitAWall(rIdx))  // up left
        { return true; }
    return false;
}

void GLWidget::TraceOneStep()
{
    //std::cout << "Trace One Step\n";
    if(_traceList.size() == 0)
    {
        AnIndex startIdx(0, 0);

        _traceList.push_back(startIdx); // put in list
        _cells[startIdx.x][startIdx.y]._isVisited = true; // mark
        //_cells[startIdx.x][startIdx.y]._tileType = TileType::TILE_CORNER;   // because (0,0)
        _cells[startIdx.x][startIdx.y]._directionType = DirectionType::DIR_DOWNRIGHT;

        _tilePainter->SetTiles(_cells, _gridSpacing);

        this->repaint();
    }
    else
    {
        DirectionType dirType = DirectionType::DIR_NONE;

        AnIndex curIdx = _traceList[_traceList.size() - 1];
        DirectionType curDir =_cells[curIdx.x][curIdx.y]._directionType;

        AnIndex urIdx(curIdx.x + 1, curIdx.y - 1);    // up right
        AnIndex drIdx(curIdx.x + 1, curIdx.y + 1);    // down right
        AnIndex dlIdx(curIdx.x - 1, curIdx.y + 1);    // down left
        AnIndex ulIdx(curIdx.x - 1, curIdx.y - 1);    // up left


        AnIndex rIdx(curIdx.x + 1, curIdx.y    );     // right
        AnIndex dIdx(curIdx.x    , curIdx.y + 1);     // down
        AnIndex lIdx(curIdx.x - 1, curIdx.y    );     // left
        AnIndex uIdx(curIdx.x    , curIdx.y - 1);     // up

        /*
        CellBreakMarker urM;
        CellBreakMarker drM;
        CellBreakMarker dlM;
        CellBreakMarker ulM;
        */

        if(_traceList.size() == 1)  // start, can go anywhere
        {
            if(!DoesHitAWall(rIdx))        // up right (r)    && no wall
            {
                // put in list
                // mark the cell
                // give direction
            }
            else if(!DoesHitAWall(drIdx))  // down right (dr) && no wall
            {
            }
            else if(!DoesHitAWall(dIdx))   // down left (d)   && no wall
            {
            }
            else if(!DoesHitAWall(curIdx)) // up left (c)     && no wall
            {
            }

            /*else if(!DoesHitAWall(rIdx))   // right    && no wall
            {
            }
            else if(!DoesHitAWall(dIdx))   // down     && no wall
            {
            }
            else if(!DoesHitAWall(lIdx))   // left     && no wall
            {
            }
            else if(!DoesHitAWall(uIdx))   // up       && no wall
            {
            }*/
            else
            {
                std::cout << "Houston, we have a problem.";
            }

        }
        else    // not start
        {
            AnIndex prevIdx = _traceList[_traceList.size() - 2];
            CCell prevCell = _cells[prevIdx.x][prevIdx.y];
            DirectionType prevDir = prevCell._directionType;

            // if curdir up right   && !hitawall(r)  && up right never visited
            // if curdir down right && !hitawall(dr) && down right never visited
            // if curdir down left  && !hitawall(d)  && down left never visited
            // if curdir up left    && !hitawall(c)  && up left never visited

            // if curdir up right   && hitawall(r)  --> up left    (next cell ?)
            // if curdir down right && hitawall(dr) --> down left  (next cell ?)
            // if curdir down left  && hitawall(d)  --> down right (next cell ?)
            // if curdir up left    && hitawall(c)  --> up right   (next cell ?)




            // if corner && prev up right
            // if corner && prev down right
            // if corner && prev down left
            // if corner && prev up left
            // if corner && prev right      && right cell not visited
            // if corner && prev down       && down cell not visited
            // if corner && prev left       && left cell not visited
            // if corner && prev up         &&

            // if hit the wall && cur up right
            // if hit the wall && cur down right
            // if hit the wall && cur down left
            // if hit the wall && cur up left
            // if hit the wall && cur right
            // if hit the wall && cur down
            // if hit the wall && cur left
            // if hit the wall && cur up
        }


        this->repaint();
    }
}

void GLWidget::InitCells()
{
    _cells.clear();
    _gridSpacing = 10;
    _gridSize = QSize(5, 5);

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

    // BOUNDARY
    for(size_t a = 0; a < _actualGridSize.width(); a++)
    {
        for(size_t b = 0; b < _actualGridSize.height(); b++)
        {
            if(a == 0 || a == _actualGridSize.width() - 1 || b == 0 || b == _actualGridSize.height() - 1)
            {
                _cells[a][b]._cellBreakMarker = CellBreakMarker::BREAK_MARKER_BREAK;
            }
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


