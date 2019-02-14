/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Парсер входящих данных                                                                   **
**                                                                                          **
**********************************************************************************************/
#include "packparser.h"

PackParser::PackParser(QObject *parent) : QObject(parent)
{
    qDebug() << "D::LoRa parser started!";
    runFlag = true;
    qRegisterMetaType<FILE_INFO>("FILE_INFO");
    qRegisterMetaType<LORA_PACK>("LORA_PACK");
}

void PackParser::setLoraQueue(QQueue<QPair<QString, QByteArray>> *_que)
{
    loraQue = _que;
}

void PackParser::startParser()
{
    runFlag = true;
    while(runFlag){
        if(!loraQue->isEmpty()){
            QPair <QString, QByteArray> _dataPair;
            _dataPair = loraQue->dequeue();
            QByteArray arr = _dataPair.second;
            if(arr.contains("overdue")){
                emit overdueRecived(_dataPair.first);
            } else {
                quint8 _port = (quint8)arr.at(0);
                arr = arr.right(arr.size() - 1);
                dataParser(arr, _port, _dataPair.first);
            }
        }
        if(!queryDataToSend.isEmpty()){
            emit sendLoraPack(queryDataToSend.dequeue());
        }
        QThread::msleep(1);
    }
    qDebug() << "D::Parser stoped!";
    emit finished(0);
}

void PackParser::stopParser()
{
    runFlag = false;
}

void PackParser::dataParser(QByteArray data, quint8 _port, QString _devEui)
{
    Q_UNUSED(_port);
    ushort cmd = uchar(data.at(0) & MASK_ANSWER) << 8 | uchar(data.at(1));
    switch(cmd){
    case SEND_DATA_BLOCK_CMD:
        emit sendDataBlockAnswer(data.at(2), data.at(3), quint8(data.at(4)) << 8 | quint8(data.at(5)), quint8(data.at(6)) << 8 | quint8(data.at(7)), _devEui);
        break;
    case DEV_CREATE_BUFFER_CMD:
        emit createBufferAnswer(data.at(2), data.at(3), quint8(data.at(4)) << 8 | quint8(data.at(5)), _devEui);
        break;
    case RECEIVE_PACKAGE_CMD:
        emit receivePackage(data.at(2), _devEui, data.right(data.size() - 3));
        break;
    case FINISH_SEND_DATA_CMD:
        emit finishSendPackAnswer(data.at(2), _devEui);
        break;
    case DEV_CREATE_BUFFER:
        emit createBufferCmd(data.at(2), quint8(data.at(3)) << 8 | quint8(data.at(4)), _devEui);
        break;
    case DEV_RECEIVE_DATA_BLOCK_CMD:
        emit receivedDataBlock(data.at(2), quint8(data.at(3)) << 8 | quint8(data.at(4)), data.right(data.size() - 5), _devEui);
        break;
    case DEV_FINISH_RECV_DATA_CMD:
        emit finishReceivedPackAnswer(data.at(2), quint8(data.at(3)) << 8 | quint8(data.at(4)), quint8(data.at(5)) << 24 | quint8(data.at(6)) << 16 | quint8(data.at(7)) << 8 | quint8(data.at(8)), _devEui);
        break;
    }
}

void PackParser::addDataToQuery(LORA_PACK _loraPack)
{
    queryDataToSend.enqueue(_loraPack);
}
