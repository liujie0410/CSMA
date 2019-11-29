//消息文件
//定义数据包结构
message VoicePacket
{
    int source_id;//源站
    int aim_id;//目的站
    double send_time; //该数据包发送的时间
}