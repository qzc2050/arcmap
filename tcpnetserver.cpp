#include "tcpnetserver.h"
#define _NETWORK_CMD_C_
#include "mainwindow.h"
#if 1
#define DBG_PRINTF		printf
#else
#define DBG_PRINTF		//
#endif
#include "ddc2256.h"
typedef    uint8_t   INT8U ;
typedef    uint16_t   INT16U ;
typedef    uint32_t   INT32U ;
#include <QSettings>
network_send_cmd__ network_send_cmd;
network_send_cmd__ network_send_cmdimage;
uint8_t Gcorrec_factor[2048]={0};
/******************************************************
 *函数名称:CheckSumSum16
 *输   入:buf 要校验的数据
 len 校验数据的长
 *输   出:校验和
 *功   能:校验和-16
 *******************************************************/
uint16_t CheckSum16(uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    uint32_t Sum = 0;
    uint16_t CheckSum = 0;

    for (i = 0; i < len; i++) {
        Sum += *buf++;
    }
    //32转16
    CheckSum = (Sum >> 16) + (Sum & 0xffff);
    return CheckSum;
}
/*************************************************************
 功能：添加校验和
 形参：data_buf:数据包首指针       len:所要进行校验计算的数据长度
 返回：
 详解：产生一个校验和
 编写：HB    2018.06
**************************************************************/
uint8_t procotol_get_checksum(uint8_t *data_buf, uint16_t len)
{
    uint8_t checksum = 0;
    uint16_t i;

    //计算校验和
    for(i = 0; i < len ; i++)
    {
        checksum += data_buf[i];
    }

    checksum = 0 - checksum;

    return checksum;
}


