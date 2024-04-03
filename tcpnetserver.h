#ifndef TCPNETSERVER_H
#define TCPNETSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>


#ifdef _NETWORK_CMD_C_
#define _NETWORK_CMD_C_EXT_
#define _NETWORK_CMD_C_EXT_INT_ //
#else
#define _NETWORK_CMD_C_EXT_  extern
#define _NETWORK_CMD_C_EXT_INT_ extern
#endif

//数据上传套接字编号
#define DATA_UPLOAD_SOCKET_NUM (0)
//设置设备套接字编号
#define SETTING_SOCKET_NUM (1)


//命令包头
#define NET_CMD_HEAD (0x1234)
//命令包尾
#define NET_CMD_END (0x5678)

//应答包头
#define NET_CMD_ACK_HEAD (0x89ab)
//应答包尾
#define NET_CMD_ACK_END (0xcdef)

//设置结果
//设置成功
#define NET_CMD_SET_SUCCESS (0)
//设置失败
#define NET_CMD_SET_FAIL (1)

//设备类型
#define NET_CMD_DEVICE_TYPE (0x01)

//具体数据前所有字节的之和
#define NET_CMD_FIXED_HEAD_LEN (6)

//命令代码
#define NET_CMD_CODE_SEND_IMAGE (200)	//发送图片

#define NET_CMD_CODE_SET_INTEGRAL_TIME (1)	//设置积分时间
#define NET_CMD_CODE_SET_INTEGRAL_CAP (2)	//设置积分电容
#define NET_CMD_CODE_SET_ADD_CNT (3)		//设置累加次数
#define NET_CMD_CODE_UPLOAD_TEMPERATURE (4)	//上传温度
#define NET_CMD_CODE_UPLOAD_PRESSURE (5)	//上传气压
#define NET_CMD_CODE_WRITE_SN (6)	//写序列号
#define NET_CMD_CODE_READ_SN (7)	//读序列号
#define NET_CMD_CODE_WRITE_CORRECTION_FACTOR (8)	//写修正系数
#define NET_CMD_CODE_READ_CORRECTION_FACTOR (9)	//读修正系数
#define NET_CMD_CODE_SET_IP (10)	//设置IP
#define NET_CMD_CODE_GET_FW_VER (11)	//获取固件版本号
#define NET_CMD_CODE_SET_ANGLE_0 (12)	//设置当前位置角度为0

//数据长度位置引索
#define NET_CMD_HEAD_INDEX_1 (0)//同步头
#define NET_CMD_HEAD_INDEX_2 (1)

#define NET_CMD_DEVICE_TYPE_INDEX (2) //设备类型
#define NET_CMD_CMD_CODE_INDEX (3)	//命令代码

#define NET_CMD_DATA_LEN_INDEX_1 (4)//数据长度
#define NET_CMD_DATA_LEN_INDEX_2 (5)

#define NET_CMD_DATA_INDEX (6)



;typedef struct sysrecvPkg__
{
    uint8_t head_h;			//包头	aa
    uint8_t head_l;			//包头	aa
    uint8_t length_h;		//  包的长度= 数据长度(length_h)+10;
    uint8_t length_l;		//
    uint8_t cmd;			//命令
    uint8_t type;			//类型
    uint8_t data[1024];		//数据(长度可变)(数据+crc(u16)+0x0d0a)
                            //uint8_t end;			//包尾FE
} sysrecvPkg, *sysrecvPkg_pst;



#define CMD_DDC	0	//ddc参数配置
#define CMD_SYS	1 	//系统参数配置
#define CMD_RSU	2 	//系统更新
#define CMD_FAC	3 	//工厂模式

#define TYPE_DDC_TIME 	0		//积分时间
#define TYPE_DDC_CAP  	1		//积分电容

#define TYPE_SYS_TEMP   	0	//温度
#define TYPE_SYS_STP		1	//气压
#define TYPE_SYS_AG			2	//角度
#define TYPE_SYS_MODEL		3	//软件模式
#define TYPE_SYS_SN			4	//SN码
#define TYPE_SYS_RFACTOR	5 	//读取修正系数
#define TYPE_SYS_FACTOR_1	6 	//修正系数1
#define TYPE_SYS_FACTOR_2	7 	//修正系数2
#define TYPE_SYS_FACTOR_3	8	//修正系数3
#define TYPE_SYS_FACTOR_4	9 	//修正系数4
#define TYPE_SYS_FACTOR_5	10 	//修正系数5
#define TYPE_SYS_FACTOR_6	11	//修正系数6

#define TYPE_RSU_RESTART 0		//更新完成 重启
#define TYPE_RSU_START	 1		//开始更新

#define TYPE_FAC_SN			1 	//设置SN码
#define TYPE_FAC_FACTOR_1	2 	//修正系数1
#define TYPE_FAC_FACTOR_2	3 	//修正系数2
#define TYPE_FAC_FACTOR_3	4 	//修正系数3
#define TYPE_FAC_FACTOR_4	5 	//修正系数4
#define TYPE_FAC_FACTOR_5	6 	//修正系数5
#define TYPE_FAC_FACTOR_6	7 	//修正系数6



#pragma pack(1)

struct network_send_cmd__
{
    uint8_t buf[3600];
    uint16_t len;
};

#pragma pack()    //取消自定义字节对齐方式



enum net_handle_task_event_table_
{
    NET_HANDLE_EVENT = 0,
};


enum net_mode_update_task_event_table_
{
    NET_MODE_UPDATE_EVENT = 0,
};



class TcpNetServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpNetServer(QObject *parent = nullptr);
    void start(int port);
    int iport;


    void sendback(sysrecvPkg *cmd);
public slots:
    void sendDataToClientsLocally();

private slots:
    void onNewConnection();
    void onReadyRead();

    void onClientDisconnected() ;

private:
    QTcpServer *server;
    QTcpSocket *currentconnect;

   // QTcpServer *server2;

    QList<QTcpSocket*> clients;

signals:

};

#endif // TCPNETSERVER_H
