#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), simulation_thread{nullptr}, scene_chart{nullptr}
{
    ui->setupUi(this);
//    Form_Add.show();
    connect(&Form_Add,&add::pushAddRS, this, &MainWindow::reactionAddRS);
    connect(&Form_Add,&add::pushAddCC, this, &MainWindow::reactionAddCC);
    connect(&Form_Add,&add::pushAddLS, this, &MainWindow::reactionAddLS);
    connect(&Form_Add,&add::pushAddT, this, &MainWindow::reactionAddT);

    model_dispatcher = new ModelDispatcher(this);

    scene_update_timer = new QTimer();
    connect(scene_update_timer, &QTimer::timeout, this, &MainWindow::updateGraphics);
    scene_update_timer->start(SimParameters::TIME_STEP * 1000);

    setupTreeWidget();
    setupGraph();
    drawModel();
}

MainWindow::~MainWindow()
{
    delete ui;

    scene_update_timer->stop();
    delete scene_update_timer;
    delete model_dispatcher;
}

void MainWindow::addTreeItem(ItemTypes type, const std::string& name)
{
    QTreeWidgetItem* insert_target = nullptr;
    switch (type)
    {
    case ItemTypes::RADAR_STATION:
        insert_target = ui->treeWidget->findItems("Radar Stations", Qt::MatchFlag::MatchExactly)[0];
        break;
    case ItemTypes::LAUNCH_SILO:
        insert_target = ui->treeWidget->findItems("Launch Silos", Qt::MatchFlag::MatchExactly)[0];
        break;
    case ItemTypes::CONTROL_CENTER:
        insert_target = ui->treeWidget->findItems("Control Centers", Qt::MatchFlag::MatchExactly)[0];
        break;
    default:
        break;
    }
    insert_target->addChild(new QTreeWidgetItem(QStringList(QString(name.c_str()))));
    drawModel();
}

void MainWindow::clearTreeItems(ItemTypes type)
{
    QTreeWidgetItem* insert_target = nullptr;
    switch (type)
    {
    case ItemTypes::RADAR_STATION:
        insert_target = ui->treeWidget->findItems("Radar Stations", Qt::MatchFlag::MatchExactly)[0];
        break;
    case ItemTypes::LAUNCH_SILO:
        insert_target = ui->treeWidget->findItems("Launch Silos", Qt::MatchFlag::MatchExactly)[0];
        break;
    case ItemTypes::CONTROL_CENTER:
        insert_target = ui->treeWidget->findItems("Control Centers", Qt::MatchFlag::MatchExactly)[0];
        break;
    default:
        break;
    }
    auto child_count = insert_target->childCount();
    for (auto i = 0; i < child_count; i++)
    {
        auto child = insert_target->child(0);
        insert_target->removeChild(child);
        delete child;
    }
}

void MainWindow::setupTreeWidget()
{
    QTreeWidgetItem* radar_stations = new QTreeWidgetItem(ui->treeWidget);
    radar_stations->setText(0, "Radar Stations");
    QTreeWidgetItem* control_centers = new QTreeWidgetItem(ui->treeWidget);
    control_centers->setText(0, "Control Centers");
    QTreeWidgetItem* launch_silos = new QTreeWidgetItem(ui->treeWidget);
    launch_silos->setText(0, "Launch Silos");
}

void MainWindow::setupGraph()
{
    auto graphics_scene = new QGraphicsScene{nullptr};

    scene_chart = new QChart();
    scene_chart->legend()->hide();
    scene_chart->resize(800, 800);
    scene_chart->setTitle("Environment");

    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("X, km");
    axisX->setRange(-100.0, 100.0);
    axisX->setTickCount(11);
    axisX->setMinorTickCount(1);

    QValueAxis* axisZ = new QValueAxis();
    axisZ->setTitleText("Z, km");
    axisZ->setRange(-100.0, 100.0);
    axisZ->setTickCount(11);
    axisZ->setMinorTickCount(1);

    scene_chart->addAxis(axisX, Qt::AlignLeft);
    scene_chart->addAxis(axisZ, Qt::AlignBottom);

    auto configure_scatter_series = [this, &axisX, &axisZ](QScatterSeries*& series, std::string const& name, std::string const& texture_path,
            double size)
    {
        series = new QScatterSeries();
        series->setName(name.c_str());
        QImage texture(texture_path.c_str());
        series->setBrush(texture);
        series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
        series->setMarkerSize(size);
        scene_chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisZ);
    };

    configure_scatter_series(radar_series, "radar", "Resources/radar.png", 32.0);
    configure_scatter_series(target_series, "target", "Resources/target.png", 8.0);
    configure_scatter_series(control_center_series, "control center", "Resources/control_center.png", 32.0);
    configure_scatter_series(launch_silo_series, "launch silo", "Resources/launch_silo.png", 32.0);
    configure_scatter_series(missile_series, "missile", "Resources/missile.png", 16.0);

    graphics_scene->addItem(scene_chart);

    ui->graphicsView->setScene(graphics_scene);
    ui->graphicsView->centerOn(scene_chart);
}

