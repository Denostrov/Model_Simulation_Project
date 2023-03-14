#ifndef ADD_H
#define ADD_H

#include <QWidget>
#include <QVector>

#include "ModelDispatcher.h"


namespace Ui {
class add;
}

class add : public QWidget
{
    Q_OBJECT

public:
    explicit add(QWidget *parent = nullptr);
    ~add();

//    void get_add();

signals:
    void pushAddRS(QVector<double> arr);

    void pushAddCC(QVector<double> arr);

    void pushAddLS(QVector<double> arr, uint64_t m);

    void pushAddT(QVector<Position> arr_pos, QVector<double> arr);


private slots:
    void on_PB_RS_clicked();

    void on_PB_CC_clicked();

    void on_PB_LS_clicked();

    void on_PB_T_clicked();



private:
    Ui::add *ui;

    ModelDispatcher* model_dispatcher;





};

#endif // ADD_H
