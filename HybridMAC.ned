//CSMA_CA网络描述文件
//复合模块，network关键字表明其可以自己运行 
network HybridMAC{
    parameters:
        int dataNodeNum;						  //节点数目
        int queueSize;							  //队列长度
        double rounds; 							  //执行次数
        
        double CW_checkInterval;				  //碰撞窗口分时隙长
        int CWmin;								  //碰撞窗口总时长
        double DIFS_checkInterval;				  //DIFS分时隙长
        double transporttime;	     			  //数据包传输时间
        int retryLimit;							  //数据包冲突后，重传次数限制
        int backoffLimit;					  	  //二进制退避指数限制
        double lamda;							  //数据包产生，泊松分布参数
        double packetSize;						  //一个数据包中有多少位
        double networkSpeed;					  //网络传播速度
        
    submodules:
        node[dataNodeNum] : Node{
        }
    
    connections allowunconnected://允许连接

}