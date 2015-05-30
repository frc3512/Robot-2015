// =============================================================================
// File Name: BezierCurve.cpp
// Description: Provides a way to more easily generate and manage Bézier curves
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "BezierCurve.hpp"
#include <cmath>

BezierCurve::BezierCurve(const std::pair<double, double>& pt1,
                         const std::pair<double, double>& pt2,
                         const std::pair<double, double>& pt3,
                         const std::pair<double,
                                         double>& pt4)
{
    push_back(pt1);
    push_back(pt2);
    push_back(pt3);
    push_back(pt4);
}

double BezierCurve::getArcLength(double start, double end) const {
    double length = 0.0;

    for (double t = start; t < end; t += 0.0001) {
        length += std::hypot(getDerivative(t, true), getDerivative(t, false)) *
                  0.0001;
    }

    return length;
}

double BezierCurve::getCurvature(double t) const {
    return (getDerivative(t, true) * getDerivative2(t, false) -
            getDerivative(t, false) * getDerivative2(t, true)) /
           std::pow(std::pow(getDerivative(t, true), 2.0) +
                    std::pow(getDerivative(t, false), 2.0), 1.5);
}

double BezierCurve::getValue(double t, bool xComponent) const {
    if (xComponent) {
        return std::pow(1 - t,
                        3) * (*this)[0].first + 3.0 *
               std::pow(1 - t,
                        2) * t * (*this)[1].first + 3.0 * (1 - t) * std::pow(t,
                                                                             2)
               * (*this)[2].first + std::pow(t, 3) * (*this)[3].first;
    }
    else {
        return std::pow(1 - t,
                        3) * (*this)[0].second + 3.0 *
               std::pow(1 - t,
                        2) * t * (*this)[1].second + 3.0 * (1 - t) * std::pow(t,
                                                                              2)
               * (*this)[2].second + std::pow(t, 3) * (*this)[3].second;
    }
}

double BezierCurve::getDerivative(double t, bool xComponent) const {
    if (xComponent) {
        return 3.0 *
               std::pow(1 - t,
                        2) * ((*this)[1].first - (*this)[0].first) + 6.0 *
               (1 - t) * t *
               ((*this)[2].first - (*this)[1].first) + 3.0 *
               std::pow(t, 2) * ((*this)[3].first - (*this)[2].first);
    }
    else {
        return 3.0 *
               std::pow(1 - t,
                        2) * ((*this)[1].second - (*this)[0].second) + 6.0 *
               (1 - t) * t *
               ((*this)[2].second - (*this)[1].second) + 3.0 *
               std::pow(t, 2) * ((*this)[3].second - (*this)[2].second);
    }
}

double BezierCurve::getDerivative2(double t, bool xComponent) const {
    if (xComponent) {
        return 6.0 * (1 - t) *
               ((*this)[2].first - 2.0 * (*this)[1].first + (*this)[0].first) +
               6.0 *
               t *
               ((*this)[3].first - 2.0 * (*this)[2].first + (*this)[1].first);
    }
    else {
        return 6.0 * (1 - t) *
               ((*this)[2].second - 2.0 * (*this)[1].second +
                (*this)[0].second) +
               6.0 * t *
               ((*this)[3].second - 2.0 * (*this)[2].second +
                (*this)[1].second);
    }
}

