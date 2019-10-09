## Speex 集成 

##### 集成方式cmake 


#### 碰到的问题
1. 需要EXPORT 用 JNIEXPORT 代替 注意添加头文件
2. speex 如有要降噪 需要集成speexdsp ,
3. speex 和speexdsp 一起集成，需要修改一些文件 fftwrap.c 需要修改把一些方法放出来
4. arch.h 需要兼容2个的所有定义
