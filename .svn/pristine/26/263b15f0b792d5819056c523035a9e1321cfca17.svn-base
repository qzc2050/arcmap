#ifndef RDSMODBUSSLAVE_H
#define RDSMODBUSSLAVE_H
#include <iostream>
#include <thread>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include <string>
//#include "libmodbus-master/src/modbus.h"
#include "QModbusServer"
#include <QModbusTcpServer>
using namespace std;
/*如果是windows平台则要加载相应的静态库和头文件*/
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>


/*linux平台*/
#else

#include <unistd.h>
#include <error.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#endif

#include <QThread>
class RDSModbusSlave: public QModbusTcpServer
{
public:
    RDSModbusSlave();
    ~RDSModbusSlave();



    QModbusResponse handleWriteSingleCoil(const QModbusPdu &request);
    QModbusResponse handleWriteHoldingRegisters(const QModbusPdu &request);
    QModbusResponse handleReadHoldingRegisters(const QModbusPdu &request);
    QModbusResponse handleReadInputRegisters(const QModbusPdu &request);

public:
    bool modbus_set_slave_id(int id);
    bool initModbus(std::string Host_Ip, int port, bool debugging);
    uint8_t getTab_Input_Bits(int NumBit);
    bool setTab_Input_Bits(int NumBit, uint8_t Value);
    uint16_t getRegisterValue(int registerNumber);
    bool setRegisterValue(int registerNumber, uint16_t Value);
    bool setRegisterFloatValue(float Value, int registerStartaddress);
    float getRegisterFloatValue(int registerStartaddress);

    void handleDeviceError(QModbusDevice::Error newError) ;

    //void onmodbusStateChanged(int state);
private Q_SLOTS:

     QModbusResponse processRequest(const QModbusPdu &request) override;

private:
    std::mutex slavemutex;
    int m_errCount{ 0 };
    int m_modbusSocket{ -1 };
    bool m_initialized{ false };
    //modbus_t* ctx{ nullptr };
   // modbus_mapping_t* mapping{ nullptr };
    /*Mapping*/
    int m_numBits{ 500 };
    int m_numInputBits{ 500 };
    int m_numRegisters{ 500 };
    int m_numInputRegisters{ 500 };


     QModbusServer *modbusDevice;

public:
    void loadFromConfigFile();
    //void run();
};
/*annotation:
(1)typedef struct _modbus_mapping_t
{
    int nb_bits;                //线圈
    int start_bits;
    int nb_input_bits;          //离散输入
    int start_input_bits;
    int nb_input_registers;     //输入寄存器
    int start_input_registers;
    int nb_registers;           //保持寄存器
    int start_registers;
    uint8_t *tab_bits;
    uint8_t *tab_input_bits;
    uint16_t *tab_input_registers;
    uint16_t *tab_registers;
}modbus_mapping_t;*/
#endif // RDSMODBUSSLAVE_H

