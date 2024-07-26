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

#define INCREMENT 5 //�¶�ÿ����/���� һ��  y�����������
#define POINT_REDIUS 3 //�������Ĵ�С
#define TEXT_OFFSET_X 12 //X��ƫ��
#define TEXT_OFFSET_Y 12 //X��ƫ��

#pragma execution_character_set("utf-8")


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);//���ô����ޱ߿�
    setFixedSize(this->width(), this->height()); //���ù̶���С
    //�����Ҽ��˵�
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText("�˳�");
    mExitAct->setIcon(QIcon(":/res/close.png"));

    mExitMenu->addAction(mExitAct);

    connect(mExitAct, &QAction::triggered, this, [=]() {
        qApp->exit(0);
        });


    InitWidGet();


    //��������
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplied);

    //ֱ���ڹ�����������������  
    //getWeatherInfo("101010100");//101010100Ϊ�������б���
    getWeatherInfo("����");

    connect(ui->btnSearch, &QPushButton::clicked, this, [=]() {
        QString cityName = ui->leCity->text();
        if (!cityName.isEmpty()) {
            getWeatherInfo(cityName);
        }
        else {
            QMessageBox::warning(this, "����", "��������ʧ��", QMessageBox::Ok);
        }
        });

    //���ߵ��±�ǩ����¼�������
    //����ָ��Ϊthsi��ǰ���ڶ��� 
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    //�����Ҽ��˵�
    mExitMenu->exec(QCursor::pos());

    event->accept();//����accept ��ʾ������¼����Ѿ���������Ҫ���ϴ�����

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

    //�ж������Ƿ�ɹ�
    if (reply->error() != QNetworkReply::NoError || status_code != 200) {
        qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this, "����", "��������ʧ��", QMessageBox::Ok);
    }
    else
    {
        //��ȡ��Ӧ��Ϣ
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
        QMessageBox::warning(this, "��ʾ", "���������Ƿ���ȷ��", QMessageBox::Ok);
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
        QMessageBox::warning(this, "��ʾ", "���������쳣��", QMessageBox::Ok);
        return;
    }

    //�������ںͳ���
    today.date = rootObj.value("date").toString();
    today.city = rootObj.value("cityInfo").toObject().value("city").toString();

    //����yesterday
    QJsonObject objData = rootObj.value("data").toObject();
    QJsonObject objYesterday = objData.value("yesterday").toObject();

    sixday[0].date = objYesterday.value("ymd").toString();
    sixday[0].week = objYesterday.value("week").toString();
    sixday[0].type = objYesterday.value("type").toString();

    //����
    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    sixday[0].high = s.toInt();

    //����
    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    sixday[0].low = s.toInt();

    //���������������
    sixday[0].fx = objYesterday.value("fx").toString();
    sixday[0].fl = objYesterday.value("fl").toString();
    sixday[0].aqi = objYesterday.value("aqi").toDouble();

    //����forcast��5�������
    QJsonArray forecastArr = objData.value("forecast").toArray();
    for (int i = 0; i < 5; i++)
    {
        QJsonObject objForecast = forecastArr[i].toObject();
        sixday[i + 1].week = objForecast.value("week").toString();
        sixday[i + 1].date = objForecast.value("ymd").toString();
        sixday[i + 1].type = objForecast.value("type").toString();
        //����
        QString s;
        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        sixday[i + 1].high = s.toInt();
        //����
        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        sixday[i + 1].low = s.toInt();
        //���򡢷�������Ⱦָ��
        sixday[i + 1].fx = objForecast.value("fx").toString();
        sixday[i + 1].fl = objForecast.value("fl").toString();
        sixday[i + 1].aqi = objForecast.value("aqi").toDouble();
    }

    //�������������
    today.ganmao = objData.value("ganmao").toString();
    today.wendu = objData.value("wendu").toString();
    today.shidu = objData.value("shidu").toString();
    today.pm25 = objData.value("pm25").toDouble();
    today.quality = objData.value("quality").toString();

    //forecast �е�һ������Ԫ���ǽ��������
    today.type = sixday[1].type;
    today.fx = sixday[1].fx;
    today.fl = sixday[1].fl;
    today.high = sixday[1].high;
    today.low = sixday[1].low;

    //����UI
    upadateUI();

    //��������
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();

}

