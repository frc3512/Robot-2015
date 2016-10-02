// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "BezierCurve.hpp"

#include <cmath>

Point::Point(double x, double y) {
    this->x = x;
    this->y = y;
}

BezierCurve::BezierCurve(const Point& pt1, const Point& pt2, const Point& pt3,
                         const Point& pt4) {
    m_pts.push_back(pt1);
    m_pts.push_back(pt2);
    m_pts.push_back(pt3);
    m_pts.push_back(pt4);
}

void BezierCurve::AddPoint(double x, double y) { m_pts.emplace_back(x, y); }

void BezierCurve::clear() { m_pts.clear(); }

double BezierCurve::getArcLength(double start, double end) const {
    double length = 0.0;

    for (double t = start; t < end; t += 0.0001) {
        length += std::hypot(getDerivativeX(t), getDerivativeY(t)) * 0.0001;
    }

    return length;
}

double BezierCurve::getCurvature(double t) const {
    return (getDerivativeX(t) * getDerivative2Y(t) -
            getDerivativeY(t) * getDerivative2X(t)) /
           std::pow(std::pow(getDerivativeX(t), 2.0) +
                        std::pow(getDerivativeY(t), 2.0),
                    1.5);
}

double BezierCurve::getValueX(double t) const {
    return std::pow(1 - t, 3) * m_pts[0].x +
           3.0 * std::pow(1 - t, 2) * t * m_pts[1].x +
           3.0 * (1 - t) * std::pow(t, 2) * m_pts[2].x +
           std::pow(t, 3) * m_pts[3].x;
}

double BezierCurve::getValueY(double t) const {
    return std::pow(1 - t, 3) * m_pts[0].y +
           3.0 * std::pow(1 - t, 2) * t * m_pts[1].y +
           3.0 * (1 - t) * std::pow(t, 2) * m_pts[2].y +
           std::pow(t, 3) * m_pts[3].y;
}

double BezierCurve::getDerivativeX(double t) const {
    return 3.0 * std::pow(1 - t, 2) * (m_pts[1].x - m_pts[0].x) +
           6.0 * (1 - t) * t * (m_pts[2].x - m_pts[1].x) +
           3.0 * std::pow(t, 2) * (m_pts[3].x - m_pts[2].x);
}

double BezierCurve::getDerivativeY(double t) const {
    return 3.0 * std::pow(1 - t, 2) * (m_pts[1].y - m_pts[0].y) +
           6.0 * (1 - t) * t * (m_pts[2].y - m_pts[1].y) +
           3.0 * std::pow(t, 2) * (m_pts[3].y - m_pts[2].y);
}

double BezierCurve::getDerivative2X(double t) const {
    return 6.0 * (1 - t) * (m_pts[2].x - 2.0 * m_pts[1].x + m_pts[0].x) +
           6.0 * t * (m_pts[3].x - 2.0 * m_pts[2].x + m_pts[1].x);
}

double BezierCurve::getDerivative2Y(double t) const {
    return 6.0 * (1 - t) * (m_pts[2].y - 2.0 * m_pts[1].y + m_pts[0].y) +
           6.0 * t * (m_pts[3].y - 2.0 * m_pts[2].y + m_pts[1].y);
}
