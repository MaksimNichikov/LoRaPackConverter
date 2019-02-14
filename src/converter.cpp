/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Класс подготовки пакетов для передачи по сети LoRa и приема пакетов от устройства        **
**                                                                                          **
**********************************************************************************************/
#include "converter.h"

Converter::Converter(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LORA_PACK>("LORA_PACK");

    alreadySent = false;

    portInterface = 0;

    retryCount = RETRY_COUNT;
    answerWaitTimeout = ANSWER_WAIT_TIMEOUT;
    idleWaitTimeout = IDLE_WAIT_TIMEOUT;

    QVariant _retryCount = Settings::getSetting("Net868_Server", "RetryCount");
    if(!_retryCount.isNull()){
        retryCount = _retryCount.toInt();
    } else {
        Settings::setSetting("Net868_Server", "RetryCount", retryCount);
    }
    QVariant _answerWaitTimeout = Settings::getSetting("Net868_Server", "AnswerTimeout");
    if(!_answerWaitTimeout.isNull()){
        answerWaitTimeout = _answerWaitTimeout.toInt() * 1000;
    } else {
        Settings::setSetting("Net868_Server", "AnswerTimeout", answerWaitTimeout);
        answerWaitTimeout = answerWaitTimeout * 1000;
    }
    QVariant _idleWaitTimeout = Settings::getSetting("MainApp", "IdleTimeout");
    if(!_idleWaitTimeout.isNull()){
        idleWaitTimeout = _idleWaitTimeout.toInt() * 1000;
    } else {
        Settings::setSetting("MainApp", "IdleTimeout", idleWaitTimeout);
        idleWaitTimeout = idleWaitTimeout  * 1000;
    }

    waitTimer = new QTimer(this);
    connect(waitTimer, SIGNAL(timeout()), this, SLOT(answerTumeout()));

    idleTimer = new QTimer(this);
    connect(idleTimer, SIGNAL(timeout()), this, SLOT(idleTimeout()));
    idleTimerReastart();
}

void Converter::setConsoleColor(int color)
{
    #ifdef Q_OS_WIN
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    #else
        Q_UNUSED(color);
    #endif
}

void Converter::getCheckBuffer(quint8 _port, QString _devEui)
{
    QByteArray data;
    data.append(_port);
    data.append(sendDataBuffer.size() >> 8);
    data.append(sendDataBuffer.size());
    qint32 crc = crc32_fsl(0, (uint8_t*)sendDataBuffer.data(), sendDataBuffer.size());
    data.append(crc >> 24);
    data.append(crc >> 16);
    data.append(crc >> 8);
    data.append(crc);
    sendCommand(FINISH_SEND_DATA_CMD, COMMAND_PORT, data, _devEui);
}

void Converter::idleTimerReastart()
{
    idleTimer->stop();
    idleTimer->start(idleWaitTimeout);
}

void Converter::setDataPackage(LORA_PACK _lp)
{
    idleTimerReastart();
    if(_lp.data.size() > 512){
        QJsonObject jsObj;
        jsObj.insert("status", "ERROR");
        jsObj.insert("info", "Data size > 512 bytes.");
        jsObj.insert("DevEUI", DevEUI);
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        return;
    }
    if(_lp.data.size() < 1 || _lp.dev_eui.isEmpty()){
        QJsonObject jsObj;
        jsObj.insert("status", "ERROR");
        jsObj.insert("info", "Data is empty.");
        jsObj.insert("DevEUI", DevEUI);
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        return;
    }
    if(alreadySent){
        QJsonObject jsObj;
        jsObj.insert("status", "ERROR");
        jsObj.insert("info", "Data already sent.");
        jsObj.insert("DevEUI", DevEUI);
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        return;
    }
    sendDataBuffer.clear();
    fileSizeCounter = 0;
    errorCounter = 0;
    alreadySent = true;
    portInterface = _lp.port_interface;
    if(_lp.data.size() <= LORA_DATA_BLOCK_SIZE){
        sendSmallPack(_lp.dev_eui, _lp.port, _lp.data);
    } else {
        quint16 _header = SEND_PACKAGE_ANSWER_CMD;
        sendDataBuffer.append(_header >> 8);
        sendDataBuffer.append(_header);
        sendDataBuffer.append(char(0x00));
        sendDataBuffer.append(_lp.data);
        devCreateBuffer(_lp.dev_eui, _lp.data.size() + 3, _lp.port);
    }
    QJsonObject jsObj;
    jsObj.insert("status", "OK");
    jsObj.insert("info", "Data sent to device...");
    jsObj.insert("DevEUI", DevEUI);
    emit sendAnswer(QJsonDocument(jsObj).toJson());
}

