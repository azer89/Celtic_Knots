#ifndef CCELL_H
#define CCELL_H

#include "DirectionType.h"
#include "TileType.h"

enum CellSign
{
    SIGN_ONE = 0,
    SIGN_TWO = 1,   // DUAL
    SIGN_EMPTY = 2,
};

struct CCell
{
public:
    CellSign _cellSign;
    DirectionType _directionType;
    bool _isVisited;

    CCell()
    {
        this->_cellSign = SIGN_EMPTY;
        this->_directionType = DirectionType::DIR_NONE;
        this->_isVisited = false;
    }
};

#endif // CCELL_H
