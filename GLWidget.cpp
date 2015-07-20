
#include "GLWidget.h"

#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>

#include <QGLFormat>
#include <QSvgGenerator>

#include "VertexData.h"
#include "SystemParams.h"
#include "CCell.h"

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


    InitCells();
    InitDots();

    // a bad thing
    ResetData();

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

    _tilePainter->DrawTiles();

    _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

    if(_dotsVao.isCreated() && SystemParams::show_grid)
    {
        int verticesPerDot = 4 + 2;
        _dotsVao.bind();
        uint nDots = _gridSize.width() * _gridSize.height();
        nDots += (_gridSize.width() + 1) * (_gridSize.height() + 1);
        for(uint a = 0; a < nDots; a++)
            { glDrawArrays(GL_TRIANGLE_FAN, a * verticesPerDot, verticesPerDot); }
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

    if(_breakLinesVao.isCreated() && SystemParams::show_grid)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _breakLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _breakLines.size() * 2);
        _breakLinesVao.release();
    }


    if(_cellLinesVao.isCreated() && SystemParams::show_grid)
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

        _drawBreakLine.XA = gVec.x; _drawBreakLine.YA = gVec.y;
        _drawBreakLine.XB = gVec.x; _drawBreakLine.YB = gVec.y;

        _drawStartIndex.x = idx.x; _drawStartIndex.y = idx.y;
        _drawEndIndex.x = idx.x; _drawEndIndex.y = idx.y;

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

        _drawBreakLine.XB = gVec.x;
        _drawBreakLine.YB = gVec.y;

        _drawEndIndex.x = idx.x;
        _drawEndIndex.y = idx.y;

        if(_drawBreakLine.GetLineType() == LineType::LINE_HORIZONTAL || _drawBreakLine.GetLineType() == LineType::LINE_VERTICAL)
        {
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

    AnIndex idx = GetIndex(AVector(dx, dy));

    // your stuff
    if(_drawBreakVao.isCreated() &&
       _cells[idx.x][idx.y]._cellSign != CellSign::SIGN_EMPTY &&
       (_drawBreakLine.GetLineType() == LineType::LINE_HORIZONTAL ||
        _drawBreakLine.GetLineType() == LineType::LINE_VERTICAL))
    {
        _breakLines.push_back(_drawBreakLine);
        PrepareLinesVAO(_breakLines, &_breakLinesVbo, &_breakLinesVao, QVector3D(0.0, 0.0, 0.0));
    }

    CalculateNarrowPath();
    std::cout << "\n";

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

bool GLWidget::IsValid(AnIndex idx)
{
    QSize gridDim(_actualGridSize.width() -1, _actualGridSize.height() -1);

    if(idx.x < 0 || idx.x >= gridDim.width() || idx.y < 0 || idx.y >= gridDim.height())
        { return false; }
    return true;
}

/*
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
}*/

bool GLWidget::IntersectHorizontalLine(AVector pt)
{
    float eps = std::numeric_limits<float>::epsilon() * 1000; // why ???
    for(int a = 0; a < _breakLines.size(); a++)
    {
        float d = DistanceToFiniteLine(_breakLines[a].GetPointA(), _breakLines[a].GetPointB(), pt);
        if(d < eps)
        {
            if(_breakLines[a].GetLineType() == LineType::LINE_HORIZONTAL)
            {
                return true;
            }
        }
    }
    return false;
}

bool GLWidget::IntersectVerticalLine(AVector pt)
{
    float eps = std::numeric_limits<float>::epsilon() * 100; // why ???
    for(int a = 0; a < _breakLines.size(); a++)
    {
        float d = DistanceToFiniteLine(_breakLines[a].GetPointA(), _breakLines[a].GetPointB(), pt);
        if(d < eps)
        {
            if(_breakLines[a].GetLineType() == LineType::LINE_VERTICAL)
            {
                return true;
            }
        }
    }
    return false;
}

LineType GLWidget::GetLineIntersection(AVector pt)
{
    float dist = std::numeric_limits<float>::max();
    LineType lType = LineType::LINE_NONE;
    float eps = std::numeric_limits<float>::epsilon() * 100; // why ???
    for(int a = 0; a < _breakLines.size(); a++)
    {
        ALine aLine = _breakLines[a];
        float d = DistanceToFiniteLine(aLine.GetPointA(), aLine.GetPointB(), pt);
        if(d < dist)
        {
            dist = d;
            lType = aLine.GetLineType();
        }
    }

    if(dist < eps) { return lType; }

    return LineType::LINE_NONE;
}

void GLWidget::UndoBreakMarkers()
{
    // 4 never delete the borders
    if(_breakLines.size() > 4) { _breakLines.pop_back(); }
}

void GLWidget::ResetData()
{
    _isTracingDone = false;
    _traceList.clear();

    for(int a = 0; a < _cells.size(); a++)
    {
        for(int b = 0; b < _cells[a].size(); b++)
            { _cells[a][b]._isVisited = false; }
    }

    if(_tilePainter) { delete _tilePainter; }

    _tilePainter = new TilePainter();
    _tilePainter->_shaderProgram = _shaderProgram;
    _tilePainter->_colorLocation = _colorLocation;
    _tilePainter->_vertexLocation = _vertexLocation;
    _tilePainter->_use_color_location = _use_color_location;
}

void GLWidget::GenerateAKnot()
{
    ResetData();

    while(!_isTracingDone)
    {
        TraceOneStep();
    }
}

void GLWidget::CalculateNarrowPath()
{
    for(int b = 0; b < _actualGridSize.height() - 1; b++)
    {
        for(int a = 0; a < _actualGridSize.width() - 1; a++)
        {
            AVector curVec(_gridSpacing *  a,      _gridSpacing * b);
            AVector rVec  (_gridSpacing * (a + 1), _gridSpacing *  b    );
            AVector dVec  (_gridSpacing *  a     , _gridSpacing * (b + 1));
            AVector drVec (_gridSpacing * (a + 1), _gridSpacing * (b + 1));

            // horizontal
            if(IntersectHorizontalLine(curVec) &&
               IntersectHorizontalLine(rVec)   &&
               IntersectHorizontalLine(dVec) &&
               IntersectHorizontalLine(drVec)   )
            {
                _cells[a][b]._straightness = Straightness::ST_HORIZONTAL;
                std::cout << "-";
            }
            // vertical
            else if(IntersectVerticalLine(curVec) &&
                    IntersectVerticalLine(rVec) &&
                    IntersectVerticalLine(dVec) &&
                    IntersectVerticalLine(drVec))
            {
                _cells[a][b]._straightness = Straightness::ST_VERTICAL;
                std::cout << "|";
            }
            else
            {
                _cells[a][b]._straightness = Straightness::ST_DIAGONAL;
                std::cout << ".";
            }
        }
        std::cout << "\n";
    }

    std::cout << "\n";
    std::cout << "\n";
}

void GLWidget::TraceOneStep()
{
    if(_traceList.size() == 0)
    {
        _isTracingDone = false;

        AnIndex startIdx(0, 0);

        _traceList.push_back(startIdx); // put in list
        _cells[startIdx.x][startIdx.y]._isVisited = true; // mark
        _cells[startIdx.x][startIdx.y]._directionType = DirectionType::DIR_UPRIGHT; // (0,0) always upright or downleft

        _tilePainter->SetTiles(_cells, _traceList, _gridSpacing, _isTracingDone);
        this->repaint();
    }
    else if(!_isTracingDone)
    {
        AnIndex curIdx = _traceList[_traceList.size() - 1];
        _cells[curIdx.x][curIdx.y]._isVisited = true;

        DirectionType curDir =_cells[curIdx.x][curIdx.y]._directionType;

        AnIndex urIdx(curIdx.x + 1, curIdx.y - 1);    // up right
        AnIndex drIdx(curIdx.x + 1, curIdx.y + 1);    // down right
        AnIndex dlIdx(curIdx.x - 1, curIdx.y + 1);    // down left
        AnIndex ulIdx(curIdx.x - 1, curIdx.y - 1);    // up left

        AnIndex rIdx(curIdx.x + 1, curIdx.y    );     // right
        AnIndex dIdx(curIdx.x    , curIdx.y + 1);     // down
        AnIndex lIdx(curIdx.x - 1, curIdx.y    );     // left
        AnIndex uIdx(curIdx.x    , curIdx.y - 1);     // up

        // point-to-line intersection
        AVector endVec;
        if(curDir == DirectionType::DIR_UPRIGHT)
            { endVec = AVector(rIdx.x  * _gridSpacing, rIdx.y * _gridSpacing); }
        else if(curDir == DirectionType::DIR_DOWNRIGHT)
            { endVec = AVector(drIdx.x  * _gridSpacing, drIdx.y * _gridSpacing); }
        else if(curDir == DirectionType::DIR_DOWNLEFT)
            { endVec = AVector(dIdx.x  * _gridSpacing, dIdx.y * _gridSpacing); }
        else if(curDir == DirectionType::DIR_UPLEFT)
            { endVec = AVector(curIdx.x  * _gridSpacing, curIdx.y * _gridSpacing); }
        LineType hitType = GetLineIntersection(endVec);


        // new code

        // enter
        if(curDir == DirectionType::DIR_RIGHT &&
           IsValid(rIdx) &&
           _cells[rIdx.x][rIdx.y]._straightness == Straightness::ST_HORIZONTAL)
        {
            //std::cout << "[a] " << "right --> right" << " - " << rIdx.x << " " << rIdx.y << "\n";
            _traceList.push_back(rIdx);
            _cells[rIdx.x][rIdx.y]._directionType = DirectionType::DIR_RIGHT;
            _cells[rIdx.x][rIdx.y]._tempDirection = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_LEFT &&
                IsValid(lIdx) &&
                _cells[lIdx.x][lIdx.y]._straightness == Straightness::ST_HORIZONTAL)
        {
            //std::cout << "[b] " << "left --> left" << " - " << lIdx.x << " " << lIdx.y << "\n";
            _traceList.push_back(lIdx);
            _cells[lIdx.x][lIdx.y]._directionType = DirectionType::DIR_LEFT;
            _cells[lIdx.x][lIdx.y]._tempDirection = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_UP &&
                IsValid(uIdx) &&
                _cells[uIdx.x][uIdx.y]._straightness == Straightness::ST_VERTICAL)
        {
            //std::cout << "[c] " << "up --> up" << " - " << uIdx.x << " " << uIdx.y << "\n";
            _traceList.push_back(uIdx);
            _cells[uIdx.x][uIdx.y]._directionType = DirectionType::DIR_UP;
            _cells[uIdx.x][uIdx.y]._tempDirection = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_DOWN &&
                IsValid(dIdx) &&
                _cells[dIdx.x][dIdx.y]._straightness == Straightness::ST_VERTICAL)
        {
            //std::cout << "[d] " << "down --> down" << " - " << dIdx.x << " " << dIdx.y << "\n";
            _traceList.push_back(dIdx);
            _cells[dIdx.x][dIdx.y]._directionType = DirectionType::DIR_DOWN;
            _cells[dIdx.x][dIdx.y]._tempDirection = _cells[curIdx.x][curIdx.y]._tempDirection;
        }

        // out
        else if(curDir == DirectionType::DIR_RIGHT &&
           IsValid(rIdx) &&
           _cells[rIdx.x][rIdx.y]._straightness == Straightness::ST_DIAGONAL)
        {
            //std::cout << "[e] " << "right --> out" << " - " << rIdx.x << " " << rIdx.y << "\n";
            _traceList.push_back(rIdx);
            _cells[rIdx.x][rIdx.y]._directionType = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_LEFT &&
                IsValid(lIdx) &&
                _cells[lIdx.x][lIdx.y]._straightness == Straightness::ST_DIAGONAL)
        {
            //std::cout << "[f] " << "left --> out" << " - " << lIdx.x << " " << lIdx.y << "\n";
            _traceList.push_back(lIdx);
            _cells[lIdx.x][lIdx.y]._directionType = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_UP &&
                IsValid(uIdx) &&
                _cells[uIdx.x][uIdx.y]._straightness == Straightness::ST_DIAGONAL)
        {
            //std::cout << "[g] " << "up --> out" << " - " << uIdx.x << " " << uIdx.y << "\n";
            _traceList.push_back(uIdx);
            _cells[uIdx.x][uIdx.y]._directionType = _cells[curIdx.x][curIdx.y]._tempDirection;
        }
        else if(curDir == DirectionType::DIR_DOWN &&
                IsValid(dIdx) &&
                _cells[dIdx.x][dIdx.y]._straightness == Straightness::ST_DIAGONAL)
        {
            //std::cout << "[h] " << "down --> out" << " - " << dIdx.x << " " << dIdx.y << "\n";
            _traceList.push_back(dIdx);
            _cells[dIdx.x][dIdx.y]._directionType = _cells[curIdx.x][curIdx.y]._tempDirection;
        }

        else if(curDir == DirectionType::DIR_UPRIGHT &&
           hitType == LineType::LINE_HORIZONTAL &&
           IsValid(rIdx) &&
           _cells[rIdx.x][rIdx.y]._straightness == Straightness::ST_HORIZONTAL) // upright --> right
        {

            //std::cout << "[1] " << "upright --> right" << " - " << rIdx.x << " " << rIdx.y << "\n";
            // rIdx
            _traceList.push_back(rIdx);
            _cells[rIdx.x][rIdx.y]._directionType = DirectionType::DIR_RIGHT;
            _cells[rIdx.x][rIdx.y]._tempDirection = DirectionType::DIR_DOWNRIGHT;
        }
        else if(curDir == DirectionType::DIR_DOWNRIGHT  &&
                hitType == LineType::LINE_HORIZONTAL &&
                IsValid(rIdx) &&
                _cells[rIdx.x][rIdx.y]._straightness == Straightness::ST_HORIZONTAL) // downright --> right
        {
            //std::cout << "[2] " << "downright --> right" << " - " << rIdx.x << " " << rIdx.y << "\n";
            // rIdx
            _traceList.push_back(rIdx);
            _cells[rIdx.x][rIdx.y]._directionType = DirectionType::DIR_RIGHT;
            _cells[rIdx.x][rIdx.y]._tempDirection = DirectionType::DIR_UPRIGHT;
        }
        else if(curDir == DirectionType::DIR_DOWNLEFT &&
                hitType == LineType::LINE_HORIZONTAL &&
                IsValid(lIdx) &&
                _cells[lIdx.x][lIdx.y]._straightness == Straightness::ST_HORIZONTAL) // downleft --> left
        {
            //std::cout << "[3] " << "downleft --> left" << " - " << lIdx.x << " " << lIdx.y << "\n";
            // lIdx
            _traceList.push_back(lIdx);
            _cells[lIdx.x][lIdx.y]._directionType = DirectionType::DIR_LEFT;
            _cells[lIdx.x][lIdx.y]._tempDirection = DirectionType::DIR_UPLEFT;
        }
        else if(curDir == DirectionType::DIR_UPLEFT &&
                hitType == LineType::LINE_HORIZONTAL &&
                IsValid(lIdx) &&
                _cells[lIdx.x][lIdx.y]._straightness == Straightness::ST_HORIZONTAL)    // upleft --> left
        {
            //std::cout << "[4] " << "// upleft --> left" << " - " << lIdx.x << " " << lIdx.y << "\n";
            // lIdx
            _traceList.push_back(lIdx);
            _cells[lIdx.x][lIdx.y]._directionType = DirectionType::DIR_LEFT;
            _cells[lIdx.x][lIdx.y]._tempDirection = DirectionType::DIR_DOWNLEFT;
        }

        else if(curDir == DirectionType::DIR_UPRIGHT &&
                hitType == LineType::LINE_VERTICAL &&
                IsValid(uIdx) &&
                _cells[uIdx.x][uIdx.y]._straightness == Straightness::ST_VERTICAL) // upright --> up
        {
            //std::cout << "[5] " << "upright --> up" << " - " << uIdx.x << " " << uIdx.y << "\n";
            // uIdx
            _traceList.push_back(uIdx);
            _cells[uIdx.x][uIdx.y]._directionType = DirectionType::DIR_UP;
            _cells[uIdx.x][uIdx.y]._tempDirection = DirectionType::DIR_UPLEFT;
        }
        else if(curDir == DirectionType::DIR_DOWNRIGHT  &&
                hitType == LineType::LINE_VERTICAL &&
                IsValid(dIdx) &&
                _cells[dIdx.x][dIdx.y]._straightness == Straightness::ST_VERTICAL) // downright --> down
        {
            //std::cout << "[6] " << "downright --> down" << " - " << dIdx.x << " " << dIdx.y << "\n";
            // dIdx
            _traceList.push_back(dIdx);
            _cells[dIdx.x][dIdx.y]._directionType = DirectionType::DIR_DOWN;
            _cells[dIdx.x][dIdx.y]._tempDirection = DirectionType::DIR_DOWNLEFT;
        }
        else if(curDir == DirectionType::DIR_DOWNLEFT &&
                hitType == LineType::LINE_VERTICAL &&
                IsValid(dIdx) &&
                _cells[dIdx.x][dIdx.y]._straightness == Straightness::ST_VERTICAL) // downleft --> down
        {
            //std::cout << "[7] " << "downleft --> down" << " - " << dIdx.x << " " << dIdx.y << "\n";
            // dIdx
            _traceList.push_back(dIdx);
            _cells[dIdx.x][dIdx.y]._directionType = DirectionType::DIR_DOWN;
            _cells[dIdx.x][dIdx.y]._tempDirection = DirectionType::DIR_DOWNRIGHT;
        }
        else if(curDir == DirectionType::DIR_UPLEFT &&
                hitType == LineType::LINE_VERTICAL &&
                IsValid(uIdx) &&
                _cells[uIdx.x][uIdx.y]._straightness == Straightness::ST_VERTICAL)    // upleft --> up
        {
            //std::cout << "[8] " << "upleft --> up" << " - " << uIdx.x << " " << uIdx.y << "\n";
            // uIdx
            _traceList.push_back(uIdx);
            _cells[uIdx.x][uIdx.y]._directionType = DirectionType::DIR_UP;
            _cells[uIdx.x][uIdx.y]._tempDirection = DirectionType::DIR_UPRIGHT;
        }

        // old code
        else if(curDir == DirectionType::DIR_UPRIGHT && IsValid(urIdx) && hitType == LineType::LINE_NONE)
        {
            //std::cout << "[9] " << hitType << " - " << urIdx.x << " " << urIdx.y << "\n";
            _traceList.push_back(urIdx);
            _cells[urIdx.x][urIdx.y]._directionType = DirectionType::DIR_UPRIGHT;
        }
        else if(curDir == DirectionType::DIR_DOWNRIGHT && IsValid(drIdx) && hitType == LineType::LINE_NONE)
        {
            //std::cout << "[10] " << hitType << " - " << drIdx.x << " " << drIdx.y << "\n";
            _traceList.push_back(drIdx);
            _cells[drIdx.x][drIdx.y]._directionType = DirectionType::DIR_DOWNRIGHT;
        }
        else if(curDir == DirectionType::DIR_DOWNLEFT && IsValid(dlIdx) && hitType == LineType::LINE_NONE)
        {
            //std::cout << "[11] " << hitType << " - " << dlIdx.x << " " << dlIdx.y << "\n";
            _traceList.push_back(dlIdx);
            _cells[dlIdx.x][dlIdx.y]._directionType = DirectionType::DIR_DOWNLEFT;
        }
        else if(curDir == DirectionType::DIR_UPLEFT && IsValid(ulIdx) && hitType == LineType::LINE_NONE)
        {
            //std::cout << "[12] " << hitType << " - " << ulIdx.x << " " << ulIdx.y << "\n";
            _traceList.push_back(ulIdx);
            _cells[ulIdx.x][ulIdx.y]._directionType = DirectionType::DIR_UPLEFT;
        }
        else if(curDir == DirectionType::DIR_UPRIGHT && ( IsValid(rIdx) || IsValid(uIdx) )  )
        {
            if(hitType == LineType::LINE_HORIZONTAL && IsValid(rIdx))
            {
                //std::cout << "[13] " << hitType << " - " << rIdx.x << " " << rIdx.y << "\n";
                _traceList.push_back(rIdx);
                _cells[rIdx.x][rIdx.y]._directionType = DirectionType::DIR_DOWNRIGHT;
            }
            else
            {
                //std::cout << "[14] " << hitType << " - " << uIdx.x << " " << uIdx.y << "\n";
                _traceList.push_back(uIdx);
                _cells[uIdx.x][uIdx.y]._directionType = DirectionType::DIR_UPLEFT;
            }
        }
        else if(curDir == DirectionType::DIR_DOWNRIGHT && ( IsValid(rIdx) || IsValid(dIdx) ))
        {
            if(hitType == LineType::LINE_HORIZONTAL && IsValid(rIdx))
            {
                //std::cout << "[15] " << hitType << " - " << rIdx.x << " " << rIdx.y << "\n";
                _traceList.push_back(rIdx);
                _cells[rIdx.x][rIdx.y]._directionType = DirectionType::DIR_UPRIGHT;
            }
            else
            {
                //std::cout << "[16] " << hitType << " - " << dIdx.x << " " << dIdx.y << "\n";
                _traceList.push_back(dIdx);
                _cells[dIdx.x][dIdx.y]._directionType = DirectionType::DIR_DOWNLEFT;
            }
        }
        else if(curDir == DirectionType::DIR_DOWNLEFT && ( IsValid(lIdx) || IsValid(dIdx) ))
        {
            if(hitType == LineType::LINE_HORIZONTAL && IsValid(lIdx))   // up left
            {
                //std::cout << "[17] " << hitType << " - " << lIdx.x << " " << lIdx.y << "\n";
                _traceList.push_back(lIdx);
                _cells[lIdx.x][lIdx.y]._directionType = DirectionType::DIR_UPLEFT;
            }
            else        // down right
            {
                //std::cout << "[18] " << hitType << " - " << dIdx.x << " " << dIdx.y << "\n";
                _traceList.push_back(dIdx);
                _cells[dIdx.x][dIdx.y]._directionType = DirectionType::DIR_DOWNRIGHT;
            }
        }
        else if(curDir == DirectionType::DIR_UPLEFT && ( IsValid(lIdx) || IsValid(uIdx) ))
        {
            if(hitType == LineType::LINE_HORIZONTAL && IsValid(lIdx))
            {
                //std::cout << "[19] " << hitType << " - " << lIdx.x << " " << lIdx.y << "\n";
                _traceList.push_back(lIdx);
                _cells[lIdx.x][lIdx.y]._directionType = DirectionType::DIR_DOWNLEFT;
            }
            else
            {
                //std::cout << "[20] " << hitType << " - " << uIdx.x << " " << uIdx.y << "\n";
                _traceList.push_back(uIdx);
                _cells[uIdx.x][uIdx.y]._directionType = DirectionType::DIR_UPRIGHT;
            }
        }

        // check if we revisit a cell which means done
        AnIndex nextIdx = _traceList[_traceList.size() - 1];
        if(_cells[nextIdx.x][nextIdx.y]._isVisited)
        {
            //std::cout << "end here " << nextIdx.x << " " << nextIdx.y << "\n";
            _isTracingDone = true;
        }

        _tilePainter->SetTiles(_cells, _traceList, _gridSpacing, _isTracingDone);
        this->repaint();
    }
}


void GLWidget::InitCells()
{
    _cells.clear();
    _gridSpacing = 10;
    _gridSize = QSize(SystemParams::w , SystemParams::h);

    // add one row and one column
    _actualGridSize = QSize((_gridSize.width() - 1) * 2 + 1, (_gridSize.height() - 1) * 2 + 1 );

    _img_width  = (_actualGridSize.width() - 1) * _gridSpacing;
    _img_height = (_actualGridSize.height() - 1) * _gridSpacing;

    // add break lines
    _breakLines.clear();
    _breakLines.push_back(ALine(0, 0, _img_width, 0));
    _breakLines.push_back(ALine(0, 0, 0, _img_height));
    _breakLines.push_back(ALine(_img_width, 0, _img_width, _img_height));
    _breakLines.push_back(ALine(0, _img_height, _img_width, _img_height));
    PrepareLinesVAO(_breakLines, &_breakLinesVbo, &_breakLinesVao, QVector3D(0.0, 0.0, 0.0));


    for(int a = 0; a < _actualGridSize.width(); a++)
        { _cells.push_back(std::vector<CCell>(_actualGridSize.height())); }


    // ONE
    for(int a = 0; a < _actualGridSize.width(); a += 2)
    {
        for(int b = 0; b < _actualGridSize.height(); b += 2)
            { _cells[a][b]._cellSign = CellSign::SIGN_ONE; }
    }

    // TWO
    for(int a = 1; a < _actualGridSize.width(); a += 2)
    {
        for(int b = 1; b < _actualGridSize.height(); b += 2)
            { _cells[a][b]._cellSign = CellSign::SIGN_TWO; }
    }

    // vao
    _cellLines.clear();
    for(int a = 0; a < _actualGridSize.width() - 1; a++)
    {
        for(int b = 0; b < _actualGridSize.height() - 1; b++)
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
    if(_dotsVao.isCreated()) { _dotsVao.destroy(); }
    _dotsVao.create();
    _dotsVao.bind();

    float radius = 1.0f;

    QVector<VertexData> vertices;

    // ONE
    for(int a = 0; a < _actualGridSize.width(); a += 2)
    {
        for(int b = 0; b < _actualGridSize.height(); b += 2)
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
    for(int a = 1; a < _actualGridSize.width(); a += 2)
    {
        for(int b = 1; b < _actualGridSize.height(); b += 2)
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
        { ptsVao->destroy(); }

    ptsVao->create();
    ptsVao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < points.size(); a++)
        { data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol)); }

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

double GLWidget::DistanceToFiniteLine(AVector v, AVector w, AVector p)
{
    float zero_epsilon = std::numeric_limits<float>::epsilon();

    // Return minimum distance between line segment vw and point p
    double l2 = v.DistanceSquared(w);					   // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 > -zero_epsilon && l2 < zero_epsilon) return p.Distance(v);   // v == w case

    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    double t = (p - v).Dot(w - v) / l2;

    if (t < 0.0)	  { return  p.Distance(v); }       // Beyond the 'v' end of the segment
    else if (t > 1.0) { return  p.Distance(w); }  // Beyond the 'w' end of the segment
    AVector projection = v + (w - v) * t;     // Projection falls on the segment
    return p.Distance(projection);
}


