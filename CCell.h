#ifndef CCELL_H
#define CCELL_H

#include "DirectionType.h"
#include "TileType.h"

enum CellSign
{
    SIGN_ONE = 0,
    SIGN_TWO = 1,
    SIGN_EMPTY = 2,
};

enum CellBreakMarker
{
    BREAK_MARKER_BREAK  = 0,
    BREAK_MARKER_EMPTY  = 1,
    BREAK_MARKERINVALID = 2,
};


struct CCell
{
public:
    CellSign _cellSign;
    CellBreakMarker _cellBreakMarker;
    DirectionType _directionType;
    TileType _tileType;
    bool _isVisited;

    CCell()
    {
        this->_cellSign = SIGN_EMPTY;
        this->_cellBreakMarker = BREAK_MARKER_EMPTY;
        this->_directionType = DirectionType::DIR_NONE;
        this->_tileType = TileType::TILE_NONE;
        this->_isVisited = false;
    }
};

#endif // CCELL_H
