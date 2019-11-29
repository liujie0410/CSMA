/*
 * Node.h
 *
 *  Created on: 2019年11月21日
 *      Author: liujie
 */
//头文件
#ifndef NODE_H_
#define NODE_H_
#include <omnetpp.h>
#include <vector>
#include <string>
#include <queue>
#include "VoicePacket_m.h"

using namespace std;
using namespace omnetpp;

//定义参数
#define init 1                   //网络初始化
#define packetcreate 2           //数据包生成
#define packetaccess 3           //数据包访问
#define DIFScheck 4              //帧间隔
#define CWcheck 5                //争用窗口
#define packettransport 6        //数据包传输

#define minislot 0.00001         //时间片

class Node:public cSimpleModule
{
private:
    int dataNodeNum;        //节点数目
    int queueSize;          //队列大小
    double rounds;          //运行次数上限
    int round_counter;      //运行次数统计
    int retryLimit;         //数据包冲突后，重传次数限制
    int retryCounter;       //重传计数器
    int backoffLimit;       //二进制退避指数限制
    int backoffStage;       //当前二进制退避指数计数器，冲突一次，该值会+1，直到达到backoffLimit

    double DIFS_checkInterval;   //DIFS分时隙长
    int DIFS_counter;            //DIFS计数器
    double CW_checkInterval;     //争用窗口分时隙长
    int CWmin;                   //争用窗口总时长
    int CW_counter;              //争用窗口计数
    double transporttime;        //数据传输时间
    double lamda;                        //泊松分布参数
    double packetSize;                   //数据包大小
    double networkSpeed;                 //网络传输速度

    queue<VoicePacket*> dataQueue;       //数据包数据传输队列

    //定义类型为message的event
    cMessage* initEvent;  //网络初始化
    cMessage* packetcreatEvent;           //生成数据
    cMessage* packetaccessEvent;          //数据包访问
    cMessage* DIFScheckEvent;             //DIFS控制
    cMessage* CWcheckEvent;               //争用窗口控制
    cMessage* packettransportEvent;       //数据包传输

    //函数声明
    void Init();                    //网络初始化
    void PacketCreate();            //生成数据包
    void DIFSCheck();               //DIFS控制
    void CWCheck();                 //争用窗口控制
    void PacketAccess();            //数据包访问
    void PacketTransport();         //数据包传输

protected:
    virtual void initialize();                  //初始化
    virtual void handleMessage(cMessage* msg);  //消息处理
    virtual void finish();                      //仿真结束时调用
public:
    static vector<Node*> datanodev;             //包含所有数据节点实例的全局向量
    int losseddatapackets;                      //有数据丢失的数据包
    int sentdatapackets;                        //成功发送数据的数据包
    Node();
    ~Node();
};
int Busy;                      //忙碌
#endif /* NODE_H_ */