void readCorrectionFactor(int p_type, int p_num) {




    QSettings *settings = new QSettings("setcorrect.ini", QSettings::IniFormat);
    QString productsernum;
    settings->beginGroup("CORRECTION_FACTOR");

    QString key = "CORRECTION_FACTOR_" + QString::number(p_type) + "_" + QString::number(p_num);

    // 从 QSettings 读取数据（Base64 编码的字符串）
    QByteArray byteArray = settings->value(key).toByteArray();

    // 解码 Base64 字符串并将数据拷贝到 Gcorrec_factor 数组中
    QByteArray decodedData = QByteArray::fromBase64(byteArray);
    memcpy(Gcorrec_factor, decodedData.constData(), decodedData.size());

    settings->sync();
    settings->endGroup();
    delete settings;



}
void writeCorrectionFactor(int p_type, int p_num) {


    QSettings *settings = new QSettings("setcorrect.ini", QSettings::IniFormat);
    QString productsernum;
    settings->beginGroup("CORRECTION_FACTOR");

    QString key = "CORRECTION_FACTOR_" + QString::number(p_type) + "_" + QString::number(p_num);

    QByteArray byteArray(reinterpret_cast<const char*>(Gcorrec_factor), sizeof(Gcorrec_factor));
    QString encodedData = byteArray.toBase64();

    settings->setValue(key, encodedData);
    settings->sync();
    settings->endGroup();
    delete settings;
}
/*************************************************************
 功能：对数据包进行校验
 形参：temp_buf：数据指针  len：数据长度
 返回：TRUE：校验成功   FALSE：校验失败
 详解：对数据包的全部数据进行相加，检测结果是否为0
 编写：HB    2018.06
**************************************************************/
INT8U protocol_check_data(INT8U *temp_buf, INT16U len)
{
    INT16U i;
    INT8U temp = 0;

    for(i = 0; i < len; i++)
        temp += temp_buf[i];	// 累加校验

    if(temp != 0)
    {
        return 0;	 //校验错误
    }

    return 1;	//校验成功
}
/************************************************************
 * 功能: 上传压力数据
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_pressure_info_api(INT8U p_result, INT16U pressure)
{
    //INT16U data_len = 2;	//数据区域的长度
    //INT16U checkbit;

    //network_send_cmd.len = 0;

    ////命令头
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa;
    //
    ////数据区域数据长度
    //network_send_cmd.buf[network_send_cmd.len++] = data_len >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = data_len;

    ////命令
    //network_send_cmd.buf[network_send_cmd.len++] = CMD_SYS;
    ////类型
    //network_send_cmd.buf[network_send_cmd.len++] = TYPE_SYS_STP;

    ////数据
    //network_send_cmd.buf[network_send_cmd.len++] = pressure >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = pressure ;

    ////计算校验和，数据长度到数据区域结束
    //checkbit = CheckSum16(&network_send_cmd.buf[2], network_send_cmd.len - 2);

    ////校验和
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit;

    ////包尾
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a;


    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_UPLOAD_PRESSURE;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //压力值
    network_send_cmd.buf[network_send_cmd.len++] = pressure >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = pressure;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{

//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}

//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}

}






/************************************************************
 * 功能: 上传温度数据
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_temperature_info_api(INT8U p_result, INT16U temperature)
{
    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_UPLOAD_TEMPERATURE;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //压力值
    network_send_cmd.buf[network_send_cmd.len++] = temperature >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = temperature;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{

//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}

}






/************************************************************
 * 功能: 积分时间设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_set_integral_time_api(INT16U p_result)
{
    //INT16U data_len = 2;	//数据区域的长度
    //INT16U checkbit;

    //network_send_cmd.len = 0;

    ////命令头
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa;
    //
    ////数据区域数据长度
    //network_send_cmd.buf[network_send_cmd.len++] = data_len >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = data_len;

    ////命令
    //network_send_cmd.buf[network_send_cmd.len++] = CMD_DDC;
    ////类型
    //network_send_cmd.buf[network_send_cmd.len++] = TYPE_DDC_TIME;

    ////数据
    //network_send_cmd.buf[network_send_cmd.len++] = p_time >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = p_time ;

    ////计算校验和，数据长度到数据区域结束
    //checkbit = CheckSum16(&network_send_cmd.buf[2], network_send_cmd.len - 2);

    ////校验和
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit;

    ////包尾
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a;

    //if(S1_State == (S_INIT|S_CONN))
    //{
    //	S1_Data &= ~S_TRANSMITOK;

    //	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
    //}

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_SET_INTEGRAL_TIME;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}


}



/************************************************************
 * 功能: 积分电容设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_set_integral_cap_api(INT8U p_result)
{
    //INT16U data_len = 1;	//数据区域的长度
    //INT16U checkbit;

    //network_send_cmd.len = 0;

    ////命令头
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0xaaaa;
    //
    ////数据区域数据长度
    //network_send_cmd.buf[network_send_cmd.len++] = data_len >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = data_len;

    ////命令
    //network_send_cmd.buf[network_send_cmd.len++] = CMD_DDC;
    ////类型
    //network_send_cmd.buf[network_send_cmd.len++] = TYPE_DDC_TIME;

    ////数据
    //network_send_cmd.buf[network_send_cmd.len++] = p_cap;

    ////计算校验和，数据长度到数据区域结束
    //checkbit = CheckSum16(&network_send_cmd.buf[2], network_send_cmd.len - 2);

    ////校验和
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = checkbit;

    ////包尾
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a >> 8;
    //network_send_cmd.buf[network_send_cmd.len++] = 0x0d0a;

    //if(S1_State == (S_INIT|S_CONN))
    //{
    //	S1_Data &= ~S_TRANSMITOK;

    //	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
    //}

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_SET_INTEGRAL_CAP;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 累加次数设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_set_add_cnt_api(INT8U p_result)
{
    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_SET_ADD_CNT;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}


/************************************************************
 * 功能: 写序列号设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_write_sn_api(INT8U p_result)
{
    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_WRITE_SN;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 读序列号设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_read_sn_api(INT8U p_result, INT8U *r_sn)
{
    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_READ_SN;

    network_send_cmd.len += 2;//先跳过数据长度的保存


    //数据区域

    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //序列号
    memcpy(&network_send_cmd.buf[network_send_cmd.len], GproductInfo.productsernum, 10);
    network_send_cmd.len += 10;


    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 写序列号设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_write_correction_factor_api(INT8U p_result, INT8U p_type, INT8U p_num)
{
    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_WRITE_CORRECTION_FACTOR;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;
    //类型
    network_send_cmd.buf[network_send_cmd.len++] = p_type;
    //编号
    network_send_cmd.buf[network_send_cmd.len++] = p_num;

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 读序列号设置返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_read_correction_factor_api(INT8U p_result, INT8U p_type, INT8U p_num, INT8U *pdata)
{
    INT16U i;

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_READ_CORRECTION_FACTOR;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    //类型
    network_send_cmd.buf[network_send_cmd.len++] = p_type;
    //编号
    network_send_cmd.buf[network_send_cmd.len++] = p_num;

    for (i = 0; i < 254*4; i++)//float
    {
        network_send_cmd.buf[network_send_cmd.len++] = pdata[i];
    }

    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 设置IP返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_set_ip_api(INT8U p_result)
{
    INT16U i;

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_SET_IP;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;


    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}





/************************************************************
 * 功能: 获取版本号返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_get_fw_ver(INT8U p_result, INT8U p_ver)
{
    INT16U i;

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_GET_FW_VER;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;

    network_send_cmd.buf[network_send_cmd.len++] = p_ver;


    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}



/************************************************************
 * 功能: 设置当前位置角度为0返回
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_ack_set_angle_0(INT8U p_result)
{
    INT16U i;

    network_send_cmd.len = 0;

    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_SET_ANGLE_0;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;


    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;

//	if (sys_data.communication_mode == NET_WIFI_MODE)
//	{
//		if (sys_data.wifi_state == WIFI_INIT_SUCCESS_STATE)	//wifi连接上，初始化成功再发数据
//		{
//			m8266_wiif_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//		}
//	}
//	else
//	{
//		//if(S1_State == (S_INIT|S_CONN))
//		//{
//		//	S1_Data &= ~S_TRANSMITOK;

//		//	Write_SOCK_Data_Buffer(1, network_send_cmd.buf, network_send_cmd.len);//指定Socket(0~7)发送数据处理,端口0发送30字节数据
//		//}

//		w5500_socket_send_data(SETTING_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
//	}
}
//network_ack_set_wifiname_api(NET_CMD_SET_SUCCESS);

//返回 参数1：INT8U p_result，参数2：命令代码INT8U cmd_code
//网络命令返回
void network_ack_cmd_api(INT8U p_result, INT8U cmd_code)
{
    INT16U i;

    network_send_cmd.len = 0;
    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;

    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = cmd_code;

    network_send_cmd.len += 2;//先跳过数据长度的保存

    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;


    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);

    ////包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;
}

// network_ack_get_wifiname_api(NET_CMD_SET_SUCCESS,wifiname);

//网络命令返回
void network_ack_get_wifiname_api(INT8U p_result, INT8U *p_wifiname)
{
    INT16U i;         //网络命令返回

    network_send_cmd.len = 0;
    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;
    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;
    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_GET_WIFINAME;
    network_send_cmd.len += 2;//先跳过数据长度的保存
    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;
    //wifi名称
    for(i = 0; i < 32; i++)
    {
        network_send_cmd.buf[network_send_cmd.len++] = p_wifiname[i];
    }
    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;
    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);
    //包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;
}
//网络命令返回
void network_ack_get_wifipassword_api(INT8U p_result, INT8U *p_wifipass)
{
    INT16U i;         //网络命令返回
    network_send_cmd.len = 0;
    //包头
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_HEAD & 0X00FF;
    //设备类型
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_DEVICE_TYPE;
    //命令代码
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_CODE_GET_WIFIPASS;
    network_send_cmd.len += 2;//先跳过数据长度的保存
    //数据区域
    //结果
    network_send_cmd.buf[network_send_cmd.len++] = p_result;
    //wifi名称
    for(i = 0; i < 32; i++)
    {
        network_send_cmd.buf[network_send_cmd.len++] = p_wifipass[i];
    }
    //数据长度字段填充
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmd.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmd.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;
    //计算校验和
    network_send_cmd.buf[network_send_cmd.len++] = procotol_get_checksum(network_send_cmd.buf, network_send_cmd.len);
    //包尾
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END >> 8;
    network_send_cmd.buf[network_send_cmd.len++] = NET_CMD_ACK_END & 0X00FF;
}
INT8U process_socket1_cmd(INT8U *p_data, INT32U len)
{
    INT8U p_cmd;
    INT16U data_len;
    INT8U temp_buf[3];

    //INT16U press =
    // int16_value = int(float_value * 32767.0)
    if (!((p_data[NET_CMD_HEAD_INDEX_1] == (NET_CMD_HEAD >> 8)) && (p_data[NET_CMD_HEAD_INDEX_2] == (NET_CMD_HEAD & 0x00ff)))) //头验证
    {
        DBG_PRINTF("cmd head err\r\n");
        return 1;
    }

    data_len = ((INT16U)p_data[NET_CMD_DATA_LEN_INDEX_1] << 8) + (INT16U)p_data[NET_CMD_DATA_LEN_INDEX_2];  //长度

    if (!((p_data[data_len + 7] == (NET_CMD_END >> 8)) && (p_data[data_len + 8] == (NET_CMD_END & 0x00ff))))	//尾校验
    {
        DBG_PRINTF("tail err\r\n");
        return 2;
    }

    if (p_data[NET_CMD_DEVICE_TYPE_INDEX] != NET_CMD_DEVICE_TYPE)
    {
        DBG_PRINTF("decive type err\r\n");
        return 3;
    }

    if (protocol_check_data(p_data, len - 2) == 0)
    {
        DBG_PRINTF("check sum err\r\n");
        return 4;
    }

    p_cmd = p_data[NET_CMD_CMD_CODE_INDEX];	//获取命令代码

    switch (p_cmd)
    {
        case NET_CMD_CODE_SET_INTEGRAL_TIME:  //DDC 指令
            {
//                INT16U p_timer;

//                p_timer = p_data[NET_CMD_DATA_INDEX] * 256 + p_data[NET_CMD_DATA_INDEX + 1];	//单位ms

//                DBG_PRINTF("ddc_time:%d ms\r\n", p_timer);

//                if (p_timer < 20)
//                {
//                    p_timer = 20;
//                }

//                if (p_timer > 200)
//                {
//                    p_timer = 200;
//                }

//                sys_data.store_data.cap_timer = p_timer;

                //at24cxx_write(SET_CAP_TIMER_STAR_ADDR, (INT8U *)&sys_data.store_data.cap_timer, sizeof(sys_data.store_data.cap_timer));
                network_ack_set_integral_time_api(NET_CMD_SET_SUCCESS);
            }
            break;

        case NET_CMD_CODE_SET_INTEGRAL_CAP:  //积分电容
            {
//                INT8U p_cap;

//                p_cap = p_data[NET_CMD_DATA_INDEX];

//                DBG_PRINTF("ddc_cap:%d \r\n", p_cap);

//                if ((p_cap >= 0) && (p_cap <= 7))
//                {
//                    ddc2256_set_integral_cap(p_cap);
//                    //network_send_integral_cap_info_api(p_cap);

//                    ddc2256_read_reg_multi_byte(DDC2556_CHIP_1, 0x0001, temp_buf, 3);

//                    network_ack_set_integral_cap_api(NET_CMD_SET_SUCCESS);

//                    DBG_PRINTF("temp_buf = 0x%02x 0x%02x 0x%02x\r\n", temp_buf[0], temp_buf[1], temp_buf[2]);
//                }
//                else
//                {
//                    network_ack_set_integral_cap_api(NET_CMD_SET_FAIL);
//                }

                network_ack_set_integral_cap_api(NET_CMD_SET_SUCCESS);

            }
            break;

        case NET_CMD_CODE_SET_ADD_CNT:  //设置累加次数
            {
//                INT8U p_cnt;	//累加次数

//                p_cnt = p_data[NET_CMD_DATA_INDEX];

//                DBG_PRINTF("ddc_cnt:%d \r\n", p_cnt);

//                ddc2256_set_add_up_time(p_cnt);

                network_ack_set_add_cnt_api(NET_CMD_SET_SUCCESS);
            }
            break;

        case NET_CMD_CODE_UPLOAD_TEMPERATURE:  //上传温度
            {
                //network_ack_temperature_info_api(NET_CMD_SET_SUCCESS, sys_data.cur_temper);
                network_ack_temperature_info_api(NET_CMD_SET_SUCCESS, INT16U(GpsystemStatus.temp*10));
            }
            break;

        case NET_CMD_CODE_UPLOAD_PRESSURE:	//上传气压
            {
                //network_ack_pressure_info_api(NET_CMD_SET_SUCCESS, sys_data.cur_pressure);

                network_ack_pressure_info_api(NET_CMD_SET_SUCCESS, INT16U(GpsystemStatus.press*100));
            }
            break;

        case NET_CMD_CODE_WRITE_SN:	//写序列号
            {
                memcpy(&GproductInfo.productsernum[0], &p_data[NET_CMD_DATA_INDEX], 10);

                //AT24CXX_WRITE_PROTECT_OFF();

                //iic_writebuffer(&sys_data.store_data.device_sn[0], STORE_DATA_SN_STAR_ADDR, DEVICE_SN_MAX_LEN);

                //AT24CXX_WRITE_PROTECT_ON();

                network_ack_write_sn_api(NET_CMD_SET_SUCCESS);
            }
            break;

        case NET_CMD_CODE_READ_SN:	//读序列号
            {
                network_ack_read_sn_api(NET_CMD_SET_SUCCESS,((INT8U *) GproductInfo.productsernum));
            }
            break;

        case NET_CMD_CODE_WRITE_CORRECTION_FACTOR:	//写修正系数
            {
                INT8U p_type=0;
                INT8U p_num=0;

                p_type = p_data[NET_CMD_DATA_INDEX];	//获取是默认、用户
                p_num = p_data[NET_CMD_DATA_INDEX + 1];	//获取是那一片

                DBG_PRINTF("p_type:%d \r\n", p_type);
                DBG_PRINTF("p_num:%d \r\n", p_num);

                if ((p_type >= 0 && p_type <= 1) && (p_num >= 0 && p_num <= 6))//条件符合才写入
                {
                    // 将数组转换为QByteArray
                    //QByteArray byteArray(reinterpret_cast<const char*>(&p_data[NET_CMD_DATA_INDEX + 2]), 254*4);
                    memcpy(Gcorrec_factor, &p_data[NET_CMD_DATA_INDEX + 2], 254*4);

                    writeCorrectionFactor(p_type,p_num);
                   // QSettings settings("MyApp", "CORRECTION_FACTOR");
//                	AT24CXX_WRITE_PROTECT_OFF();

//                	iic_writebuffer((INT8U *)sys_data.store_data.correc_factor, STORE_DATA_CORREC_FACTOR_STAR_ADDR + p_type * (PER_CORREC_FACTOR_AREA * PER_TYPE_AREA_CNT) + p_num * (PER_CORREC_FACTOR_AREA), PER_CORREC_FACTOR_AREA);

//                	AT24CXX_WRITE_PROTECT_ON();

                    network_ack_write_correction_factor_api(NET_CMD_SET_SUCCESS, p_type, p_num);	//成功
                }
                else
                {
                    network_ack_write_correction_factor_api(NET_CMD_SET_FAIL, p_type, p_num);	//失败
                }

            }
            break;

        case NET_CMD_CODE_READ_CORRECTION_FACTOR:	//读修正系数
            {
                INT8U p_type;
                INT8U p_num;

                p_type = p_data[NET_CMD_DATA_INDEX];	//获取是默认、用户
                p_num = p_data[NET_CMD_DATA_INDEX + 1];	//获取是那一片

                DBG_PRINTF("p_type:%d \r\n", p_type);
                DBG_PRINTF("p_num:%d \r\n", p_num);


                if ((p_type >= 0 && p_type <= 1) && (p_num >= 0 && p_num <= 6))//条件符合才写入
                {

                    readCorrectionFactor(p_type,p_num);
                    network_ack_read_correction_factor_api(NET_CMD_SET_SUCCESS, p_type, p_num, (INT8U *)Gcorrec_factor);	//成功
                }
                else
                {
                    network_ack_read_correction_factor_api(NET_CMD_SET_FAIL, p_type, p_num, (INT8U *)Gcorrec_factor);	//失败
                }
            }
            break;

        case NET_CMD_CODE_SET_IP:	//设置IP
            {
                unsigned char tempip[4];
                memset(tempip,0,4);
                tempip[0]= p_data[NET_CMD_DATA_INDEX];
                tempip[1]= p_data[NET_CMD_DATA_INDEX + 1];
                tempip[2]= p_data[NET_CMD_DATA_INDEX + 2];
                tempip[3]= p_data[NET_CMD_DATA_INDEX + 3];
                
                //把tempip转换成“192.16.2.1”的QString
                QString ipstr = QString("%1.%2.%3.%4").arg(tempip[0]).arg(tempip[1]).arg(tempip[2]).arg(tempip[3]);

            
                QHostAddress ipAddress(ipstr);
                if(ipAddress.isNull())
                {
                    //如果ipAddress是null，说明ipString不是合法的IP地址 
                    //返回异常响应
                    network_ack_set_ip_api(NET_CMD_SET_FAIL);
                }
                else
                {
                    //如果ipAddress不是null，说明ipString是合法的IP地址
                    //返回正常响应
                    network_ack_set_ip_api(NET_CMD_SET_SUCCESS);

                    //更新  GconfigNet的ip
                    memset(GconfigNet.ip,0,16);
                    for(int i=0;i<ipstr.length();i++)
                    {
                        //把ipsrt赋值给GconfigNet.ip
                        GconfigNet.ip[i]=ipstr.at(i).toLatin1();
                        // GconfigNet.ip[i]=ipstr.toStdString().c_str()[i];
                    }

                    //如果ipAddress不是null，说明ipString是合法的IP地址
                    //QSetting里面设置IP地址，groupname是eth0,key是ipeth0
                    //把ipString保存到QSetting里面  
                    QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                    settings->beginGroup("eth0");
                    settings->setValue("ipeth0",ipstr);
                    settings->endGroup();
                    settings->sync();
                    delete  settings;  
                    //返回正确响应
                    //network_ack_cmd_api(INT8U p_result, INT8U cmd_code)
                    network_ack_cmd_api(NET_CMD_SET_SUCCESS, NET_CMD_CODE_SET_IP);

                
                    //更新设备ip地址
                    char cmdbuf[1024];
                    memset ( cmdbuf, 0,1024 );
                    QString ips = QString(GconfigNet.ip);
                    sprintf ( cmdbuf, "ifconfig eth0 %s netmask 255.255.254.0",ipstr.toLatin1().data());
                    system ( cmdbuf );
                }
            }   
            break;

        case NET_CMD_CODE_GET_FW_VER:	//获取固件版本号
            {
                network_ack_get_fw_ver(NET_CMD_SET_SUCCESS, 1);

            }
            break;

        case NET_CMD_CODE_SET_ANGLE_0:	//设置角度为0
            {
                //设置当前位置角度为0，读取当前角度，然后将校准值与当前值取反
                GpsystemStatus.cx =-GpsystemStatus.x;
                GpsystemStatus.cy =-GpsystemStatus.y;
                GpsystemStatus.cz =-GpsystemStatus.z;
                //保存到setting group是cangle，key 分别是cx cy cz
                 QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                 settings->beginGroup("cangle");
                 settings->setValue("cx",GpsystemStatus.cx);
                 settings->setValue("cy",GpsystemStatus.cy);
                 settings->setValue("cz",GpsystemStatus.cz);
                 settings->endGroup();
                 settings->sync();
                 delete settings;
                 network_ack_cmd_api(NET_CMD_SET_SUCCESS, 1);

            }
            break;
        case NET_CMD_CODE_SET_WIFINAME:	//设置wifi名称
            {
            // #define NET_CMD_DATA_LEN_INDEX_1 (4)//数据长度
            // #define NET_CMD_DATA_LEN_INDEX_2 (5)//数据长度

            INT8U p_len = p_data[NET_CMD_DATA_LEN_INDEX_1];	//获取数据长度
            QString wifiname = QString(QByteArray::fromRawData((const char*)&p_data[NET_CMD_DATA_INDEX], p_len));
                            //ssid没法判断有没有效，所以不用判断直接保存
            memset(GconfigNet.ssid,0,32);
            for(int i=0;i<wifiname.length();i++)
            {
                //把wifiname赋值给GconfigNet.ssid
                GconfigNet.ssid[i]=wifiname.at(i).toLatin1();
            }
            //更新设备的wifi名字 wifigroup 是wlan，key是wifiname
            QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
            settings->beginGroup("wlan");
            settings->setValue("wifiname",QString(GconfigNet.ssid));
            settings->endGroup();
            settings->sync();
            delete  settings;
            //写/etc/wpa_supplicant.conf
            FILE* fp = fopen ( "/etc/wpa_supplicant.conf", "wb" );
            if(fp == NULL)
            {
                //pline() << "None Encryption";

                //return false;
            }
            fprintf ( fp, "ctrl_interface=/var/run/wpa_supplicant\nctrl_interface_group=0\nap_scan=1\n\n" );

            fprintf ( fp, "network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tpriority=2\n}\n", GconfigNet.ssid,GconfigNet.password);


            fclose(fp);
            //返回正确响应
            network_ack_cmd_api(NET_CMD_SET_SUCCESS, NET_CMD_CODE_SET_WIFINAME);

           
            }
            break;

        case NET_CMD_CODE_SET_WIFIPASS:	//设置wifi密码
            {
                // #define NET_CMD_DATA_LEN_INDEX_1 (4)//数据长度
                // #define NET_CMD_DATA_LEN_INDEX_2 (5)//数据长度   
                INT8U p_len = p_data[NET_CMD_DATA_LEN_INDEX_1];	//获取数据长度
                QString wifipassword = QString(QByteArray::fromRawData((const char*)&p_data[NET_CMD_DATA_INDEX], p_len));
                //wifi密码没法判断有没有效，所以不用判断直接保存
                memset(GconfigNet.password,0,32);
                for(int i=0;i<wifipassword.length();i++)
                {
                    //把wifipassword赋值给GconfigNet.password
                    GconfigNet.password[i]=wifipassword.at(i).toLatin1();
                }
                //更新设备的wifi密码 wifigroup 是wlan，key是wifipassword

                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("wlan");
                settings->setValue("wifipassword",QString(GconfigNet.password));
                settings->endGroup();    
                settings->sync();
                delete  settings;
                //写/etc/wpa_supplicant.conf
                FILE* fp = fopen ( "/etc/wpa_supplicant.conf", "wb" );
                if(fp == NULL)
                {
                    //pline() << "None Encryption";

                    //return false;
                }
                fprintf ( fp, "ctrl_interface=/var/run/wpa_supplicant\nctrl_interface_group=0\nap_scan=1\n\n" );

                fprintf ( fp, "network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n\tpriority=2\n}\n", GconfigNet.ssid,GconfigNet.password);


                fclose(fp);
                //返回正确响应
                network_ack_cmd_api(NET_CMD_SET_SUCCESS, NET_CMD_CODE_SET_WIFIPASS);
            }
            break;
            
        case NET_CMD_CODE_GET_WIFINAME:	//读取wifi名称
            {
                //读取设备的wifi名字 wifigroup 是wlan，key是wifiname
                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("wlan");
                QString wifiname = settings->value("wifiname").toString();
                settings->endGroup();    
                delete  settings;
                //返回正确响应
               
                QByteArray wifiNameUtf8 = wifiname.toUtf8();
                 // 现在将 char * 转换为 unsigned char *
                 unsigned char *wifiNameUChar = (unsigned char *)wifiNameUtf8.data();
                 network_ack_get_wifiname_api(NET_CMD_SET_SUCCESS,wifiNameUChar);

            }
            break;
            
        case NET_CMD_CODE_GET_WIFIPASS:	//读取wifi密码
            {
                //读取设备的wifi密码 wifigroup 是wlan，key是wifipassword            
                QSettings *settings = new QSettings("set.ini",QSettings::IniFormat);
                settings->beginGroup("wlan");
                QString wifipassword = settings->value("wifipassword").toString();              
                settings->endGroup();    
                delete  settings;
                //返回正确响应,INT8U *p_wifipass
            
                QByteArray wifiPasswordUtf8 = wifipassword.toUtf8();

                // 现在将 char * 转换为 unsigned char *
                unsigned char *wifiPasswordUChar = reinterpret_cast<unsigned char *>(wifiPasswordUtf8.data());


                network_ack_get_wifipassword_api(NET_CMD_SET_SUCCESS,wifiPasswordUChar);
            }
            break;


        default:
            break;
    }

    return 0;
}



/************************************************************
 * 功能: 上传图像数据
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/


void network_send_image_api_2dmap(void)
{
    //#define DATA_LEN (1792)
    //INT16U data_len = 1792;
    uint16_t data_len = 1280;
    uint16_t i, j, count = 0;
    uint16_t checkbit;
    static uint32_t num = 0;
    static uint32_t p_timer = 0;

    network_send_cmdimage.len = 0;	//记录要发送字节的长度

    //序号
    num++;
    //包头
    network_send_cmdimage.buf[network_send_cmdimage.len++] = 12345 >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = 12345 & 0X00FF;
    //序号
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 24;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 16;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num & 0X00000FF;
    network_send_cmdimage.len += 2;//先跳校验
    network_send_cmdimage.buf[network_send_cmdimage.len++] = data_len >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = data_len & 0X00FF;

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 256; j++)
        {
            network_send_cmdimage.buf[network_send_cmdimage.len++] = ddc2556_data[i].adc_vale[j] >> 8;
            network_send_cmdimage.buf[network_send_cmdimage.len++] = ddc2556_data[i].adc_vale[j] & 0X00FF;
            count++;
        }

    }

    checkbit = CheckSum16(network_send_cmdimage.buf + 10, count * 2);
    network_send_cmdimage.buf[6] = checkbit >> 8;
    network_send_cmdimage.buf[7] = checkbit & 0X00FF;
    //包尾
    network_send_cmdimage.buf[network_send_cmdimage.len++] = 56789 >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++]= 56789 & 0X00FF;

    //w5500_socket_send_data(DATA_UPLOAD_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);
}
/************************************************************
 * 功能: 上传图像数据(用旧协议的格式)
 * 形参:
 * 返回: 无
 * 说明: 无
 *************************************************************/
