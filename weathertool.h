#pragma once

#include <QString>
#include <QMap>
#include <QFile>
#include <qdebug.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#pragma execution_character_set("utf-8")

class WeatherTool {
public:
	static QString getCityCode(QString cityName) {
		if (mCityMap.isEmpty()) {
			InitCityMap();
		}

		QMap<QString, QString>::iterator it = mCityMap.find(cityName);
		if (it == mCityMap.end())//��ֹ���������С��С�δ���ҵ�
		{
			//QString q = "��";
			it = mCityMap.find(cityName + "��");
		}

		if (it != mCityMap.end()) {
			return it.value();
		}
		else
			return "";

	}

private:
	static QMap<QString, QString> mCityMap;// �������ƺͳ��д����ӳ���

	static void InitCityMap() {
		//��ȡ�ļ�
		QString filePath = ":/citycode.json";
		QFile file(filePath);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		if (file.isOpen()) {
			// �ļ��ѳɹ���
			qDebug() << "�ļ��򿪳ɹ�";
		}
		else {
			// �ļ���ʧ��
			qDebug() << "�ļ���ʧ��";
		}

		QByteArray json = file.readAll();
		file.close();

		//��������д��map
		QJsonParseError err;
		QJsonDocument doc = QJsonDocument::fromJson(json, &err);

		if (err.error != QJsonParseError::NoError)
			return;
		if (!doc.isArray())
			return;

		QJsonArray cities = doc.array();
		for (int i = 0; i < cities.size(); i++)
		{
			QString city = cities[i].toObject().value("city_name").toString();
			QString code = cities[i].toObject().value("city_code").toString();

			if (code.size() > 0) {
				mCityMap.insert(city, code);
			}

		}

	}

};

QMap<QString, QString> WeatherTool::mCityMap = {};