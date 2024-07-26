#include "mainwindow.h"
#include <QContextMenuEvent>
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <qmessagebox.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qjsonvalue.h>
#include "weathertool.h"
#include <QPainter>

#define INCREMENT 5 //温度每升高/降低 一度  y轴坐标的增量
#define POINT_REDIUS 3 //曲线描点的大小
#define TEXT_OFFSET_X 12 //X轴偏移
#define TEXT_OFFSET_Y 12 //X轴偏移

#pragma execution_character_set("utf-8")


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);//设置窗口无边框
    setFixedSize(this->width(), this->height()); //设置固定大小
    //构建右键菜单
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/res/close.png"));

    mExitMenu->addAction(mExitAct);

    connect(mExitAct, &QAction::triggered, this, [=]() {
        qApp->exit(0);
        });


    InitWidGet();


    //网络请求
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplied);

    //直接在构造中请求天气数据  
    //getWeatherInfo("101010100");//101010100为北京城市编码
    getWeatherInfo("贵阳");

    connect(ui->btnSearch, &QPushButton::clicked, this, [=]() {
        QString cityName = ui->leCity->text();
        if (!cityName.isEmpty()) {
            getWeatherInfo(cityName);
        }
        else {
            QMessageBox::warning(this, "天气", "请求数据失败", QMessageBox::Ok);
        }
        });

    //给高低温标签添加事件过滤器
    //参数指定为thsi当前窗口对象 
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    //弹出右键菜单
    mExitMenu->exec(QCursor::pos());

    event->accept();//调用accept 表示，这个事件我已经处理，不需要向上传递了

}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    mOffset = event->globalPos() - this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    this->move(event->globalPos() - mOffset);
    this->setCursor(Qt::ClosedHandCursor);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    this->setCursor(Qt::ArrowCursor);
}

void MainWindow::onReplied(QNetworkReply* reply)
{
    qDebug() << "onReplied success";
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    //qDebug() << "operation: " << reply->operation();
    //qDebug() << "status code: " << status_code;
    //qDebug() << "url: " << reply->url();
    //qDebug() << "rawHeader: " << reply->rawHeaderList();

    //判断请求是否成功
    if (reply->error() != QNetworkReply::NoError || status_code != 200) {
        qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this, "天气", "请求数据失败", QMessageBox::Ok);
    }
    else
    {
        //获取响应信息
        QByteArray byteArray = reply->readAll();
        qDebug() << "read all: " << byteArray.data();
        parseJson(byteArray);
        reply->deleteLater();
    }


}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName);
    if (cityCode.isEmpty()){
        QMessageBox::warning(this, "提示", "请检查输入是否正确！", QMessageBox::Ok);
    }

    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    mNetAccessManager->get(QNetworkRequest(url));

}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError* err = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, err);
    if (err->error != QJsonParseError::NoError) {
        return;
    }
    QJsonObject rootObj = doc.object();
    qDebug() << rootObj.value("message").toString();

    if (rootObj.value("status").toInt() != 200) {
        QMessageBox::warning(this, "提示", "天气数据异常！", QMessageBox::Ok);
        return;
    }

    //解析日期和城市
    today.date = rootObj.value("date").toString();
    today.city = rootObj.value("cityInfo").toObject().value("city").toString();

    //解析yesterday
    QJsonObject objData = rootObj.value("data").toObject();
    QJsonObject objYesterday = objData.value("yesterday").toObject();

    sixday[0].date = objYesterday.value("ymd").toString();
    sixday[0].week = objYesterday.value("week").toString();
    sixday[0].type = objYesterday.value("type").toString();

    //高温
    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    sixday[0].high = s.toInt();

    //低温
    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    sixday[0].low = s.toInt();

    //风力风向空气质量
    sixday[0].fx = objYesterday.value("fx").toString();
    sixday[0].fl = objYesterday.value("fl").toString();
    sixday[0].aqi = objYesterday.value("aqi").toDouble();

    //解析forcast中5天的数据
    QJsonArray forecastArr = objData.value("forecast").toArray();
    for (int i = 0; i < 5; i++)
    {
        QJsonObject objForecast = forecastArr[i].toObject();
        sixday[i + 1].week = objForecast.value("week").toString();
        sixday[i + 1].date = objForecast.value("ymd").toString();
        sixday[i + 1].type = objForecast.value("type").toString();
        //高温
        QString s;
        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        sixday[i + 1].high = s.toInt();
        //低温
        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        sixday[i + 1].low = s.toInt();
        //风向、风力、污染指数
        sixday[i + 1].fx = objForecast.value("fx").toString();
        sixday[i + 1].fl = objForecast.value("fl").toString();
        sixday[i + 1].aqi = objForecast.value("aqi").toDouble();
    }

    //解析今天的数据
    today.ganmao = objData.value("ganmao").toString();
    today.wendu = objData.value("wendu").toString();
    today.shidu = objData.value("shidu").toString();
    today.pm25 = objData.value("pm25").toDouble();
    today.quality = objData.value("quality").toString();

    //forecast 中第一个数组元素是今天的数据
    today.type = sixday[1].type;
    today.fx = sixday[1].fx;
    today.fl = sixday[1].fl;
    today.high = sixday[1].high;
    today.low = sixday[1].low;

    //更新UI
    upadateUI();

    //更新曲线
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();

}