void MainWindow::upadateUI()
{
    //�������ںͳ���
    ui->lblDate->setText(QDateTime::fromString(today.date, "yyyyMMdd").toString("yyyy/MM/dd") + "   " + sixday[1].week);
    ui->lblCity->setText(today.city);

    //���½���
    ui->lblTypeIcon->setPixmap(todayTypeMap[today.type]);
    ui->lblTemp->setText(today.wendu);
    ui->lblType->setText(today.type);
    ui->lblLowHigh->setText(QString::number(today.low) + "�� ~ " + QString::number(today.high) + "��");
    ui->lblGanMao->setText("ע�⣺" + today.ganmao);
    ui->lblWindFx->setText(today.fx);
    ui->lblWindFl->setText(today.fl);
    ui->lblPM25->setText(QString::number(today.pm25));
    ui->lblShiDu->setText(today.shidu);
    ui->lblQuality->setText(today.quality);

    //��������
    for (int i = 0; i < 6; i++)
    {
        //�������ں�ʱ��
        WeekList[i]->setText("��" + sixday[i].week.right(1));
        ui->lblWeek0->setText("����");
        ui->lblWeek1->setText("����");
        ui->lblWeek2->setText("����");
        QStringList ymdList = sixday[i].date.split("-");
        DateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //������������
        TypeList[i]->setText(sixday[i].type);
        TypeIconList[i]->setPixmap(todayTypeMap[sixday[i].type]);

        //���¿�������
        if (sixday[i].aqi >= 0 && sixday[i].aqi <= 50)
        {
            AqlList[i]->setText("��");
            AqlList[i]->setStyleSheet("background-color: rgb(121,184,0);");
        }
        else if (sixday[i].aqi > 50 && sixday[i].aqi <= 100)
        {
            AqlList[i]->setText("��");
            AqlList[i]->setStyleSheet("background-color: rgb(255,178,23);");
        }
        else if (sixday[i].aqi > 100 && sixday[i].aqi <= 150)
        {
            AqlList[i]->setText("���");
            AqlList[i]->setStyleSheet("background-color: rgb(255,87,97);");
        }
        else if (sixday[i].aqi > 150 && sixday[i].aqi <= 200)
        {
            AqlList[i]->setText("�ж�");
            AqlList[i]->setStyleSheet("background-color: rgb(235,17,27);");
        }
        else if (sixday[i].aqi > 200 && sixday[i].aqi <= 250)
        {
            AqlList[i]->setText("�ض�");
            AqlList[i]->setStyleSheet("background-color: rgb(170,0,0);");
        }
        else
        {
            AqlList[i]->setText("����");
            AqlList[i]->setStyleSheet("background-color: rgb(110,0,0);");
        }

        //���·�������
        FxList[i]->setText(sixday[i].fx);
        FlList[i]->setText(sixday[i].fl);

    }




}

