#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <QString>

class Today
{
public:
    Today()
        : date("2022-10-20"), city("����"), ganmao("��ðָ��"), wendu("0"), shidu("0%"), pm25(0),
        quality("������"), type("����"), fx("�Ϸ�"), fl("2��"), high(30), low(18)
    {}

    QString date;   // ����
    QString city;   // ����
    QString ganmao; // ��ðָ��
    QString wendu;  // �¶�
    QString shidu;  // ʪ��
    int pm25;       // pm2.5
    QString quality; // ��������
    QString type;   // ����
    QString fx;     // ����
    QString fl;     // ����
    int high;       // ����
    int low;        // ����
};

class SixDay
{
public:
    SixDay()
        : date("2022-10-20"), week("����"), type("����"), high(0), low(0), fx("�Ϸ�"), fl("2��"), aqi(0)
    {
    }

    QString date;   // ����
    QString week;   // ���ڼ�
    QString type;   // ����
    int high;       // ����
    int low;        // ����
    QString fx;     // ����
    QString fl;     // ����
    int aqi;        // ��������
};

#endif // WEATHER_DATA_H