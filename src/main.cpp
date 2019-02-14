/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#include <QCoreApplication>
#include <iostream>
#include "cleanexit.h"
#include "webapiserver.h"
#include "settings.h"
#include "appinfo.h"

#define DEFAULT_PORT 44556

bool printLogToFile = false;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
    QString logMsg = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") + " " + msg + "\n";
    std::cout << logMsg.toStdString();
    std::cout.flush();

    if(printLogToFile){
        QFile fMessFile(qApp->applicationDirPath() + "/lora_pack_converter.log");
        if(!fMessFile.open(QIODevice::Append | QIODevice::Text)){
           return;
        }
        QTextStream tsTextStream(&fMessFile);
        tsTextStream << logMsg;
        tsTextStream.flush();
        fMessFile.flush();
        fMessFile.close();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc > 1){
        for (int i = 1; i < argc; ++i) {
            if(!strcmp(argv[i], "-log")){
                printLogToFile = true;
            }
        }
    }

    CleanExit cleanExit;

    qInstallMessageHandler(myMessageOutput);

    qDebug() << "I::LoRa pack converter started! " << APP_VERSION << " (" << BUILD_DATE << ")\n";

    int mainAppPort = DEFAULT_PORT;
    bool useSsl = false;

    QVariant _port = Settings::getSetting("MainApp", "port");
    if(!_port.isNull()){
        mainAppPort = _port.toInt();
    } else {
        Settings::setSetting("MainApp", "port", mainAppPort);
    }
    QVariant _useSsl = Settings::getSetting("Security", "useSsl");
    if(!_useSsl.isNull()){
        useSsl = _useSsl.toBool();
    } else {
        Settings::setSetting("Security", "useSsl", useSsl);
    }

    WebApiServer server(mainAppPort, useSsl);

    int ret = a.exec();

    return ret;
}
