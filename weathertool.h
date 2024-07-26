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
		if (it == mCityMap.end())//防止城市名中有“市”未查找到
		{
			//QString q = "市";
			it = mCityMap.find(cityName + "市");
		}

		if (it != mCityMap.end()) {
			return it.value();
		}
		else
			return "";

	}

private:
	static QMap<QString, QString> mCityMap;// 城市名称和城市代码的映射表

	static void InitCityMap() {
		//读取文件
		QString filePath = ":/citycode.json";
		QFile file(filePath);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		if (file.isOpen()) {
			// 文件已成功打开
			qDebug() << "文件打开成功";
		}
		else {
			// 文件打开失败
			qDebug() << "文件打开失败";
		}

		QByteArray json = file.readAll();
		file.close();

		//解析，并写入map
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