## Speex 集成 

##### 集成方式cmake 


#### 碰到的问题
1. 需要EXPORT 用 JNIEXPORT 代替 注意添加头文件
2. speex 如有要降噪 需要集成speexdsp ,
3. speex 和speexdsp 一起集成，需要修改一些文件 fftwrap.c 需要修改把一些方法放出来
4. arch.h 需要兼容2个的所有定义

``` java

    //编解码相关
    public native int open(int compression);

    public native int getFrameSize();

    public native int decode(byte[] encoded, short[] lin, int size);

    public native int encode(short[] lin, int offset, byte[] encoded, int size);

    public native void close();

    //噪音处理
    public native int CancelNoiseInit(int frame_size,int sample_rate);

    public native int CancelNoisePreprocess(byte[] buffer);

    public native int CancelNoiseDestroy();

    //回音处理
    public native int InitAudioAEC(int frame_size,int filter_length,int sampling_rate);

    public native int AudioAECProc(byte[] recordArray  ,byte[] playArray  ,byte[] szOutArray);

    public native int ExitSpeexDsp();
```