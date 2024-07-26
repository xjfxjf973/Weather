#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <qmenu.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <weatherdata.h>
#include <qlist.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void contextMenuEvent(QContextMenuEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void onReplied(QNetworkReply* reply);

    void getWeatherInfo(QString cityName);

    void parseJson(QByteArray &byteArray);

    void upadateUI();

    void InitWidGet(); // �ؼ���ʼ��

    //��д�����eventfileter����
    bool eventFilter(QObject* watched, QEvent* event);

    //���Ƹߵ�������
    void painHighCurve();
    void painLowCurve();



private:
    Ui::MainWindow* ui;

    QMenu* mExitMenu;//�Ҽ��˳��˵�
    QAction* mExitAct;//�˵���-�˳�
    QPoint mOffset;//ƫ��ֵ

    QNetworkAccessManager* mNetAccessManager; // �������������

    Today today; // ������������
    SixDay sixday[6]; // ������������

    QList<QLabel*> WeekList; // �����б�
    QList<QLabel*> DateList; // �����б�

    QList<QLabel*> TypeList; // ���������б�
    QList<QLabel*> TypeIconList; // ����ͼ���б�

    QList<QLabel*> AqlList; // ������Ⱦָ���б�

    QList<QLabel*> FxList; // �����б�
    QList<QLabel*> FlList; // �����б�

    QMap<QString, QString> todayTypeMap; // ������������ӳ���

    void paint(QString HighOrLow);

};