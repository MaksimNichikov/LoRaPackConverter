/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Функции чтения и записи параметров в файл конфигурации                                   **
**                                                                                          **
**********************************************************************************************/
#include "settings.h"

QVariant Settings::getSetting(QString section, QString param)
{
    QSettings _settings(qApp->applicationDirPath() + "/settings.conf", QSettings::IniFormat);
    if(!_settings.value(section + "/" + param).isNull()){
        return _settings.value(section + "/" + param);
    } else {
        return QVariant();
    }
}

void Settings::setSetting(QString section, QString param, QVariant value)
{
    QSettings _settings(qApp->applicationDirPath() + "/settings.conf", QSettings::IniFormat);
    _settings.setValue(section + "/" + param, value);
    _settings.sync();
}
