
/**
 * Bezier curve interpolation using DeCasteljau
 *
 *
 * Author: Reza Adhitya Saputra (reza.adhitya.saputra@gmail.com)
 * Version: 2014
 *
 *
 */

#ifndef __Curve_Interpolation__
#define __Curve_Interpolation__

#include <vector>
#include "AVector.h"

//namespace CVSystem
//{
    class CurveInterpolation
    {
    public:

        // recursive de casteljau
        static void DeCasteljau(std::vector<AVector>& poly, AVector p0, AVector p1, AVector p2, AVector p3, double subdivide_limit);

        // mid point de casteljau (non recursive)
        static AVector DeCasteljauMidPoint(AVector p0, AVector p1, AVector p2, AVector p3);

        // point interpolation in between two points
        static void PointInterpolation(std::vector<AVector>& poly, AVector pt1, AVector pt2, double f, double subLimit);

        static void GetAnchors(AVector p0, AVector p1, AVector p2, AVector p3, AVector& cp0, AVector& cp1, float smoothFactor);
    };
//}

#endif


