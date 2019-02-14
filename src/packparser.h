/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef PACKPARSER_H
#define PACKPARSER_H

#include <QObject>
#include <QQueue>
#include <QThread>
#include <QDebug>
#include <inttypes.h>

#define DEV_INFO_CMD 0x8002
#define CREATE_FILE_CMD 0x8100
#define CHECK_FILE_CMD 0x8101
#define REBOOT_CMD 0x8001
#define GET_FILEINFO_CMD 0x8102
#define LOAD_BLOCKS_CMD 0x8103

#define SWITCH_FSK_MODE_CMD 0x8300
#define SWITCH_NORMAL_MODE_CMD 0x8301
#define FSK_MASTER_MODE 1
#define FSK_SLAVE_MODE 0
#define FSK_CODE_CMD 0x8302
#define CLEAR_FSK_STATUS_CMD 0x8304
#define REMOTE_FSK_ANSWER_CMD 0x8305

#define SEND_DATA_BLOCK_CMD 0x8400
#define DEV_CREATE_BUFFER_CMD 0x8401
#define FINISH_SEND_DATA_CMD 0x8402
#define SEND_PACKAGE_ANSWER_CMD 0x8900
#define RECEIVE_PACKAGE_CMD 0x8901

#define DEV_RECEIVE_DATA_BLOCK_CMD 0x8403
#define DEV_CREATE_BUFFER 0x8404
#define DEV_FINISH_RECV_DATA_CMD 0x8405

#define APP_RECEIVE_DATA_BLOCK_CMD 0xC403
#define APP_CREATE_BUFFER 0xC404
#define APP_FINISH_RECV_DATA_CMD 0xC405

#define MASK_ANSWER 0xBF

struct FILE_INFO
{
    quint8 fileType;
    QString bootloaderVer;
    QString fwDevVer;
    ushort devID;
    QString fwVerInFile;
    ushort devIDInFile;
};

struct LORA_PACK{
    QString dev_eui;
    quint8 port;
    quint8 port_interface;
    QString msg_type;
    QByteArray data;
};

class PackParser : public QObject
{
    Q_OBJECT
public:

    explicit PackParser(QObject *parent = 0);

    void setLoraQueue(QQueue<QPair<QString, QByteArray> > *_que);

signals:

    void finished(int);

    void setDevInfo(QString, QString, QString _dev_eui);

    void createFile(quint8 status, QString _dev_eui);

    void sendNextBlock(quint8 fileType, quint32 offset, quint32, QString _dev_eui);

    void resultCheckFile(quint8 statusCode, QString _dev_eui);

    void rebootAnswer(quint8 statusCode, QString _dev_eui);

    void switchFskModeAnswer(quint8 resultCode, quint8 statusCode, QString _dev_eui);

    void switchNormalModeAnswer(quint8 resultCode, quint8 statusCode, QString _dev_eui);

    void fskCommandAnswer(quint8 resultCode, quint8 statusCode, QString _dev_eui);

    void remoteFskAnswer(QString devEui, QByteArray data, QString _dev_eui);

    void fileInfoAnswer(FILE_INFO, QString _dev_eui);

    void sendLog(QString, QString _dev_eui);

    void loadedBlocks(int, QString _dev_eui);

    void overdueRecived(QString _dev_eui);

    void sendLoraPack(LORA_PACK);

    void receivePackage(quint8 _interface, QString _devEui, QByteArray _data);

    void createBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui);

    void sendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _dataWriten, QString _dev_eui);

    void finishSendPackAnswer(quint8 _resultCode, QString _dev_eui);

    void createBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui);

    void receivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui);

    void finishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui);

public slots:

    void startParser();

    void stopParser();

    void dataParser(QByteArray data, quint8 _port, QString _devEui);

    void addDataToQuery(LORA_PACK _loraPack);

private:

    QQueue<QPair<QString, QByteArray>> *loraQue;

    QQueue <LORA_PACK> queryDataToSend;

    bool runFlag;
};

#endif // PACKPARSER_H