void network_send_image_api_old_protocol(void)
{
    #define DATA_LEN (1792)
    //INT16U data_len = 1792;
    uint16_t i, j, count = 0;
    uint16_t checkbit;
    static uint32_t num = 0;
    static uint32_t p_timer = 0;


    //序号
    num++;
    //包头
    network_send_cmdimage.buf[0] = 12345 >> 8;
    network_send_cmdimage.buf[1] = 12345 & 0X00FF;
    //序号
    network_send_cmdimage.buf[2] = num >> 24;
    network_send_cmdimage.buf[3] = num >> 16;
    network_send_cmdimage.buf[4] = num >> 8;
    network_send_cmdimage.buf[5] = num & 0X00000FF;
    //数据长度
    network_send_cmdimage.buf[8] = DATA_LEN >> 8;
    network_send_cmdimage.buf[9] = DATA_LEN & 0X00FF;

    //DBG_PRINTF("collect cnt :");
    //数据
    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 256; j++)
        {
            //ddc2556_data[i].adc_vale[j] = pdata;
            network_send_cmdimage.buf[10 + count * 2] = ddc2556_data[i].adc_vale[j] >> 8;//ddc2556_adc_vale[i][j] >> 8;
            network_send_cmdimage.buf[10 + count * 2 + 1] = ddc2556_data[i].adc_vale[j];//ddc2556_adc_vale[i][j] & 0X00FF;
            count++;
        }

        //if (ddc2556_data[i].collect_cnt == 1)
        //{
        //	DBG_PRINTF("%02u ", ddc2556_data[i].collect_cnt);
        //}
        //else
        //{
        //	DBG_PRINTF("%02uxx ", ddc2556_data[i].collect_cnt);
        //}

        ddc2556_data[i].collect_cnt = 0;
    }

    //DBG_PRINTF("\r\n");

    //计算校验和
    checkbit = CheckSum16(network_send_cmdimage.buf + 10, count * 2);
    network_send_cmdimage.buf[6] = checkbit >> 8;
    network_send_cmdimage.buf[7] = checkbit & 0X00FF;
    //包尾
    network_send_cmdimage.buf[10 + count * 2] = 56789 >> 8;
    network_send_cmdimage.buf[10 + count * 2 + 1] = 56789 & 0X00FF;

    //总长度
    network_send_cmdimage.len = 10 + count * 2 + 2;



}

