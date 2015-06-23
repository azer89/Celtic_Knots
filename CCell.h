#ifndef CCELL_H
#define CCELL_H

enum CellSign
{
    SIGN_ONE,
    SIGN_TWO,
    SIGN_EMPTY,
};

enum CellBreakMarker
{
    BREAK_MARKER_BREAK,
    BREAK_MARKER_EMPTY,
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