void Converter::receivePackage(quint8 _interface, QString _devEui, QByteArray _data)
{
    if(DevEUI.contains(_devEui, Qt::CaseInsensitive)){
        //qDebug() << "D::Main DevEui: " << DevEUI << " recv devEui: " << _devEui;
        waitTimer->stop();
        QJsonObject jsObj;
        jsObj.insert("status", "OK");
        jsObj.insert("Interface", QString::number(_interface));
        jsObj.insert("DevEUI", _devEui);
        jsObj.insert("Data", QString(_data.toHex()));
        qDebug() << "D::Canverter data send";
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        alreadySent = false;
        //idleTimerReastart();
        idleTimeout();
    }
}

void Converter::setDevEUI(QString _devEui)
{
    DevEUI = _devEui;
}

void Converter::createBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        setConsoleColor(YELLOW_TEXT_COLOR);
        qDebug() << "\r\nD::Result create buffer: " << _resultCode << " port: " << _port << " buffer size: " << _dataSize << " DevEUI: " << _dev_eui;
        setConsoleColor(DEFAULT_TEXT_COLOR);
        switch (_resultCode) {
        case 0:
            fileSizeCounter = 0;
            errorCounter = 0;
            sendDataBlock(_port, 0, _dev_eui);
            break;
        default:
            QJsonObject jsObj;
            jsObj.insert("status", "ERROR");
            jsObj.insert("info", "Error creating buffer in device. Code " + QString::number(_resultCode));
            jsObj.insert("DevEUI", DevEUI);
            emit sendAnswer(QJsonDocument(jsObj).toJson());
            break;
        }
    }
}

void Converter::sendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _packSize, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        setConsoleColor(MAGENTA_TEXT_COLOR);
        qDebug() << "\r\nD::Result: " << _resultCode << " port: " << _port << " offset: " << _offset << " buffer counter: " << _packSize << " devEui: " << _dev_eui;
        setConsoleColor(DEFAULT_TEXT_COLOR);

        if(_packSize > 0 || _resultCode != 0){
            _offset += _packSize;

            qDebug() << "D::---------------------------------------";
            qDebug() << "D::DevEUI: " << _dev_eui;
            qDebug() << "D::Current file counter: " << fileSizeCounter;
            qDebug() << "D::Received pack size: " << _packSize;
            qDebug() << "D::Device file counter: " << _offset;

            if(_offset > 0){
                if(_offset - fileSizeCounter < _packSize){
                    setConsoleColor(RED_TEXT_COLOR);
                    qDebug() << "D::Bad answer received. Retry send...";
                    setConsoleColor(DEFAULT_TEXT_COLOR);
                    if(fileSizeCounter == _offset){ //Если пришел повторный ответ на один и тот же отправленный пакет.
                        waitTimer->start(answerWaitTimeout);
                        return;
                    }
                }
                if((_offset - fileSizeCounter == _packSize) || (_offset - fileSizeCounter == sendDataBuffer.size() - fileSizeCounter)){
                    setConsoleColor(DARK_GREEN_TEXT_COLOR);
                    qDebug() << "D::Answer write block OK!";
                    setConsoleColor(DEFAULT_TEXT_COLOR);

                    fileSizeCounter += _packSize;
                    errorCounter = 0;
                }
            } else {
                qDebug() << "D::Send first package...";
                qDebug() << "D::Data buffer size:" << sendDataBuffer.size();
            }

            qDebug() << "D::Buffer size counter: " << fileSizeCounter;
            qDebug() << "D::---------------------------------------";
            if(fileSizeCounter == sendDataBuffer.size()){
                qDebug() << "D::End write file!";
                getCheckBuffer(_port, _dev_eui);
            } else {
                sendDataBlock(_port, fileSizeCounter, _dev_eui);
            }
        } else {
            sendDataBlock(_port, fileSizeCounter, _dev_eui);
        }
    }
}