void MainWindow::drawModel()
{   
    //remove radar range graphics
    for (auto serial : radar_ranges)
    {
        scene_chart->removeSeries(serial);
        delete serial;
    }
    radar_ranges.clear();

    //remove radar sector graphics
    for (auto serial : radar_sectors)
    {
        scene_chart->removeSeries(serial);
        delete serial;
    }
    radar_sectors.clear();

    radar_series->clear();
    for (auto const& station : model_dispatcher->radar_stations)
    {
        radar_series->append(station.position.z, station.position.x);
        auto const& radar = station.radar;
        //add range circle
        QPieSeries* range = new QPieSeries();
        QPieSlice* slice = new QPieSlice("range", 1.0);
        slice->setColor(Qt::darkRed);
        range->append(slice);
        auto pie_size = rangeToPieSize(radar.range);
        range->setPieSize(pie_size);
        range->setHoleSize(pie_size * 0.985);
        range->setHorizontalPosition(positionToPiePosition(station.position.z));
        range->setVerticalPosition(1.0 - positionToPiePosition(station.position.x));
        scene_chart->addSeries(range);
        radar_ranges.push_back(range);

        if (radar.azimuth_sector_size < 360.0)
        {
            QLineSeries* sector = new QLineSeries();
            sector->setColor(Qt::red);
            sector->append(std::cos(degToRad(90.0 + radar.direction - radar.azimuth_sector_size / 2.0)) * radar.range + station.position.z,
                           std::sin(degToRad(90.0 + radar.direction - radar.azimuth_sector_size / 2.0)) * radar.range + station.position.x);
            sector->append(station.position.z, station.position.x);
            sector->append(std::cos(degToRad(90.0 + radar.direction + radar.azimuth_sector_size / 2.0)) * radar.range + station.position.z,
                           std::sin(degToRad(90.0 + radar.direction + radar.azimuth_sector_size / 2.0)) * radar.range + station.position.x);
            scene_chart->addSeries(sector);
            radar_sectors.push_back(sector);
            auto axes = scene_chart->axes();
            for (auto axis : axes)
            {
                sector->attachAxis(axis);
            }
        }

        QLineSeries* scanbeam = new QLineSeries();
        scanbeam->setColor(Qt::green);
        scanbeam->append(station.position.z, station.position.x);
        scanbeam->append(std::cos(degToRad(90.0 + radar.scanbeam_direction)) * radar.range + station.position.z,
                         std::sin(degToRad(90.0 + radar.scanbeam_direction)) * radar.range + station.position.x);
        scene_chart->addSeries(scanbeam);
        radar_sectors.push_back(scanbeam);
        auto axes = scene_chart->axes();
        for (auto axis : axes)
        {
            scanbeam->attachAxis(axis);
        }

    }

    target_series->clear();
    for (auto const& target : model_dispatcher->environment.air_objects)
    {
        target_series->append(target.current_position.z, target.current_position.x);
    }

    control_center_series->clear();
    for (auto const& center : model_dispatcher->control_centers)
    {
        control_center_series->append(center.position.z, center.position.x);
    }

    launch_silo_series->clear();
    missile_series->clear();
    for (auto const& silo : model_dispatcher->launch_silos)
    {
        launch_silo_series->append(silo.position.z, silo.position.x);
        uint64_t total_missile_count = 0;
        for (auto count : silo.missiles)
        {
            total_missile_count += count;
        }
        for (uint64_t i = 0; i < total_missile_count; i++)
        {
            double z_offset = -spriteSizeToPosition(8.0) + spriteSizeToPosition(16.0) * (i % 2);
            double x_offset = -spriteSizeToPosition(24.0) - spriteSizeToPosition(16.0) * (i / 2);
            missile_series->append(silo.position.z + z_offset, silo.position.x + x_offset);
        }
    }
    for (auto const& missile : model_dispatcher->environment.missiles)
    {
        missile_series->append(missile.current_position.z, missile.current_position.x);
    }
}

void MainWindow::updateGraphics()
{
    drawModel();
}

void MainWindow::on_actionRun_triggered()
{
    if (!simulation_thread || simulation_thread->isFinished())
    {
        delete simulation_thread;
        auto simulate = std::mem_fn(&ModelDispatcher::run);
        simulation_thread = QThread::create(simulate, model_dispatcher);
        simulation_thread->start();
    }
}


void MainWindow::on_actionOpen_triggered()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open save file", "", "*.sv");
    readFile(filename.toStdString(), *model_dispatcher);
}