void MainWindow::upadateUI()
{
    //更新日期和城市
    ui->lblDate->setText(QDateTime::fromString(today.date, "yyyyMMdd").toString("yyyy/MM/dd") + "   " + sixday[1].week);
    ui->lblCity->setText(today.city);

    //更新今天
    ui->lblTypeIcon->setPixmap(todayTypeMap[today.type]);
    ui->lblTemp->setText(today.wendu);
    ui->lblType->setText(today.type);
    ui->lblLowHigh->setText(QString::number(today.low) + "℃ ~ " + QString::number(today.high) + "℃");
    ui->lblGanMao->setText("注意：" + today.ganmao);
    ui->lblWindFx->setText(today.fx);
    ui->lblWindFl->setText(today.fl);
    ui->lblPM25->setText(QString::number(today.pm25));
    ui->lblShiDu->setText(today.shidu);
    ui->lblQuality->setText(today.quality);

    //更新六天
    for (int i = 0; i < 6; i++)
    {
        //更新日期和时间
        WeekList[i]->setText("周" + sixday[i].week.right(1));
        ui->lblWeek0->setText("昨天");
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");
        QStringList ymdList = sixday[i].date.split("-");
        DateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //更新天气类型
        TypeList[i]->setText(sixday[i].type);
        TypeIconList[i]->setPixmap(todayTypeMap[sixday[i].type]);

        //更新空气质量
        if (sixday[i].aqi >= 0 && sixday[i].aqi <= 50)
        {
            AqlList[i]->setText("优");
            AqlList[i]->setStyleSheet("background-color: rgb(121,184,0);");
        }
        else if (sixday[i].aqi > 50 && sixday[i].aqi <= 100)
        {
            AqlList[i]->setText("良");
            AqlList[i]->setStyleSheet("background-color: rgb(255,178,23);");
        }
        else if (sixday[i].aqi > 100 && sixday[i].aqi <= 150)
        {
            AqlList[i]->setText("轻度");
            AqlList[i]->setStyleSheet("background-color: rgb(255,87,97);");
        }
        else if (sixday[i].aqi > 150 && sixday[i].aqi <= 200)
        {
            AqlList[i]->setText("中度");
            AqlList[i]->setStyleSheet("background-color: rgb(235,17,27);");
        }
        else if (sixday[i].aqi > 200 && sixday[i].aqi <= 250)
        {
            AqlList[i]->setText("重度");
            AqlList[i]->setStyleSheet("background-color: rgb(170,0,0);");
        }
        else
        {
            AqlList[i]->setText("严重");
            AqlList[i]->setStyleSheet("background-color: rgb(110,0,0);");
        }

        //更新风力风向
        FxList[i]->setText(sixday[i].fx);
        FlList[i]->setText(sixday[i].fl);

    }




}

