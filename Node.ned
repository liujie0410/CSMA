 //节点描述
 //简单模块
   simple Node{   
   parameters: 
   //参数 
      double x;  //节点位置
      double y;
      @display("p=$x,$y;i=old/x_blank,red");
   gates:
   //门，模块的连接点
      input in[];
      output out[];
}