/************************************************************
 * 功能: 上传图像数据
 * 形参:
 * 返回: 无
 * 说明: 无 arcmap
 *************************************************************/
void network_send_image_api(void)
{
    //#define DATA_LEN (1792)
    //INT16U data_len = 1792;
    uint16_t i, j, count = 0;
    uint16_t checkbit;
    static uint32_t num = 0;
    static uint32_t p_timer = 0;

    network_send_cmdimage.len = 0;	//记录要发送字节的长度

    //序号
    num++;
    //包头
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_HEAD >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_HEAD & 0X00FF;

    //设备类型
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_DEVICE_TYPE;

    //命令代码
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_CODE_SEND_IMAGE;

    network_send_cmdimage.len += 2;//先跳过数据长度的保存

    //序号
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 24;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 16;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = num & 0X00000FF;

    //DBG_PRINTF("collect cnt :");
    //数据
    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 256; j++)
        {
            checkbit = i * 256 + j;
            network_send_cmdimage.buf[network_send_cmdimage.len++] = ddc2556_data[i].adc_vale[j] >> 8;//ddc2556_adc_vale[i][j] >> 8;
            network_send_cmdimage.buf[network_send_cmdimage.len++] = ddc2556_data[i].adc_vale[j];//ddc2556_adc_vale[i][j] & 0X00FF;
        }

        //if (ddc2556_data[i].collect_cnt == 1)
        //{
        //	DBG_PRINTF("%02u ", ddc2556_data[i].collect_cnt);
        //}
        //else
        //{
        //	DBG_PRINTF("%02uxx ", ddc2556_data[i].collect_cnt);
        //}

        ddc2556_data[i].collect_cnt = 0;
    }

    //数据长度字段填充
    network_send_cmdimage.buf[NET_CMD_DATA_LEN_INDEX_1] = (network_send_cmdimage.len - NET_CMD_FIXED_HEAD_LEN) >> 8;
    network_send_cmdimage.buf[NET_CMD_DATA_LEN_INDEX_2] = (network_send_cmdimage.len - NET_CMD_FIXED_HEAD_LEN) & 0X00FF;

    //计算校验和
    network_send_cmdimage.buf[network_send_cmdimage.len++] = procotol_get_checksum(network_send_cmdimage.buf, network_send_cmdimage.len);

    ////包尾
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_END >> 8;
    network_send_cmdimage.buf[network_send_cmdimage.len++] = NET_CMD_END & 0X00FF;


        //if (sys_timer - p_timer >= 110)
        //{
        //	DBG_PRINTF("st %02lu ms\r\n\r\n\r\n", sys_timer - p_timer);
        //}


        //p_timer = sys_timer;

    //w5500_socket_send_data(DATA_UPLOAD_SOCKET_NUM, network_send_cmd.buf, network_send_cmd.len);



}
int mode=0;
TcpNetServer::TcpNetServer(QObject *parent) : QObject(parent) {
    server = new  QTcpServer(this);
    //server2 = new QTcpServer(this);



    currentconnect = nullptr;
    connect(server, &QTcpServer::newConnection, this, &TcpNetServer::onNewConnection);
}