void MainWindow::InitWidGet()
{
    //将控件添加到控件数组
/*星期几*/
    WeekList << ui->lblWeek0 << ui->lblWeek1 << ui->lblWeek2 << ui->lblWeek3 << ui->lblWeek4 << ui->lblWeek5;
    /*日期*/
    DateList << ui->lblDate0 << ui->lblDate1 << ui->lblDate2 << ui->lblDate3 << ui->lblDate4 << ui->lblDate5;
    /*天气*/
    TypeList << ui->lblType0 << ui->lblType1 << ui->lblType2 << ui->lblType3 << ui->lblType4 << ui->lblType5;
    /*天气图标*/
    TypeIconList << ui->lblTypeIcon0 << ui->lblTypeIcon1 << ui->lblTypeIcon2 << ui->lblTypeIcon3 << ui->lblTypeIcon4 << ui->lblTypeIcon5;

    /*天气污染*/
    AqlList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;
    /*风向和风力*/
    FxList << ui->lblFx0 << ui->lblFx1 << ui->lblFx2 << ui->lblFx3 << ui->lblFx4 << ui->lblFx5;
    FlList << ui->lblFl0 << ui->lblFl1 << ui->lblFl2 << ui->lblFl3 << ui->lblFl4 << ui->lblFl5;

    //图标
    todayTypeMap.insert("暴雪", ":/res/type/BaoXue.png");
    todayTypeMap.insert("暴雨", ":/res/type/BaoYu.png");
    todayTypeMap.insert("暴雨到大暴雨", ":/res/type/BaoYuDaoDaBaoYu.png");
    todayTypeMap.insert("大暴雨", ":/res/type/DaBaoYu.png");
    todayTypeMap.insert("大暴雨到特大暴雨", ":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    todayTypeMap.insert("大到暴雪", ":/res/type/DaDaoBaoXue.png");
    todayTypeMap.insert("大到暴雨", ":/res/type/DaDaoBaoYu.png");
    todayTypeMap.insert("大雪", ":/res/type/DaXue.png");
    todayTypeMap.insert("大雨", ":/res/type/DaYu.png");
    todayTypeMap.insert("冻雨", ":/res/type/DongYu.png");
    todayTypeMap.insert("多云", ":/res/type/DuoYun.png");
    todayTypeMap.insert("浮沉", ":/res/type/FuChen.png");
    todayTypeMap.insert("雷阵雨", ":/res/type/LeiZhenYu.png");
    todayTypeMap.insert("雷阵雨伴有冰雹", ":/res/type/LeiZhenYuBanYouBingBao.png");
    todayTypeMap.insert("霾", ":/res/type/Mai.png");
    todayTypeMap.insert("强沙城暴", ":/res/type/QiangShaChenBao.png");
    todayTypeMap.insert("晴", ":/res/type/Qing.png");
    todayTypeMap.insert("沙城暴", ":/res/type/ShaChenBao.png");
    todayTypeMap.insert("特大暴雨", ":/res/type/TeDaBaoYu.png");
    todayTypeMap.insert("undefined", ":/res/type/undefined.png");
    todayTypeMap.insert("雾", ":/res/type/Wu.png");
    todayTypeMap.insert("小到中雪", ":/res/type/XiaoDaoZhongXue.png");
    todayTypeMap.insert("小到中雨", ":/res/type/XiaoDaoZhongYu.png");
    todayTypeMap.insert("小雪", ":/res/type/XiaoXue.png");
    todayTypeMap.insert("小雨", ":/res/type/XiaoYu.png");
    todayTypeMap.insert("雪", ":/res/type/Xue.png");
    todayTypeMap.insert("扬沙", ":/res/type/YangSha.png");
    todayTypeMap.insert("阴", ":/res/type/Yin.png");
    todayTypeMap.insert("雨", ":/res/type/Yu.png");
    todayTypeMap.insert("雨夹雪", ":/res/type/YuJiaXue.png");
    todayTypeMap.insert("阵雪", ":/res/type/ZhenXue.png");
    todayTypeMap.insert("阵雨", ":/res/type/ZhenYu.png");
    todayTypeMap.insert("中到大雪", ":/res/type/ZhongDaoDaXue.png");
    todayTypeMap.insert("中到大雨", ":/res/type/ZhongDaoDaYu.png");
    todayTypeMap.insert("中雪", ":/res/type/ZhongXue.png");
    todayTypeMap.insert("中雨", ":/res/type/ZhongYu.png");

}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->lblHighCurve && event->type() == QEvent::Paint) {
        painHighCurve();
    }
    if (watched == ui->lblLowCurve && event->type() == QEvent::Paint) {
        painLowCurve();
    }
    return QWidget::eventFilter(watched, event);
}


