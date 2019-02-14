/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QCoreApplication>

class Settings : public QObject
{
    Q_OBJECT
public:

    static QVariant getSetting(QString section, QString param);

    static void setSetting(QString section, QString param, QVariant value);
};

#endif // SETTINGS_H
