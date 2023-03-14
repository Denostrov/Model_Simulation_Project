#include "Environment.h"

AirObject::AirObject(std::vector<Position> const& trajectory_targets, double max_G, double speed)
    :trajectory_targets{trajectory_targets}, max_G{max_G}, speed{speed}
{
    if (trajectory_targets.size() > 0)
    {
        current_position = trajectory_targets[0];
        current_target = 1;
    }
    else
    {
        current_position = Position{0.0, 0.0, 0.0};
        current_target = 0;
        direction = Vector3D(1.0, 0.0, 0.0);
    }
    if (current_target < trajectory_targets.size())
    {
        direction = (trajectory_targets[current_target] - current_position).normalize();
    }
}

std::pair<Position, double> AirObject::flatness_by_3_points(Position const& p1, Position const& p2, Position const& p3){
    Position ABC;
    ABC.x = (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y);
    ABC.y = (p2.x - p1.x) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.x - p1.x);
    ABC.z = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    double D = -(p1.x * ((p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y)) + p2.y * ((p2.x - p1.x) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.x - p1.x)) + p3.z * ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)));

    return std::pair<Position, double>(ABC, D);
}

Position AirObject::gaus_solve(std::array <std::array <double,4>,3> a) {
    for (int i = 0; i < 3; ++i) {
        if (a[i][i] == 0) {
            int not_zero = i;
            for (int j = i; j < 3; j++) {
                if (a[j][i] != 0) {
                    not_zero = j;
                }
            }
            a[i].swap(a[not_zero]);
        }

        double base_element = a[i][i];
        for (int j = 0; j <= 3; ++j) {
            a[i][j] = a[i][j]/base_element;
        }
        for (int j = 0; j < 3; j++) {
            if (j != i) {
                base_element = a[j][i];
                for (int k = 0; k < 4; ++k) {
                    a[j][k] -= base_element * a[i][k];
                }
            }
        }

    }

    return Position(a[0][3], a[1][3], a[2][3]);
}

Position AirObject::round_position(Position const & pos) {
    Position pos1 = pos;
    if (fabs(pos1.x) < 0.00001) {
        pos1.x = 0;
    }
    if (fabs(pos1.y) < 0.00001) {
        pos1.y = 0;
    }
    if (fabs(pos1.z) < 0.00001) {
        pos1.z = 0;
    }
    return(pos1);
}

Position AirObject::vector_product(Position const& p1, Position const & p2) {
    Position product;
    product.x = p1.y * p2.z - p1.z * p2.y;
    product.y = p1.z * p2.x - p1.x * p2.z;
    product.z = p1.x * p2.y - p1.y * p2.x;

    return product;
}

std::pair<double,double> AirObject::intersect(double a1, double a2, double b1, double b2, double c1, double c2)
{
    double det = a1 * b2 - a2 * b1;
    double x = (b1 * c2 - b2 * c1) / det;
    double y = (a2 * c1 - a1 * c2) / det;
    return std::pair<double, double>(x, y);
}


std::pair<Position, Position> AirObject::calculate_trajectory (Position const& direction, Position const& first_position, Position const& second_position){
    std::pair<Position, double> flatness = flatness_by_3_points(first_position + direction, first_position, second_position);
    Position n1 = vector_product(direction, flatness.first);
    Position n2 = vector_product(second_position - first_position, flatness.first);
    Position k = (first_position + second_position) * 0.5;
    if ((n1.y == 0) && (n2.y == 0)) {
        double x1 = first_position.x;
        double y1 = first_position.z;
        double x2 = (first_position + n1).x;
        double y2 = (first_position + n1).z;
        double x3 = k.x;
        double y3 = k.z;
        double x4 = (k + n2).x;
        double y4 = (k + n2).z;
        std::pair<double, double> xz = intersect(y1 - y2, y3 - y4, x2 - x1, x4 - x3, x1 * y2 - x2 * y1, x3 * x4 - x4 * y3);
        return std::pair<Position, Position>(Position(xz.first, first_position.y ,xz.second), flatness.first.normalize());
    }
    else {
        if (n1.x * n2.y - n1.y * n2.x == 0) {
            std::array<std::array <double, 4>, 3> A{ {std::array <double,4> {n1.y,-n1.x,0,first_position.x* n1.y - first_position.y * n1.x},
                                                      std::array <double,4> {0,n1.z,-n1.y,first_position.y* n1.z - first_position.z * n1.y},
                                                      std::array <double,4> {0,n2.z,-n2.y,(first_position.y + second_position.y) / 2 * n2.z - (first_position.z + second_position.z) / 2 * n2.y}} };
            return std::pair<Position, Position>(gaus_solve(A), flatness.first.normalize());
        }
        else {
            std::array<std::array <double, 4>, 3> A{ {std::array <double,4> {n1.y,-n1.x,0,first_position.x* n1.y - first_position.y * n1.x},
                                                      std::array <double,4> {0,n1.z,-n1.y,first_position.y* n1.z - first_position.z * n1.y},
                                                      std::array <double,4> {n2.y,-n2.x,0,(first_position.x + second_position.x) / 2 * n2.y - (first_position.y + second_position.y) / 2 * n2.x}} };
            return std::pair<Position, Position>(gaus_solve(A), flatness.first.normalize());
        }
    }
}



void AirObject::update()
{

    if (current_target < trajectory_targets.size())
    {
        if (round_position(vector_product(direction, trajectory_targets[current_target] - trajectory_targets[current_target - 1])) == Position(0, 0, 0)) {
            current_position += direction * speed * SimParameters::TIME_STEP;
        }
        else {
            if(!trajectory_created) {
                circle_trajectory = calculate_trajectory(direction, current_position, trajectory_targets[current_target]);
                trajectory_created = true;
                current_radius = (circle_trajectory.first - current_position).length();
            }
            if (speed*speed/(max_G*9.8) > current_radius){
                current_position += direction * speed * SimParameters::TIME_STEP;
                trajectory_created = false;
            }
            else {
                current_position += direction * speed * SimParameters::TIME_STEP + (circle_trajectory.first - current_position).normalize() * (speed * speed / 2 / current_radius * SimParameters::TIME_STEP * SimParameters::TIME_STEP);
                direction = (vector_product(current_position - circle_trajectory.first, circle_trajectory.second)).normalize();
            }
        }
        if ((current_position - trajectory_targets[current_target]).length() < 2*speed * SimParameters::TIME_STEP)
        {
            trajectory_created = false;
            current_position = trajectory_targets[current_target];
            current_target++;
        }
    }
}




Environment::Environment()
{

}

Environment::Environment(std::vector<AirObject> const &air_objects):
    air_objects{air_objects}
{

}

void Environment::update()
{
    log_targets.resize(air_objects.size());
    for (uint64_t i = 0; i < air_objects.size(); i++)
    {
        if (!air_objects[i].destroyed)
        {
            log_targets[i].push_back(air_objects[i].current_position);
        }
    }
    for (uint64_t i = 0; i < missiles.size(); i++)
    {
        missiles[i].update();
        if (missiles[i].exploded)
        {
            log_missiles.push_back(missiles[i].log_position);
            checkExplosion(missiles[i].current_position);
            missiles.erase(missiles.begin() + i);
            i--;
        }
    }
    for (auto& object : air_objects)
    {
        object.update();
    }
}

void Environment::checkExplosion(Position epicenter)
{
    for (auto& object : air_objects)
    {
        if ((object.current_position - epicenter).length() < 5.0)
        {
            object = AirObject{{{200, 0, 200}, {210, 0, 210}}, 1.0, 1.0};
            object.destroyed = true;
        }
    }
}

