#ifndef CCELL_H
#define CCELL_H

enum CellSign
{
    SIGN_ONE = 0,
    SIGN_TWO = 1,
    SIGN_EMPTY = 2,
};

enum CellBreakMarker
{
    BREAK_MARKER_BREAK = 0,
    BREAK_MARKER_EMPTY = 1,
};


struct CCell
{
public:
    CellSign _cellSign;
    CellBreakMarker _cellBreakMarker;

    CCell()
    {
        this->_cellSign = SIGN_EMPTY;
        this->_cellBreakMarker = BREAK_MARKER_EMPTY;
    }
};

#endif // CCELL_H
