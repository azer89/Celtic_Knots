

/**
 *
 * Reza Adhitya Saputra (reza.adhitya.saputra@gmail.com)
 * Version: 2014
 *
 */

//#include "stdafx.h"
#include "CurveInterpolation.h"

void CurveInterpolation::PointInterpolation(std::vector<AVector>& poly, AVector pt1, AVector pt2, double f, double subLimit)
{
    if(pt1.Distance(pt2) <= subLimit)
    {
        poly.push_back(pt1);
    }
    else
    {
        AVector newPt = pt1 + (pt2 - pt1) * f;
        PointInterpolation(poly, pt1, newPt, f, subLimit);
        PointInterpolation(poly, newPt, pt2, f, subLimit);
    }
}

AVector CurveInterpolation::DeCasteljauMidPoint(AVector p0, AVector p1, AVector p2, AVector p3)
{
    double splitParam = 0.5;	// split into two equal parts

    double x0 = p0.x; double y0 = p0.y;
    double x1 = p1.x; double y1 = p1.y;
    double x2 = p2.x; double y2 = p2.y;
    double x3 = p3.x; double y3 = p3.y;

    double x01 = (x1 - x0) * splitParam + x0;		double x12 = (x2 - x1) * splitParam + x1;		double x23 = (x3 - x2) * splitParam + x2;
    double y01 = (y1 - y0) * splitParam + y0;		double y12 = (y2 - y1) * splitParam + y1;		double y23 = (y3 - y2) * splitParam + y2;

    double x012 = (x12 - x01) * splitParam + x01;	double x123 = (x23 - x12) * splitParam + x12;
    double y012 = (y12 - y01) * splitParam + y01;	double y123 = (y23 - y12) * splitParam + y12;

    double x0123 = (x123 - x012) * splitParam + x012;
    double y0123 = (y123 - y012) * splitParam + y012;

    return AVector(x0123, y0123);
}


void CurveInterpolation::DeCasteljau(std::vector<AVector>& poly, AVector p0, AVector p1, AVector p2, AVector p3, double subdivide_limit)
{
    if(p0.Distance(p3) <= subdivide_limit)
    {
        poly.push_back(p0);
    }
    else
    {
        double splitParam = 0.5;	// split into two equal parts

        double x0 = p0.x; double y0 = p0.y;
        double x1 = p1.x; double y1 = p1.y;
        double x2 = p2.x; double y2 = p2.y;
        double x3 = p3.x; double y3 = p3.y;

        double x01 = (x1 - x0) * splitParam + x0;		double x12 = (x2 - x1) * splitParam + x1;		double x23 = (x3 - x2) * splitParam + x2;
        double y01 = (y1 - y0) * splitParam + y0;		double y12 = (y2 - y1) * splitParam + y1;		double y23 = (y3 - y2) * splitParam + y2;

        double x012 = (x12 - x01) * splitParam + x01;	double x123 = (x23 - x12) * splitParam + x12;
        double y012 = (y12 - y01) * splitParam + y01;	double y123 = (y23 - y12) * splitParam + y12;

        double x0123 = (x123 - x012) * splitParam + x012;
        double y0123 = (y123 - y012) * splitParam + y012;

        DeCasteljau(poly, AVector(x0, y0),        AVector(x01, y01),   AVector(x012, y012), AVector(x0123, y0123), subdivide_limit);
        DeCasteljau(poly, AVector(x0123,  y0123), AVector(x123, y123), AVector(x23, y23),   AVector(x3, y3),	   subdivide_limit);
    }
}

void CurveInterpolation::GetAnchors(AVector p0, AVector p1, AVector p2, AVector p3, AVector& cp0, AVector& cp1, float smoothFactor)
{
    double xc1 = (p0.x + p1.x) / 2.0;		double yc1 = (p0.y + p1.y) / 2.0;
    double xc2 = (p1.x + p2.x) / 2.0;		double yc2 = (p1.y + p2.y) / 2.0;
    double xc3 = (p2.x + p3.x) / 2.0;		double yc3 = (p2.y + p3.y) / 2.0;

    double len1 = sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
    double len2 = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
    double len3 = sqrt((p3.x - p2.x) * (p3.x - p2.x) + (p3.y - p2.y) * (p3.y - p2.y));

    double k1 = len1 / (len1 + len2);		double k2 = len2 / (len2 + len3);

    double xm1 = xc1 + (xc2 - xc1) * k1;	double ym1 = yc1 + (yc2 - yc1) * k1;
    double xm2 = xc2 + (xc3 - xc2) * k2;	double ym2 = yc2 + (yc3 - yc2) * k2;

    // Resulting control points. Here smooth_value is mentioned
    // above coefficient K whose value should be in range [0...1].
    cp0.x = xm1 + (xc2 - xm1) * smoothFactor + p1.x - xm1;
    cp0.y = ym1 + (yc2 - ym1) * smoothFactor + p1.y - ym1;

    cp1.x = xm2 + (xc2 - xm2) * smoothFactor + p2.x - xm2;
    cp1.y = ym2 + (yc2 - ym2) * smoothFactor + p2.y - ym2;
}