void MainWindow::paint(QString HighOrLow)
{
    QLabel* Now = nullptr; // 当前要绘制的曲线标签
    QColor color; // 曲线的颜色
    if (HighOrLow == "High")
    {
        Now = ui->lblHighCurve; // 获取高温曲线标签
        color = QColor(255, 170, 0); // 设置高温曲线颜色为橙色
    }
    else if (HighOrLow == "Low")
    {
        Now = ui->lblLowCurve; // 获取低温曲线标签
        color = QColor(0, 255, 255); // 设置低温曲线颜色为青色
    }
    else
    {
        return; // 如果不是"High"或"Low"，则直接返回
    }

    QPainter painter(Now); // 创建一个绘图对象，绘制在当前曲线标签上
    painter.setRenderHint(QPainter::Antialiasing, true); // 设置抗锯齿

    int pointX[6] = { 0 }; // 存储X坐标
    int tempSum = 0; // 温度总和
    int tempAverage = 0; // 温度平均值
    int value;

    // 获取X坐标
    for (int i = 0; i < 6; ++i) {
        pointX[i] = WeekList[i]->pos().x() + WeekList[i]->width() / 2 - 6;
        value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        tempSum += value;
    }
    tempAverage = tempSum / 6; // 计算温度平均值

    int pointY[6] = { 0 }; // 存储Y坐标
    int yCenter = Now->height() / 2; // Y轴中心坐标

    // 计算Y轴坐标
    for (int i = 0; i < 6; ++i) {
        value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        pointY[i] = yCenter - ((value - tempAverage) * INCREMENT);
    }

    QPen pen = painter.pen(); // 获取当前画笔
    pen.setWidth(1); // 设置画笔宽度
    pen.setColor(color); // 设置画笔颜色
    painter.setPen(pen); // 设置画笔

    painter.setBrush(color); // 设置画刷颜色

    // 绘制点和温度文本
    for (int i = 0; i < 6; ++i) {
        painter.drawEllipse(QPoint(pointX[i], pointY[i]), POINT_REDIUS, POINT_REDIUS); // 绘制点
        int value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        painter.drawText(pointX[i] - TEXT_OFFSET_X, pointY[i] - TEXT_OFFSET_Y, QString::number(value) + "℃"); // 绘制温度文本
    }

    // 绘制连线
    for (int i = 0; i < 5; ++i) {
        pen.setStyle(i < 2 ? Qt::SolidLine : Qt::DotLine); // 设置画笔样式
        painter.setPen(pen); // 设置画笔
        painter.drawLine(pointX[i], pointY[i], pointX[i + 1], pointY[i + 1]); // 绘制线段
    }
}


void MainWindow::painHighCurve()
{
    paint("High");
}

void MainWindow::painLowCurve()
{
    paint("Low");
}