void Converter::finishSendPackAnswer(quint8 _resultCode, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        qDebug() << "I::Send package OK! Wait receive package from device..." << _resultCode;
    }
}

void Converter::createBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        setConsoleColor(DARK_MAGENTA_TEXT_COLOR);
        qDebug() << "I::Receiver create recv buffer created, port: " << _port << " data size: " << _dataSize;
        setConsoleColor(DEFAULT_TEXT_COLOR);
        recvBufferSize = _dataSize;
        recvDataBuffer.clear();
        recvDataBuffer.resize(_dataSize);
        appCreatedBuffer(_port, _dataSize, _dev_eui);
    }
}

void Converter::appCreatedBuffer(quint8 _port, quint16 _dataSize, QString _dev_eui)
{
    QByteArray data;
    data.append(char(0x00));
    data.append(_port);
    data.append(_dataSize >> 8);
    data.append(_dataSize);
    sendCommand(APP_CREATE_BUFFER, COMMAND_PORT, data, _dev_eui);
}

void Converter::receivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        setConsoleColor(BLUE_TEXT_COLOR);
        qDebug() << "D::Received data block, port: " << _port << " offset: " << _offset << " data size: " << _data.size() << " data: " << _data.toHex() << " DevEUI: " << _dev_eui;
        setConsoleColor(DEFAULT_TEXT_COLOR);
        for(int i = 0; i < _data.size(); i++){
            recvDataBuffer[_offset + i] = _data.at(i);
        }
        QByteArray data;
        data.append(char(0x00));
        data.append(_port);
        data.append((_offset) >> 8);
        data.append(_offset);
        data.append(_data.size() >> 8);
        data.append(_data.size());
        sendCommand(APP_RECEIVE_DATA_BLOCK_CMD, COMMAND_PORT, data, _dev_eui);
    }
}

void Converter::finishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui)
{
    if(DevEUI.contains(_dev_eui, Qt::CaseInsensitive)){
        waitTimer->stop();
        setConsoleColor(YELLOW_TEXT_COLOR);
        qDebug() << "D::Receiver pack finished, port: " << _port << " data size: " << _dataSize << " CRC32: " << QString::number((quint32)_crc, 16) << " DevEUI: " << _dev_eui;
        setConsoleColor(DEFAULT_TEXT_COLOR);
        QByteArray data;
        data.append(_port);
        data.append(recvDataBuffer.size() >> 8);
        data.append(recvDataBuffer.size());
        qint32 crc = crc32_fsl(0, (uint8_t*)recvDataBuffer.data(), recvDataBuffer.size());
        data.append(crc >> 24);
        data.append(crc >> 16);
        data.append(crc >> 8);
        data.append(crc);
        sendCommand(APP_FINISH_RECV_DATA_CMD, COMMAND_PORT, data, _dev_eui);

        QJsonObject jsObj;
        if(_crc == crc){
            jsObj.insert("status", "OK");
            jsObj.insert("Interface", QString::number(portInterface));
            jsObj.insert("Data", QString(recvDataBuffer.right(recvDataBuffer.size() - 3).toHex()));
            setConsoleColor(GREEN_TEXT_COLOR);
            qDebug() << "D::Check CRC OK!" << " all data size: " << recvDataBuffer.size();
            setConsoleColor(DEFAULT_TEXT_COLOR);
        } else {
            jsObj.insert("status", "ERROR");
            jsObj.insert("info", "BAD CRC");
            setConsoleColor(RED_TEXT_COLOR);
            qDebug() << "D::Check CRC ERROR!";
            setConsoleColor(DEFAULT_TEXT_COLOR);
        }
        //jsObj.insert("CRC", "0x" + QString::number((quint32)crc, 16));
        jsObj.insert("DevEUI", _dev_eui);

        qDebug() << "D::Converter data send";
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        alreadySent = false;
        //idleTimerReastart();
        idleTimeout();
    }
}

