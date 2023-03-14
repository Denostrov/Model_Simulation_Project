#include "add.h"
#include "ui_add.h"

add::add(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::add)
{
    ui->setupUi(this);

    ui->LE_RS_X->setText("");
    ui->LE_RS_Y->setText("0");
    ui->LE_RS_Z->setText("");

    ui->LE_LS_X->setText("");
    ui->LE_LS_Y->setText("0");
    ui->LE_LS_Z->setText("");

    ui->LE_CC_X->setText("");
    ui->LE_CC_Y->setText("0");
    ui->LE_CC_Z->setText("");

    ui->LE_T_X->setText("");
    ui->LE_T_Y->setText("");
    ui->LE_T_Z->setText("");
    ui->LE_T_Load->setText("");
    ui->LE_T_Vel->setText("");

}

add::~add()
{
    delete ui;
}

void add::on_PB_RS_clicked()
{
    QVector<double> arr_rs;
    arr_rs.clear();
    arr_rs.resize(3);
    arr_rs[0]=ui->LE_RS_X->text().toDouble();
    arr_rs[1]=ui->LE_RS_Y->text().toDouble();
    arr_rs[2]=ui->LE_RS_Z->text().toDouble();
    emit pushAddRS(arr_rs);

}


void add::on_PB_CC_clicked()
{
    QVector<double> arr_cc;
    arr_cc.clear();
    arr_cc.resize(3);
    arr_cc[0]=ui->LE_CC_X->text().toDouble();
    arr_cc[1]=ui->LE_CC_Y->text().toDouble();
    arr_cc[2]=ui->LE_CC_Z->text().toDouble();
    emit pushAddCC(arr_cc);

}


void add::on_PB_LS_clicked()
{
    QVector<double> arr_ls;
    uint64_t m;
    arr_ls.clear();
    arr_ls.resize(3);
    arr_ls[0]=ui->LE_LS_X->text().toDouble();
    arr_ls[1]=ui->LE_LS_Y->text().toDouble();
    arr_ls[2]=ui->LE_LS_Z->text().toDouble();
    m=ui->LE_LS_M->text().toInt();
    emit pushAddLS(arr_ls, m);
}


void add::on_PB_T_clicked()
{
    QVector<Position> arr_pos;
    QVector<double> arr_t;
    QVector<QString> arr_s;

    arr_pos.clear();
    arr_t.clear();

    arr_t.resize(2);
    arr_s.resize(3);

    arr_s[0]=ui->LE_T_X->text();
    arr_s[1]=ui->LE_T_Y->text();
    arr_s[2]=ui->LE_T_Z->text();



    QStringList path_x = arr_s[0].split(" ");
    QStringList path_y = arr_s[1].split(" ");
    QStringList path_z = arr_s[2].split(" ");

    for(int i=0; i<std::min({path_x.size(),path_y.size(),path_z.size()}); i++)
    {
        arr_pos.append({path_x[i].toDouble(),path_y[i].toDouble(),path_z[i].toDouble()});

    }

    arr_t[0]=ui->LE_T_Load->text().toDouble();
    arr_t[1]=ui->LE_T_Vel->text().toDouble();

    emit pushAddT(arr_pos, arr_t);
}

//void add::get_add()
//{

//}



