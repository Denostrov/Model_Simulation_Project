#pragma once

#include <vector>

#include "auxiliary.h"
#include "RadarStation.h"
#include "Environment.h"
#include "ControlCenter.h"

enum class ItemTypes
{
    RADAR_STATION, CONTROL_CENTER, LAUNCH_SILO
};

class MainWindow;

class ModelDispatcher
{
public:
    explicit ModelDispatcher(MainWindow* main_window);

    ModelDispatcher(ModelDispatcher const& other) = delete;
    ModelDispatcher& operator=(ModelDispatcher const& other) = delete;

    ~ModelDispatcher() = default;

    void run();

    void addRadarStation(RadarStation const& radar_station);
    void addControlCenter(ControlCenter const& control_center);
    void addLaunchSilo(LaunchSilo const& launch_silo);
    void addTargets(std::vector<AirObject> const& targets);

    void setRadarStations(std::vector<RadarStation> const& new_radar_stations);

    void setEnvironment(Environment const& new_environment);
    void setControlCenters(std::vector<ControlCenter> const& new_control_centers);
    void setLaunchSilos(std::vector<LaunchSilo> const& new_launch_silos);

    void deleteRadarStation(std::string name);
    void deleteControlCenter(std::string name);
    void deleteLaunchSilo(std::string name);

    std::vector<RadarStation> radar_stations;
    Environment environment;
    std::vector<ControlCenter> control_centers;
    std::vector<LaunchSilo> launch_silos;

    bool paused = false;

    MainWindow* main_window;

private:
    void update();

    void fillRadarStations();
    void fillControlCenter();
    void fillLaunchSilos();
    void fillEnvironment();

    void writeLog();
};