void TcpNetServer::start(int port)
{

    if (server->listen(QHostAddress::Any, port))
    {
        iport = port;
        qDebug() << "Server started, listening on port" << port;
    } else {
        qWarning() << "Failed to start server:" << server->errorString();
    }
}



void TcpNetServer::sendDataToClientsLocally()
{
   // QTcpSocket* clientSocket =
     //qDebug()<<iport<<"trn";
    if(nullptr!=currentconnect)
    {
        if(currentconnect != nullptr && currentconnect->state() == QAbstractSocket::ConnectedState)
        {
            //return;
            if(mode==0)
            {
                //qDebug
                network_send_image_api();

            }
            else
            {
                network_send_image_api_old_protocol();
            }
            //network_send_image_api_2dmap();
            //
            QByteArray data((const char *)network_send_cmdimage.buf,network_send_cmdimage.len);
            currentconnect->write(data,network_send_cmdimage.len);
            qDebug()<<iport<<"trn";
        }
    }

}



void TcpNetServer::onNewConnection()
{

    if(currentconnect != nullptr) // 如果有原有连接，要先断开并释放资源
    {
        currentconnect->disconnectFromHost();
        currentconnect->deleteLater();
    }
    // 新的连接成为当前的socket
    currentconnect = server->nextPendingConnection();
    connect(currentconnect, &QTcpSocket::readyRead, this, &TcpNetServer::onReadyRead);


}

