#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QGraphicsScene>
#include <QtCharts>
#include <thread>
#include <functional>

#include "ModelDispatcher.h"
#include "Input.h"
#include "add.h"

using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addTreeItem(ItemTypes type, std::string const& name);

    void clearTreeItems(ItemTypes type);

    void readSimParameters(double& time, double& speed, double& step);

    //clear model drawings and redraw it
    void drawModel();

public slots:
    void updateGraphics();

private slots:

    void on_actionRun_triggered();

    void on_actionOpen_triggered();

    void reactionAddRS(QVector<double> arr);
    void reactionAddCC(QVector<double> arr);
    void reactionAddLS(QVector<double> arr, uint64_t m);
    void reactionAddT(QVector<Position> arr_pos, QVector<double> arr);

    void on_actionAdd_triggered();

    void on_actionClear_triggered();

    void on_actionStop_triggered();

    void on_actionPause_triggered();

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionDelete_triggered();


private:
    //create entries for object insertion
    void setupTreeWidget();

    //create grid for graphics scene
    void setupGraph();

    Ui::MainWindow *ui;

    //    add* Form_Add;
    add Form_Add;
    QThread* simulation_thread;
    ModelDispatcher* model_dispatcher;
    QTimer* scene_update_timer;
    QChart* scene_chart;
    QScatterSeries* radar_series;
    QScatterSeries* target_series;
    QScatterSeries* control_center_series;
    QScatterSeries* launch_silo_series;
    QScatterSeries* missile_series;
    std::vector<QPieSeries*> radar_ranges;
    std::vector<QLineSeries*> radar_sectors;

};
#endif // MAINWINDOW_H
