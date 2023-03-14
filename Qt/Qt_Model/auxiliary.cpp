#include "auxiliary.h"

#include <iostream>

double rangeToPieSize(double range)
{
    return range / CHART_RANGE;
}

double positionToPiePosition(double position)
{
    return position / (2.0 * CHART_RANGE) + 0.5;
}

double spriteSizeToPosition(double size)
{
    return size / 800.0 * 2.0 * CHART_RANGE;
}

double degToRad(double deg)
{
    return deg / 180.0 * PI;
}

double radToDeg(double rad)
{
    return rad * 180.0 / PI;
}

uint64_t SimParameters::simulation_tick = 0;
double SimParameters::TIME_STEP = 0.03125;
double SimParameters::TIME_MULTIPLIER = 1.0;
double SimParameters::SIMULATION_TIME = 15.0;

uint64_t Random::state = 69;
uint64_t Random::randInt()
{
    state ^= state << 13;
    state ^= state << 7;
    state ^= state << 17;
    return state - 1;
}
uint64_t Random::randDouble()
{
    return static_cast<double>(randInt()) / static_cast<double>(std::numeric_limits<uint64_t>::max() - 1);
}

Position::Position()
    :x{0},y{0},z{0}
{

}

Position::Position(double x, double y, double z)
    :x{x}, y{y}, z{z}
{

}

Position Position::normalize()
{
    auto vec_length = length();
    return Position(x/vec_length, y/vec_length, z/vec_length);
}

double Position::length()
{
    return std::sqrt(x*x + y*y + z*z);
}

Position& Position::operator-=(Position const& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}
Position& Position::operator+=(Position const& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}
Position& Position::operator*=(double rhs)
{
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}


Position operator-(Position lhs, Position const& rhs)
{
    lhs -= rhs;
    return lhs;
}
Position operator+(Position lhs, Position const& rhs)
{
    lhs += rhs;
    return lhs;
}
Position operator*(Position lhs, double rhs)
{
    lhs *= rhs;
    return lhs;
}
double dot(Position const& lhs, Position const& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}
bool operator==(Position const& lhs, Position const& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.y == rhs.y);
}