void TcpNetServer::onClientDisconnected()
{

}

void TcpNetServer::  sendback(sysrecvPkg *cmd)
{

    uint16_t ret, size, len, sentsize = 0, checkbit;
    len = cmd->length_h * 256 + cmd->length_l;		//数据长度
    if(len>1024)
    {
        return;
    }

    size = len + 10;								//包长
    ret = size;
    checkbit = CheckSum16(&cmd->length_h, len + 4); //计算校验??  包头 包尾和校验位 不???算
    cmd->data[len] = checkbit >> 8;					//??充校验位
    cmd->data[len + 1] = checkbit & 0X00FF;
    cmd->data[len + 2] = 0x0d; //??充包??
    cmd->data[len + 3] = 0x0a;



    if(currentconnect != nullptr && currentconnect->state() == QAbstractSocket::ConnectedState)
    {

        QByteArray data((const char *)cmd,size);
        qDebug()<<"return data size "<<data<<size<<iport;
        currentconnect->write((const char *)cmd,size);

    }

}

void TcpNetServer::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket->isValid())
    {
        QByteArray datarecv = clientSocket->readAll();
        QHostAddress readaddresss =clientSocket->peerAddress();
//        if(readaddresss.)
//        {
            qDebug() << "Received data from"  <<iport<<clientSocket->peerAddress().toString() << ":" << datarecv.toHex();
//            return;

//        }
//"\x12""4\x01\x07\x00\x00\xB2Vx"
        // 在这里处理接收到的数据
            network_send_cmd.len=0;
            if(datarecv.contains("bgtest"))
            {
                 mode=1;
                 datarecv.append("ok");
//                if(datarecv.contains("e"))
//                {
//                   mode=1;
//                   qDebug()<<"modee 1111";
//                }
                 //QByteArray data((const char *)network_send_cmd.buf,network_send_cmd.len);
                 currentconnect->write(datarecv,datarecv.length());
            }
            else if(datarecv.contains("hvtest"))
            {
                datarecv.append("ok");
                GDebugMoni =1;
                currentconnect->write(datarecv,datarecv.length());
            }
            else
            {
                INT8U result = process_socket1_cmd(reinterpret_cast<INT8U*>(datarecv.data()), datarecv.size());

                QByteArray data((const char *)network_send_cmd.buf,network_send_cmd.len);
                currentconnect->write(data,network_send_cmd.len);
            }





        // 示例：发送回复数据给客户端
       // clientSocket->write("Server reply: " + data);
    }
}
