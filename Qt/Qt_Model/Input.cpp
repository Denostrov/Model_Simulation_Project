#include "Input.h"

void readFile(std::string const& filename, ModelDispatcher& dispatcher)
{
    std::string line;
    std::ifstream in(filename);
    if(!in)
    {
        return;
    }

    uint64_t count = 0;
    std::vector<double> read_vals;
    Position position;
    auto read_header = [&line, &in, &count]()
    {
        std::getline(in, line);
        line.erase(0,line.find(" ")+1);
        count = std::stoi(line);
    };
    auto read_double_values = [](std::vector<double>& vec, std::istream& in)
    {
        for (auto& val : vec)
        {
            in >> val;
        }
        return !in.fail();
    };
    auto read_position = [&position](std::istream& in)
    {
        in >> position.x >> position.y >> position.z;
        return !in.fail();
    };

    auto read_line = [] (std::istream& in)
    {
        std::string s;
        for (int i = 0; i < 11; i++)
        std::getline(in, s);
    };

    read_line(in);
    read_header();
    std::vector<RadarStation> radar_stations;
    Radar template_radar{0.0, 40.0, 2.0, 200.0, 120.0, 80.0, 4};
    for (uint64_t i = 0; i < count; i++)
    {
        read_position(in);
        radar_stations.push_back({template_radar, position, "Radar Station " + std::to_string(i)});
    }
    std::getline(in, line);

    read_header();
    std::vector<AirObject> air_objects;
    for (uint64_t i = 0; i < count; i++)
    {
        std::vector<Position> target_points;
        std::getline(in, line);
        std::istringstream target_points_stream(line);
        while (read_position(target_points_stream))
        {
            target_points.push_back(position);
        }

        //max_G, speed
        read_vals = {0.0, 0.0};
        read_double_values(read_vals, in);
        air_objects.push_back(AirObject{target_points, read_vals[0], read_vals[1]});

        std::getline(in, line);
    }

    std::vector<ControlCenter> control_centers;
    read_header();
    for (uint64_t i = 0; i < count; i++)
    {
        read_position(in);
        control_centers.push_back(ControlCenter{position, "Control Center " + std::to_string(i)});
    }
    std::getline(in, line);

    std::vector<LaunchSilo> launch_silos;
    read_header();
    std::vector<uint64_t> missile_count{0};
    for (uint64_t i = 0; i < count; i++)
    {
        read_position(in);
        in >> missile_count[0];
        launch_silos.push_back(LaunchSilo{position, "Launch Silo " + std::to_string(i), missile_count});
    }

    in.close();

    dispatcher.setRadarStations(radar_stations);
    dispatcher.setEnvironment(Environment(air_objects));
    dispatcher.setControlCenters(control_centers);
    dispatcher.setLaunchSilos(launch_silos);
}