void MainWindow::InitWidGet()
{
    //���ؼ���ӵ��ؼ�����
/*���ڼ�*/
    WeekList << ui->lblWeek0 << ui->lblWeek1 << ui->lblWeek2 << ui->lblWeek3 << ui->lblWeek4 << ui->lblWeek5;
    /*����*/
    DateList << ui->lblDate0 << ui->lblDate1 << ui->lblDate2 << ui->lblDate3 << ui->lblDate4 << ui->lblDate5;
    /*����*/
    TypeList << ui->lblType0 << ui->lblType1 << ui->lblType2 << ui->lblType3 << ui->lblType4 << ui->lblType5;
    /*����ͼ��*/
    TypeIconList << ui->lblTypeIcon0 << ui->lblTypeIcon1 << ui->lblTypeIcon2 << ui->lblTypeIcon3 << ui->lblTypeIcon4 << ui->lblTypeIcon5;

    /*������Ⱦ*/
    AqlList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;
    /*����ͷ���*/
    FxList << ui->lblFx0 << ui->lblFx1 << ui->lblFx2 << ui->lblFx3 << ui->lblFx4 << ui->lblFx5;
    FlList << ui->lblFl0 << ui->lblFl1 << ui->lblFl2 << ui->lblFl3 << ui->lblFl4 << ui->lblFl5;

    //ͼ��
    todayTypeMap.insert("��ѩ", ":/res/type/BaoXue.png");
    todayTypeMap.insert("����", ":/res/type/BaoYu.png");
    todayTypeMap.insert("���굽����", ":/res/type/BaoYuDaoDaBaoYu.png");
    todayTypeMap.insert("����", ":/res/type/DaBaoYu.png");
    todayTypeMap.insert("���굽�ش���", ":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    todayTypeMap.insert("�󵽱�ѩ", ":/res/type/DaDaoBaoXue.png");
    todayTypeMap.insert("�󵽱���", ":/res/type/DaDaoBaoYu.png");
    todayTypeMap.insert("��ѩ", ":/res/type/DaXue.png");
    todayTypeMap.insert("����", ":/res/type/DaYu.png");
    todayTypeMap.insert("����", ":/res/type/DongYu.png");
    todayTypeMap.insert("����", ":/res/type/DuoYun.png");
    todayTypeMap.insert("����", ":/res/type/FuChen.png");
    todayTypeMap.insert("������", ":/res/type/LeiZhenYu.png");
    todayTypeMap.insert("��������б���", ":/res/type/LeiZhenYuBanYouBingBao.png");
    todayTypeMap.insert("��", ":/res/type/Mai.png");
    todayTypeMap.insert("ǿɳ�Ǳ�", ":/res/type/QiangShaChenBao.png");
    todayTypeMap.insert("��", ":/res/type/Qing.png");
    todayTypeMap.insert("ɳ�Ǳ�", ":/res/type/ShaChenBao.png");
    todayTypeMap.insert("�ش���", ":/res/type/TeDaBaoYu.png");
    todayTypeMap.insert("undefined", ":/res/type/undefined.png");
    todayTypeMap.insert("��", ":/res/type/Wu.png");
    todayTypeMap.insert("С����ѩ", ":/res/type/XiaoDaoZhongXue.png");
    todayTypeMap.insert("С������", ":/res/type/XiaoDaoZhongYu.png");
    todayTypeMap.insert("Сѩ", ":/res/type/XiaoXue.png");
    todayTypeMap.insert("С��", ":/res/type/XiaoYu.png");
    todayTypeMap.insert("ѩ", ":/res/type/Xue.png");
    todayTypeMap.insert("��ɳ", ":/res/type/YangSha.png");
    todayTypeMap.insert("��", ":/res/type/Yin.png");
    todayTypeMap.insert("��", ":/res/type/Yu.png");
    todayTypeMap.insert("���ѩ", ":/res/type/YuJiaXue.png");
    todayTypeMap.insert("��ѩ", ":/res/type/ZhenXue.png");
    todayTypeMap.insert("����", ":/res/type/ZhenYu.png");
    todayTypeMap.insert("�е���ѩ", ":/res/type/ZhongDaoDaXue.png");
    todayTypeMap.insert("�е�����", ":/res/type/ZhongDaoDaYu.png");
    todayTypeMap.insert("��ѩ", ":/res/type/ZhongXue.png");
    todayTypeMap.insert("����", ":/res/type/ZhongYu.png");

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
    QLabel* Now = nullptr; // ��ǰҪ���Ƶ����߱�ǩ
    QColor color; // ���ߵ���ɫ
    if (HighOrLow == "High")
    {
        Now = ui->lblHighCurve; // ��ȡ�������߱�ǩ
        color = QColor(255, 170, 0); // ���ø���������ɫΪ��ɫ
    }
    else if (HighOrLow == "Low")
    {
        Now = ui->lblLowCurve; // ��ȡ�������߱�ǩ
        color = QColor(0, 255, 255); // ���õ���������ɫΪ��ɫ
    }
    else
    {
        return; // �������"High"��"Low"����ֱ�ӷ���
    }

    QPainter painter(Now); // ����һ����ͼ���󣬻����ڵ�ǰ���߱�ǩ��
    painter.setRenderHint(QPainter::Antialiasing, true); // ���ÿ����

    int pointX[6] = { 0 }; // �洢X����
    int tempSum = 0; // �¶��ܺ�
    int tempAverage = 0; // �¶�ƽ��ֵ
    int value;

    // ��ȡX����
    for (int i = 0; i < 6; ++i) {
        pointX[i] = WeekList[i]->pos().x() + WeekList[i]->width() / 2 - 6;
        value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        tempSum += value;
    }
    tempAverage = tempSum / 6; // �����¶�ƽ��ֵ

    int pointY[6] = { 0 }; // �洢Y����
    int yCenter = Now->height() / 2; // Y����������

    // ����Y������
    for (int i = 0; i < 6; ++i) {
        value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        pointY[i] = yCenter - ((value - tempAverage) * INCREMENT);
    }

    QPen pen = painter.pen(); // ��ȡ��ǰ����
    pen.setWidth(1); // ���û��ʿ��
    pen.setColor(color); // ���û�����ɫ
    painter.setPen(pen); // ���û���

    painter.setBrush(color); // ���û�ˢ��ɫ

    // ���Ƶ���¶��ı�
    for (int i = 0; i < 6; ++i) {
        painter.drawEllipse(QPoint(pointX[i], pointY[i]), POINT_REDIUS, POINT_REDIUS); // ���Ƶ�
        int value = (HighOrLow == "High") ? sixday[i].high : sixday[i].low;
        painter.drawText(pointX[i] - TEXT_OFFSET_X, pointY[i] - TEXT_OFFSET_Y, QString::number(value) + "��"); // �����¶��ı�
    }

    // ��������
    for (int i = 0; i < 5; ++i) {
        pen.setStyle(i < 2 ? Qt::SolidLine : Qt::DotLine); // ���û�����ʽ
        painter.setPen(pen); // ���û���
        painter.drawLine(pointX[i], pointY[i], pointX[i + 1], pointY[i + 1]); // �����߶�
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


