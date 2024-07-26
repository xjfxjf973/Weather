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

    void InitWidGet(); // 控件初始化

    //重写父类的eventfileter方法
    bool eventFilter(QObject* watched, QEvent* event);

    //绘制高低温曲线
    void painHighCurve();
    void painLowCurve();



private:
    Ui::MainWindow* ui;

    QMenu* mExitMenu;//右键退出菜单
    QAction* mExitAct;//菜单项-退出
    QPoint mOffset;//偏移值

    QNetworkAccessManager* mNetAccessManager; // 网络请求管理器

    Today today; // 当天天气数据
    SixDay sixday[6]; // 六天天气数据

    QList<QLabel*> WeekList; // 星期列表
    QList<QLabel*> DateList; // 日期列表

    QList<QLabel*> TypeList; // 天气类型列表
    QList<QLabel*> TypeIconList; // 天气图标列表

    QList<QLabel*> AqlList; // 天气污染指数列表

    QList<QLabel*> FxList; // 风向列表
    QList<QLabel*> FlList; // 风力列表

    QMap<QString, QString> todayTypeMap; // 当天天气类型映射表

    void paint(QString HighOrLow);

};