void MainWindow::reactionAddRS(QVector<double> arr)
{
    static uint32_t radar_stations_created = 0;
    Radar radar = Radar(0.0, 40.0, 1.0, 10.0, 60.0, 80.0, 4);
    RadarStation station(radar, {arr[0],arr[1], arr[2]}, "Radar Station " + std::to_string(radar_stations_created));
    model_dispatcher->addRadarStation(station);
    radar_stations_created++;
}

void MainWindow::reactionAddCC(QVector<double> arr)
{
    if (model_dispatcher->control_centers.size()>=1) return;
    static uint32_t control_center_created = 0;
    ControlCenter control_center = ControlCenter({arr[0],arr[1], arr[2]}, "Control Center " + std::to_string(control_center_created));
    model_dispatcher->addControlCenter(control_center);
    control_center_created++;
}

void MainWindow::reactionAddLS(QVector<double> arr, uint64_t m)
{
    static uint32_t launch_silo_created = 0;
    std::vector<uint64_t> missiles{m};
    LaunchSilo launch_silo = LaunchSilo({arr[0],arr[1], arr[2]}, "Launch Silo " + std::to_string(launch_silo_created), missiles);
    model_dispatcher->addLaunchSilo(launch_silo);
    launch_silo_created++;
}

void MainWindow::reactionAddT(QVector<Position> arr_pos, QVector<double> arr)
{
    auto pos = arr_pos.toStdVector();
    model_dispatcher->addTargets({{pos,arr[0],arr[1]}});
}


void MainWindow::on_actionAdd_triggered()
{
//    Form_Add=new add(this);
//    Form_Add->show();
//    connect(Form_Add,&add::pushAddRS, this, &MainWindow::reactionAddRS);
//    connect(Form_Add,&add::pushAddCC, this, &MainWindow::reactionAddCC);
//    connect(Form_Add,&add::pushAddLS, this, &MainWindow::reactionAddLS);
//    connect(Form_Add,&add::pushAddT, this, &MainWindow::reactionAddT);
    Form_Add.show();
//    Form_Add.get_add();
}


void MainWindow::on_actionClear_triggered()
{
    clearTreeItems(ItemTypes::CONTROL_CENTER);
    clearTreeItems(ItemTypes::LAUNCH_SILO);
    clearTreeItems(ItemTypes::RADAR_STATION);
    delete model_dispatcher;
    model_dispatcher = new ModelDispatcher(this);
}

void MainWindow::readSimParameters(double &time, double &speed, double &step)
{
    time = ui->simTime->text().toDouble();
    speed = ui->simSpeed->text().toDouble();
    step = ui->simStep->text().toDouble();
}

void MainWindow::on_actionStop_triggered()
{
    model_dispatcher->paused = !model_dispatcher->paused;
}


void MainWindow::on_actionPause_triggered()
{
    model_dispatcher->paused = !model_dispatcher->paused;
}


void MainWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    if (!item || !item->parent()) return;

    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(on_actionDelete_triggered()));

    ui->treeWidget->setCurrentItem(item);
    QMenu *menu = new QMenu(this);
    menu->addAction(ui->actionDelete);

    menu->popup(ui->treeWidget->mapToGlobal(pos));
}


void MainWindow::on_actionDelete_triggered()
{
    QTreeWidgetItem *clickedItem = ui->treeWidget->currentItem();
    if(clickedItem->parent())
    {

        QTreeWidgetItem *insert_target = nullptr;
        ItemTypes type;
        std::string str = clickedItem->text(0).toStdString();
        if (str.find("Radar Station ")!= std::string::npos)
        {
            type=ItemTypes::RADAR_STATION;
            model_dispatcher->deleteRadarStation(clickedItem->text(0).toStdString());
        }
        if (str.find("Launch Silo ")!= std::string::npos)
        {
            type = ItemTypes::LAUNCH_SILO;
            model_dispatcher->deleteLaunchSilo(clickedItem->text(0).toStdString());
        }
        if(str.find("Control Center ")!= std::string::npos)
        {
            type = ItemTypes::CONTROL_CENTER;
            model_dispatcher->deleteControlCenter(clickedItem->text(0).toStdString());
        }
        switch (type)
        {
        case ItemTypes::RADAR_STATION:
            insert_target = ui->treeWidget->findItems("Radar Stations", Qt::MatchFlag::MatchExactly)[0];
            break;
        case ItemTypes::LAUNCH_SILO:
            insert_target = ui->treeWidget->findItems("Launch Silos", Qt::MatchFlag::MatchExactly)[0];
            break;
        case ItemTypes::CONTROL_CENTER:
            insert_target = ui->treeWidget->findItems("Control Centers", Qt::MatchFlag::MatchExactly)[0];
            break;
        default:
            break;
        }

        insert_target->removeChild(clickedItem);
        delete clickedItem;
        ui->treeWidget->setCurrentItem(insert_target);
    }
    drawModel();
}