void Converter::answerTumeout()
{
    waitTimer->stop();
    errorCounter++;
    if(errorCounter < retryCount){
        setConsoleColor(RED_TEXT_COLOR);
        qDebug() << "D::Device " << DevEUI << " answer timeout. Retry send...";
        setConsoleColor(DEFAULT_TEXT_COLOR);
        emit sendLoraPack(lastLp);
        waitTimer->start(answerWaitTimeout);
    } else {
        errorCounter = 0;
        setConsoleColor(RED_TEXT_COLOR);
        qDebug() << "D::Device " << DevEUI << " answer timeout. All attempts are exhausted.";
        setConsoleColor(DEFAULT_TEXT_COLOR);
        alreadySent = false;
        QJsonObject jsObj;
        jsObj.insert("status", "ERROR");
        jsObj.insert("info", "Answer timeout.");
        jsObj.insert("DevEUI", DevEUI);
        emit sendAnswer(QJsonDocument(jsObj).toJson());
        //idleTimerReastart();
        idleTimeout();
    }
}

void Converter::idleTimeout()
{
    idleTimer->stop();
    emit idleDisconnectTimeout(DevEUI);
}

void Converter::sendCommand(ushort _cmd, quint8 _port, QByteArray data, QString _devEui)
{
    idleTimerReastart();
    LORA_PACK _lp;
    QByteArray _data;
    _data.append(_cmd >> 8);
    _data.append(_cmd);
    if(!data.isEmpty() && !data.isNull()) _data.append(data);
    _lp.dev_eui = _devEui;
    _lp.port = _port;
    _lp.data = _data;
    qDebug() << "D::Send LoRa pack with data: " << _data.toHex();
    lastLp = _lp;
    emit sendLoraPack(_lp);
    if(_cmd != APP_FINISH_RECV_DATA_CMD) waitTimer->start(answerWaitTimeout);
}

void Converter::sendSmallPack(QString _devEui, quint8 _port, QByteArray _data)
{
    QByteArray data;
    data.append(char(0x00));
    data.append(_data);
    sendCommand(SEND_PACKAGE_ANSWER_CMD, _port, data, _devEui);
}

void Converter::devCreateBuffer(QString _devEui, quint16 _dataSize, quint8 _port)
{
    QByteArray data;
    data.append(_port);
    data.append(_dataSize >> 8);
    data.append(_dataSize);
    sendCommand(DEV_CREATE_BUFFER_CMD, COMMAND_PORT, data, _devEui);
}

void Converter::sendDataBlock(quint8 _port, quint16 _offset, QString _devEui)
{
    //qDebug() << "OFFSET SEND PACKAGE: " << _offset << " DATA BUFFER SIZE: " << sendDataBuffer.size();
    QByteArray data;
    data.append(_port);
    data.append(_offset >> 8);
    data.append(_offset);
    if(_offset + LORA_DATA_BLOCK_SIZE <= sendDataBuffer.size()){
        qDebug() << "D::Send next package...";
        data.append(sendDataBuffer.mid(_offset, LORA_DATA_BLOCK_SIZE));
    } else {
        qDebug() << "D::Send end package... Size: " << sendDataBuffer.size() - _offset;
        data.append(sendDataBuffer.mid(_offset, sendDataBuffer.size() - _offset));
    }
    sendCommand(SEND_DATA_BLOCK_CMD, COMMAND_PORT, data, _devEui);
}
