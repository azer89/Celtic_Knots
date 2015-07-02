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

enum Straightness
{
    ST_VERTICAL   = 0,
    ST_HORIZONTAL = 1,
    ST_DIAGONAL   = 2,
};

struct CCell
{
public:
    CellSign _cellSign;
    DirectionType _directionType;
    bool _isVisited;

    Straightness _straightness;     // am I straight in a narrow path ?
    DirectionType _tempDirection;   // go straight then turn

    CCell()
    {
        this->_cellSign = SIGN_EMPTY;
        this->_directionType = DirectionType::DIR_NONE;
        this->_isVisited = false;
        this->_straightness = Straightness::ST_DIAGONAL;
        this->_tempDirection = DirectionType::DIR_NONE;

    }
};

#endif // CCELL_H
