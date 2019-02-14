/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef CLEANEXIT_H
#define CLEANEXIT_H

#include <QObject>
#include <QCoreApplication>
#include <csignal>

class CleanExit : public QObject
{
    Q_OBJECT

public:

    explicit CleanExit(QObject *parent = 0);

private:

    static void exitQt(int sig);
};

#endif // CLEANEXIT_H
