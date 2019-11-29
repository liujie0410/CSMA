/*
 * Node.cc
 *
 *  Created on: 2019年11月21日
 *      Author: liujie
 */
//协议逻辑

#include "Node.h"
#define DEBUG
#ifdef DEBUG
#include <iostream>
#include <fstream>
using namespace std;

#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

vector<Node*> Node::datanodev;  //包含所有数据节点实例的全局向量
double startTime;               //开始时间



Define_Module(Node);

Node::Node()
{//构造函数
    initEvent = packetcreatEvent = packetaccessEvent
    = DIFScheckEvent = CWcheckEvent = packettransportEvent= NULL;
}

Node::~Node()
{//析构函数
    //对类型为message的event调用cancel，删除即将到来的message，delete则删除现有的message
    cancelAndDelete(initEvent);
    cancelAndDelete(packetcreatEvent);
    cancelAndDelete(packetaccessEvent);
    cancelAndDelete(DIFScheckEvent);
    cancelAndDelete(CWcheckEvent);
    cancelAndDelete(packettransportEvent);
}

//三个回调函数：initialize()、handleMessage(cMessage* msg)、finish()
//初始化函数
void Node::initialize(){
//模块初始化
    cModule* parent =this->getParentModule();    //复合模块指针
    //通过par（）函数访问模块指针
    this->dataNodeNum =parent->par("dataNodeNum");
    this->queueSize  =parent->par("queueSize");
    this->rounds=parent->par("rounds");
    this->DIFS_checkInterval =parent->par("DIFS_checkInterval");
    this->CW_checkInterval  =parent->par("CW_checkInterval");
    this->CWmin  =parent->par("CWmin");
    this->transporttime  =parent->par("transporttime");
    this->retryLimit  =parent->par("retryLimit");
    this->backoffLimit  =parent->par("backoffLimit");
    this->lamda  =parent->par("lamda");
    this->packetSize  =parent->par("packetSize");
    this->networkSpeed  =parent->par("networkSpeed");
    this->setGateSize("in",this->dataNodeNum+3);//多给3个位置，怕指针出错
    this->setGateSize("out",this->dataNodeNum+3);
    this->getDisplayString().setTagArg("i",1,"red");
//事件处理cMessage类对象初始化
    datanodev.push_back(this);
    initEvent=new cMessage("网络初始化");
    packetcreatEvent=new cMessage("产生数据");
    packetaccessEvent=new cMessage("数据包访问");
    DIFScheckEvent=new cMessage("DIFS控制");
    CWcheckEvent=new cMessage("CW控制");
    packettransportEvent=new cMessage("数据包传输");
//网络节点初始化
    initEvent->setKind(init);//网络初始化获取参数
    this->scheduleAt(simTime()+minislot,initEvent); //scheduleAt仿真时间t，把自信息发给自身模块；simTime()返回当前时间
}
//当模块收到消息时，调用handleMessage处理不同事件
void Node::handleMessage(cMessage* msg)
{
    if(msg->isSelfMessage()){
        switch(msg->getKind())
        {
        case init:
            Init();
            break;
        case packetcreate:
            PacketCreate();
            break;
        case packetaccess:
            PacketAccess();
            break;
        case DIFScheck:
            DIFSCheck();
            break;
        case CWcheck:
            CWCheck();
            break;
        case packettransport:
            PacketTransport();
            break;
        }
    }
    else{
        delete msg;
    }
}
//仿真成功终止时调用finish，在仿真期间收集统计信息
void Node::finish()
{
    int totalSentDataPackets=0;//发送数据包总数
    int totallosseddatapackets=0;//丢失数据包总数
    double DataThroughput=0.0;//数据吞吐量
    double endTime=SIMTIME_DBL(simTime());//网络运行结束时间
    if(this->getId()==5){
                char fname[80];
                ofstream constructionStream;
                sprintf(fname,"Statistic_for_CSMA.txt");
                constructionStream.open(fname,ofstream::app);
        for(int i=0;i<dataNodeNum;i++){
            totalSentDataPackets +=datanodev[i]->sentdatapackets;//发送数据包总数=每个数据节点发送的数据包数量之和
            totallosseddatapackets +=datanodev[i]->losseddatapackets;//丢失数据包总数=每个数据节点丢失的数据包数量之和
        }
        double runningTime=(double)(endTime - startTime);//总运行时间
        //数据吞吐量
        DataThroughput=(double)(totalSentDataPackets*this->packetSize)/(double)(runningTime*this->networkSpeed);
        cout<<"------------*运行结果*------------"<<endl;
        cout<<"数据节点总数为： "<<this->dataNodeNum<< endl;
        cout<<"发送成功的数据包总数为： "<<totalSentDataPackets<<"，丢失的数据包总数为："<<totallosseddatapackets<<endl;
        cout<<"运行时间："<<runningTime<<"，数据吞吐量：  "<<DataThroughput<< endl;
        constructionStream<<"数据节点总数"<<"\t"<<"泊松分布参数"<<"\t"<<"数据吞吐量"<<endl;
        constructionStream<<"   "<<this->dataNodeNum<<"   "<<"\t"<<"   "<<this->lamda<<"   "<<"\t"<<DataThroughput<<endl;
        constructionStream.flush();
        constructionStream.close();
    }
}
//其他函数
//初始化函数
void Node::Init(){
    this->round_counter=0;
    this->retryCounter=0;
    this->CW_counter=-1;
    this->backoffStage=0;
    this->DIFS_counter=0;
    this->losseddatapackets=0;
    this->sentdatapackets=0;
    Busy=0;//信道空闲
    startTime = SIMTIME_DBL(simTime());//网络运行开始时间

    PacketCreate();//调用数据包生成函数，节点服从泊松分布产生数据包，即相邻数据包间隔服从指数分布

    //设置CSMA（载波监听多点接入）事件
    packetaccessEvent->setKind(packetaccess);
    this->scheduleAt(simTime() + minislot,packetaccessEvent);
}
//数据包生成函数
void Node::PacketCreate(){
    //当数据队列未达上限时，持续生成数据包
        if(this->dataQueue.size()<this->queueSize){
        VoicePacket* data=new VoicePacket();
        data->setSource_id(this->getId());//源站为当前站
        //随机产生目的站
        int id=uniform(0,dataNodeNum);
        if(datanodev[id]->getId()!=this->getId()){
            data->setAim_id(datanodev[id]->getId());
        }
        data->setSend_time(simTime().dbl());//利用当前时间产生一个随机的数据包发送时间
        this->dataQueue.push(data);
    }
    //节点服从泊松分布产生数据包，即相邻的数据包生成间隔服从指数分布
    double time = exponential(1/this->lamda);
    packetcreatEvent->setKind(packetcreate);
    this->scheduleAt(simTime() + time,packetcreatEvent);
}

