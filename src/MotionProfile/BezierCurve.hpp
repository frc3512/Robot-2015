// =============================================================================
// File Name: BezierCurve.hpp
// Description: Provides a way to more easily generate and manage Bézier curves
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef BEZIER_CURVE_HPP
#define BEZIER_CURVE_HPP

#include <vector>
#include <utility>

class BezierCurve : public std::vector<std::pair<double, double>> {
public:
    BezierCurve();
    BezierCurve(const std::pair<double, double>& pt1,
                const std::pair<double, double>& pt2,
                const std::pair<double, double>& pt3,
                const std::pair<double, double>& pt4);
    virtual ~BezierCurve();

    // 'start' and 'end' represent start and end t values [0..1]
    double getArcLength(double start, double end) const;
    double getCurvature(double t) const;

    // Return value and first and second derivatives at parameter t
    double getValue(double t, bool xComponent) const;
    double getDerivative(double t, bool xComponent) const;
    double getDerivative2(double t, bool xComponent) const;
};

#endif // BEZIER_CURVE_HPP

