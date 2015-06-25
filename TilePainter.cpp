
#include "TilePainter.h"

#include <stdlib.h>
#include <iostream>


TilePainter::TilePainter()
{
}

TilePainter::~TilePainter()
{
}

void TilePainter::SetTiles(std::vector<std::vector<CCell>> cells, float gridSpacing)
{
    if(_vao.isCreated())
    {
        _vao.destroy();
        _lines.clear();
    }


    QSize gridSize(cells.size(), cells[0].size());
    for(size_t a = 0; a < gridSize.width(); a++)
    {
        for(size_t b = 0; b < gridSize.height(); b++)
        {
            if(cells[a][b]._tileType == TileType::TILE_CORNER)
            {
                CreateCorner(cells[a][b], AnIndex(a, b), gridSpacing);
            }
            else
            {
                std::cout << ".";
            }
        }
        std::cout << "\n";
    }

}

void TilePainter::DrawTiles()
{

}

void TilePainter::CreateCorner(CCell cell, AnIndex idx, float gridSpacing)
{
    std::cout << "c";
}

void TilePainter::CreateCross(CCell cell, AnIndex idx, float gridSpacing)
{
}

void TilePainter::CreateSlash(CCell cell, AnIndex idx, float gridSpacing)
{
}

void TilePainter::CreateStraight(CCell cell, AnIndex idx, float gridSpacing)
{
}

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