void Node::PacketAccess(){
    this->round_counter++;
    if(this->round_counter>=this->rounds){
        endSimulation();
    }
    Busy=0;
    //当数据包队列非空时，进入DIFS
        if( !this->dataQueue.empty()){
            DIFScheckEvent->setKind(DIFScheck);
            this->scheduleAt(simTime() + DIFS_checkInterval,DIFScheckEvent);
        }
}

//DIFS控制函数
void Node::DIFSCheck(){
    if((Busy==0)&&(DIFS_counter < 5)){
        //DIFS分时隙长为0.01，总时长0.05，则每经过一次分时隙长，计数器加1
        DIFS_counter++;
        if(DIFS_counter==5){
            DIFS_counter=0;
            //当DIFS结束，产生退避时间
            if(CW_counter==-1){
              int r = (int)pow(2,(double)backoffStage);
              //基本退避时间为CWmin，则退避时间为r倍的争用期
              CW_counter=(int)uniform(1,CWmin*r);
             }
                CWcheckEvent->setKind(CWcheck);
                this->scheduleAt(simTime() + CW_checkInterval,CWcheckEvent);
        }else{
            DIFScheckEvent->setKind(DIFScheck);
            //每执行一次DIFS事件，相当于经过一次DIFS_checkInterval的片长时间，且DIFS_counter计数器+1
            this->scheduleAt(simTime() + DIFS_checkInterval,DIFScheckEvent);
        }
    }else{
        DIFS_counter=0;
        DIFScheckEvent->setKind(DIFScheck);
        this->scheduleAt(simTime() + transporttime ,DIFScheckEvent);
    }
}

//CW
void Node::CWCheck(){
    //每次检测到信道空闲时，则退避计数器-1，否则等待当前数据包传输完毕，且经过DIFS后再继续CW
    if(Busy ==0&&CW_counter !=0){
        CW_counter--;
        if(CW_counter != 0){
            CWcheckEvent->setKind(CWcheck);
            this->scheduleAt(simTime() +CW_checkInterval,CWcheckEvent);
        }else{
            //当退避计数器减为0后，等待一个小时间片后发送数据包
            packettransportEvent->setKind(packettransport);
            this->scheduleAt(simTime() + minislot,packettransportEvent);
        }
    }else if(Busy==1&&CW_counter!=0){
        //当信道忙，冻结当前退避倒计时
        DIFScheckEvent->setKind(DIFScheck);
        //因本次进入CW事件已经过一个CW_checkInterval，则需要退回该时间，等待上一个数据包传输完成，进入DIFS
        this->scheduleAt(simTime() - CW_checkInterval + transporttime
                +DIFS_checkInterval,DIFScheckEvent);
    }
}

//数据包传输
void Node::PacketTransport(){
    Busy=1;//传输时将信道空闲状态置1，表示忙碌
    int collision_indication=0; //碰撞记号
        for(int i=0;i<dataNodeNum;i++){
            //找到退避完成且此次未传输的节点，对退避次数和重传次数都+1，将碰撞标记置1
            if(datanodev[i]->CW_counter ==0 && datanodev[i]->getId() != this->getId()){
                this->retryCounter++;
                this->backoffStage++;
                collision_indication=1;
                break;
            }
        }
        this->CW_counter=-1;
     if(collision_indication==1){
        if(backoffStage>backoffLimit){
         //当退避指数达到上限，取上限值作为退避指数
         backoffStage=backoffLimit;
        }
        if(retryCounter>retryLimit){
         //重传次数超过限制，丢包
           this->retryCounter=0;
           this->backoffStage=0;
           this->dataQueue.pop();
           this->losseddatapackets++;
        }
    }else{
        //未发生碰撞时，完成数据包发送
        this->dataQueue.pop();
        this->sentdatapackets++;
        this->backoffStage=0;
        this->retryCounter=0;
    }
    packetaccessEvent->setKind(packetaccess);
    this->scheduleAt(simTime() - minislot+transporttime,packetaccessEvent);
}



