

#include "ModelDispatcher.h"
#include "mainwindow.h"

ModelDispatcher::ModelDispatcher(MainWindow* main_window)
    :main_window{main_window}
{

}

void ModelDispatcher::addRadarStation(RadarStation const& radar_station)
{
    main_window->addTreeItem(ItemTypes::RADAR_STATION, radar_station.name);
    radar_stations.push_back(radar_station);
}

void ModelDispatcher::deleteRadarStation(std::string name)
{
    for(auto i = 0; i < radar_stations.size(); i++)
    {
        if(radar_stations[i].name == name)
        {
            radar_stations.erase(radar_stations.begin()+i);
            break;
        }

    }

}

void ModelDispatcher::addControlCenter(ControlCenter const&control_center)
{
        main_window->addTreeItem(ItemTypes::CONTROL_CENTER, control_center.name);
        control_centers.push_back(control_center);
}

void ModelDispatcher::deleteControlCenter(std::string name)
{
    for(uint64_t i = 0; i < control_centers.size(); i++)
    {
        if(control_centers[i].name == name)
        {
            control_centers.erase(control_centers.begin()+i);
            break;
        }

    }
}

void ModelDispatcher::addLaunchSilo(LaunchSilo const&launch_silo)
{
    main_window->addTreeItem(ItemTypes::LAUNCH_SILO, launch_silo.name);
    launch_silos.push_back(launch_silo);
}

void ModelDispatcher::deleteLaunchSilo(std::string name)
{
    for(auto i = 0; i < launch_silos.size(); i++)
    {
        if(launch_silos[i].name == name)
        {
            launch_silos.erase(launch_silos.begin()+i);
            break;
        }

    }
}

void ModelDispatcher::addTargets(const std::vector<AirObject> &targets)
{
    environment.air_objects.insert(environment.air_objects.end(),targets.begin(),targets.end());
}

void ModelDispatcher::setRadarStations(std::vector<RadarStation> const& new_radar_stations)
{
    main_window->clearTreeItems(ItemTypes::RADAR_STATION);
    for (auto const& station : new_radar_stations)
    {
        main_window->addTreeItem(ItemTypes::RADAR_STATION, station.name);
    }
    radar_stations = new_radar_stations;
}

void ModelDispatcher::setEnvironment(Environment const& new_environment)
{
    environment = new_environment;
}

void ModelDispatcher::setControlCenters(std::vector<ControlCenter> const& new_control_centers)
{
    control_centers.clear();
    main_window->clearTreeItems(ItemTypes::CONTROL_CENTER);
    if (new_control_centers.size() > 0)
    {
        main_window->addTreeItem(ItemTypes::CONTROL_CENTER, new_control_centers[0].name);
        control_centers.push_back(new_control_centers[0]);
    }
}

void ModelDispatcher::setLaunchSilos(std::vector<LaunchSilo> const & new_launch_silos)
{
    main_window->clearTreeItems(ItemTypes::LAUNCH_SILO);
    for (auto const& silo : new_launch_silos)
    {
        main_window->addTreeItem(ItemTypes::LAUNCH_SILO, silo.name);
    }
    launch_silos = new_launch_silos;
}

void ModelDispatcher::run()
{
    paused = false;
    main_window->readSimParameters(SimParameters::SIMULATION_TIME, SimParameters::TIME_MULTIPLIER, SimParameters::TIME_STEP);
    SimParameters::simulation_tick = 0;
    QElapsedTimer timer;
    timer.start();
    double last_time = 0.0;
    double current_time = 0.0;
    double elapsed = 0.0;
    double remaining_simulation_time = SimParameters::SIMULATION_TIME;
    double time_since_last_update = 0.0;

    for (auto& center : control_centers)
    {
        center.silo_missiles.clear();
        for (auto const& silo : launch_silos)
        {
            center.silo_missiles.push_back(silo.missiles[0]);
        }
    }

    while (remaining_simulation_time > 0.0)
    {
        current_time = timer.elapsed() / 1000.0;
        elapsed = current_time - last_time;
        last_time = current_time;
        if (!paused)
        {
            remaining_simulation_time -= elapsed * SimParameters::TIME_MULTIPLIER;
            time_since_last_update += elapsed * SimParameters::TIME_MULTIPLIER;
            if (time_since_last_update > SimParameters::TIME_STEP)
            {
                time_since_last_update -= SimParameters::TIME_STEP;
                update();
                SimParameters::simulation_tick++;
            }
        }
    }
    writeLog();
}

