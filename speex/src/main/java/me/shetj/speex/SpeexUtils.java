package me.shetj.speex;


public class SpeexUtils {
    /*
     * quality 1 : 4kbps (very noticeable artifacts, usually intelligible) 2 :
     * 6kbps (very noticeable artifacts, good intelligibility) 4 : 8kbps
     * (noticeable artifacts sometimes) 6 : 11kpbs (artifacts usually only
     * noticeable with headphones) 8 : 15kbps (artifacts not usually noticeable)
     */

//    7.Speex 编码流程
//    1、定义一个SpeexBits类型变量bits和一个Speex编码器状态变量enc_state。
//            2、调用speex_bits_init(&bits)初始化bits。
//            3、调用speex_encoder_init(&speex_nb_mode)来初始化enc_state。其中speex_nb_mode是SpeexMode类型的变量，表示的是窄带模式。还有speex_wb_mode表示宽带模式、speex_uwb_mode表示超宽带模式。
//            4、调用函数int speex_encoder_ ctl(void *state, int request, void *ptr)来设定编码器的参数，其中参数state表示编码器的状态；参数request表示要定义的参数类型，如SPEEX_ GET_ FRAME_SIZE表示设置帧大小，SPEEX_ SET_QUALITY表示量化大小，这决定了编码的质量；参数ptr表示要设定的值。
//    可通过speex_encoder_ctl(enc_state, SPEEX_GET_FRAME_SIZE, &frame_size) 和speex_encoder_ctl(enc_state, SPEEX_SET_QUALITY, &quality)来设定编码器的参数。
//            5、初始化完毕后，对每一帧声音作如下处理：调用函数speex_bits_reset(&bits)再次设定SpeexBits，然后调用函数speex_encode(enc_state, input_frame, &bits)，参数bits中保存编码后的数据流。
//            6、编码结束后，调用函数speex_bits_destroy (&bits)， speex_encoder_destroy (enc_state)来
//8.解码流程
//    1、 定义一个SpeexBits类型变量bits和一个Speex编码状态变量enc_state。
//            2、 调用speex_bits_init(&bits)初始化bits。
//            3、 调用speex_decoder_init (&speex_nb_mode)来初始化enc_state。
//            4、 调用函数speex_decoder_ctl (void *state, int request, void *ptr)来设定编码器的参数。
//            5、 调用函数 speex_decode(void *state, SpeexBits *bits, float *out)对参数bits中的音频数据进行解编码，参数out中保存解码后的数据流。
//            6、 调用函数speex_bits_destroy(&bits), speex_ decoder_ destroy (void *state)来关闭和销毁SpeexBits和解码器。

    //设置为4时压缩比为1/16(与编解码密切相关)
    private static final int DEFAULT_COMPRESSION = 4;

    static {
        try {
            System.loadLibrary("audio_speex");
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    private static SpeexUtils speexUtil = null;

     SpeexUtils() {
        open(DEFAULT_COMPRESSION);
    }

    public static SpeexUtils getInstance(){
        if (speexUtil == null) {
            synchronized (SpeexUtils.class) {
                if (speexUtil == null) {
                    speexUtil = new SpeexUtils();
                }
            }
        }
        return speexUtil;
    }
    //编解码相关
    public native int open(int compression);

    public native int getFrameSize();

    public native int decode(byte[] encoded, short[] lin, int size);

    public native int encode(short[] lin, int offset, byte[] encoded, int size);

    public native void close();

    //噪音处理
    public native int CancelNoiseInit(int frame_size,int sample_rate);

    public native int CancelNoisePreprocess(byte[] buffer);

    public native int CancelNoisePreprocessByShort(short[] buffer);

    public native int CancelNoiseDestroy();

    //回音处理
    public native int InitAudioAEC(int frame_size,int filter_length,int sampling_rate);

    public native int AudioAECProc(byte[] recordArray  ,byte[] playArray  ,byte[] szOutArray);

    public native int AudioAECProcByShort(short[] recordArray  ,short[] playArray  ,short[] szOutArray);


    public native int ExitSpeexDsp();
}
