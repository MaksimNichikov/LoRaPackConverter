/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include "crc32.h"
#include "packparser.h"
#include "settings.h"

#ifdef Q_OS_WIN
    #include <Windows.h>
#endif

#define COMMAND_PORT 2
#define LORA_DATA_BLOCK_SIZE 46
#define RETRY_COUNT 3
#define ANSWER_WAIT_TIMEOUT 30
#define IDLE_WAIT_TIMEOUT 180

#define BLACK_TEXT_COLOR 0
#define DARK_BLUE_TEXT_COLOR 1
#define DARK_GREEN_TEXT_COLOR 2
#define DARK_CYAN_TEXT_COLOR 3
#define DARK_RED_TEXT_COLOR 4
#define DARK_MAGENTA_TEXT_COLOR 5
#define BROWN_TEXT_COLOR 6
#define DEFAULT_TEXT_COLOR 7
#define GREY_TEXT_COLOR 8
#define BLUE_TEXT_COLOR 9
#define GREEN_TEXT_COLOR 10
#define CYAN_TEXT_COLOR 11
#define RED_TEXT_COLOR 12
#define MAGENTA_TEXT_COLOR 13
#define YELLOW_TEXT_COLOR 14
#define WHITE_TEXT_COLOR 15

class Converter : public QObject
{
    Q_OBJECT
public:
    explicit Converter(QObject *parent = 0);

signals:

    void sendLoraPack(LORA_PACK);

    void sendAnswer(QByteArray _data);

    void idleDisconnectTimeout(QString);

public slots:

    void setDataPackage(LORA_PACK _lp);

    void receivePackage(quint8 _interface, QString _devEui, QByteArray _data);

    void setDevEUI(QString _devEui);

    void createBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui);

    void sendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _packSize, QString _dev_eui);

    void finishSendPackAnswer(quint8 _resultCode, QString _dev_eui);

    void createBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui);

    void appCreatedBuffer(quint8 _port, quint16 _dataSize, QString _dev_eui);

    void receivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui);

    void finishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui);

    void answerTumeout();

    void idleTimeout();

private:

    void sendCommand(ushort _cmd, quint8 _port, QByteArray data, QString _devEui);

    void sendSmallPack(QString _devEui, quint8 _port, QByteArray _data);

    void devCreateBuffer(QString _devEui, quint16 _dataSize, quint8 _port);

    void sendDataBlock(quint8 _port, quint16 _offset, QString _devEui);

    QString DevEUI;

    QByteArray sendDataBuffer;

    QByteArray recvDataBuffer;

    int recvBufferSize;

    void setConsoleColor(int color);

    int fileSizeCounter;

    int errorCounter;

    void getCheckBuffer(quint8 _port, QString _devEui);

    QTimer *waitTimer;

    LORA_PACK lastLp;

    bool alreadySent;

    QTimer *idleTimer;

    void idleTimerReastart();

    quint8 portInterface;

    int retryCount;
    int answerWaitTimeout;
    int idleWaitTimeout;
};

#endif // CONVERTER_H