void ModelDispatcher::update()
{
    fillEnvironment();
    environment.update();

    fillRadarStations();
    fillControlCenter();
    fillLaunchSilos();

    for (auto& station : radar_stations)
    {
        station.update();
    }
    for (auto& center : control_centers)
    {
        center.update();
    }
    for (auto& silo : launch_silos)
    {
        silo.update();
    }
}

void ModelDispatcher::fillRadarStations()
{
    std::vector<Target> environment_targets;
    for (auto const& target : environment.air_objects)
    {
        environment_targets.emplace_back(Target{target.current_position, target.direction, target.speed});
    }
    for (auto& station : radar_stations)
    {
        station.environment_targets = environment_targets;
        station.orders.clear();
    }

    for (auto& center : control_centers)
    {
        for (uint64_t i = 0; i < center.radar_orders.size(); i++)
        {
            radar_stations[center.radar_orders[i].radar_id].orders.push_back(center.radar_orders[i]);
        }
    }
}

void ModelDispatcher::fillControlCenter()
{
    for (auto& center : control_centers)
    {
        center.radar_targets.clear();
        center.radar_channels.clear();
        for (auto const& radar : radar_stations)
        {
            center.radar_targets.push_back(radar.visible_targets);
            center.radar_channels.push_back(radar.radar.max_track_targets);
        }
        center.silo_positions.clear();
        for (auto const& silo : launch_silos)
        {
            center.silo_positions.push_back(silo.position);
        }
    }
}

void ModelDispatcher::fillLaunchSilos()
{
    for (auto& silo : launch_silos)
    {
        silo.orders.clear();
    }
    for (auto& center : control_centers)
    {
        for (uint64_t i = 0; i < center.launch_silo_orders.size(); i++)
        {
            launch_silos[center.launch_silo_orders[i].silo_id].orders.push_back(center.launch_silo_orders[i]);
        }
    }
}

void ModelDispatcher::fillEnvironment()
{
    for (auto& silo : launch_silos)
    {
        for (auto& missile : silo.launched_missiles)
        {
            environment.missiles.push_back(missile);
        }
    }
    for (auto& missile : environment.missiles)
    {
        for (auto& center : control_centers)
        {
            if (center.isTargetTracked(missile.target_id))
            {
                auto tracked_target = center.findTargetWithID(missile.target_id).second;
                missile.target_position = tracked_target.position + tracked_target.direction * tracked_target.speed * SimParameters::TIME_STEP;
            }
        }
    }
}

void ModelDispatcher::writeLog()
{
    std::ofstream log_file("log.txt", std::ios::binary);

    auto print_position_vector = [&log_file](std::vector<Position> const& data)
    {
        for (uint64_t i = 0; i < data.size(); i++)
        {
            log_file << data[i].x << " " << data[i].y << " " << data[i].z << " ";
        }
        log_file << "\n";
    };

    log_file << SimParameters::TIME_STEP << "\n";

    log_file << environment.log_targets.size() << "\n";
    for (auto const& log : environment.log_targets)
    {
        print_position_vector(log);
    }

    log_file << radar_stations.size() << "\n";
    for (uint64_t i = 0; i < radar_stations.size(); i++)
    {
        log_file << i << " " << radar_stations[i].log_targets.size() << "\n";
        for (auto const& log : radar_stations[i].log_targets)
        {
            print_position_vector(log);
        }
    }

    log_file << environment.log_missiles.size() << "\n";
    for (auto const& log : environment.log_missiles)
    {
        print_position_vector(log);
    }

    log_file << radar_stations.size() << "\n";
    for (uint64_t i = 0; i < radar_stations.size(); i++)
    {
        log_file << i << " " << radar_stations[i].radar.max_track_targets << "\n";
        for (auto const& log : radar_stations[i].log_load)
        {
            log_file << log << " ";
        }
        log_file << "\n";
    }
}
