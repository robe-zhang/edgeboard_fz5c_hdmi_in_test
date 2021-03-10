
# 百度大脑 EdgeBoard 边缘 AI 计算盒，FZ5C，HDMI-IN 视频输入测试程序
  
本程序使用 米尔源码，修改而来，本程序需要配合米尔系统镜像使用，做 HDMI-IN / 视频输入端口的硬件功能性检查  
  
本程序在百度大脑系统镜像下运行会报错，"Error open v4l device.", "Error init v4l device."，此错误是因为百度大脑系统镜像暂时不支持 HDMI-IN 视频输入接口，等百度更新镜像  
  
####  目录结构：  
Makefile   
capzu5.c   
capzu5_v2.c   
capzu5_v3.c   

####  用法：  
git clone 到本地后  
make  clean  
make  
media-ctl -v --set-format '"a0010000.v_tpg_0":0 [RBG24 1920x1080 field:none]'  
./capzu5_v3  

