//
// Created by slack on 17/2/14.
//

# include <jni.h>
#include <string>
#include <unistd.h>

#include <speex/speex.h>

#include <android/log.h>
#include <speex/speex_preprocess.h>
#include <speex/speex_echo.h>
#include "stdio.h"


#define TAG    "slack" // 这个是自定义的LOG的标识
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__) // 定义LOGD类型

static int codec_open = 0;

static int dec_frame_size;
static int enc_frame_size;

static SpeexBits ebits, dbits;
void *enc_state;
void *dec_state;

SpeexPreprocessState *st;
int nInitSuccessFlag = 0;
SpeexEchoState* m_pState;
SpeexPreprocessState* m_pPreprocessorState;
int      m_nFrameSize;
int      m_nFilterLen;
int      m_nSampleRate;
int 	 iArg;




static JavaVM *gJavaVM;

extern "C" {


JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_open(JNIEnv *env, jobject instance, jint compression) {

    int tmp;

    if (codec_open++ != 0)
        return (jint) 0;

    speex_bits_init(&ebits);
    speex_bits_init(&dbits);

    enc_state = speex_encoder_init(&speex_nb_mode);
    dec_state = speex_decoder_init(&speex_nb_mode);
    tmp = compression;
    speex_encoder_ctl(enc_state, SPEEX_SET_QUALITY, &tmp);
    speex_encoder_ctl(enc_state, SPEEX_GET_FRAME_SIZE, &enc_frame_size);
    speex_decoder_ctl(dec_state, SPEEX_GET_FRAME_SIZE, &dec_frame_size);

    return (jint) 0;

}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_getFrameSize(JNIEnv *env, jobject type) {

    if (!codec_open)
        return 0;
    return (jint) enc_frame_size;

}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_decode(JNIEnv *env, jobject instance, jbyteArray encoded,
                                      jshortArray lin, jint size) {

    jbyte buffer[dec_frame_size];
    jshort output_buffer[dec_frame_size];
    jsize encoded_length = size;

    if (!codec_open)
        return 0;

    LOGD("########## encoded_length = %d", encoded_length);
    LOGD("########## dec_frame_size = %d", dec_frame_size);
    env->GetByteArrayRegion(encoded, 0, encoded_length, buffer);
    for (int i = 0; i < encoded_length; i++) {
        LOGD("########## buffer = %c", buffer[i]);
    }

    speex_bits_read_from(&dbits, (char *) buffer, encoded_length);
    speex_decode_int(dec_state, &dbits, output_buffer);
    env->SetShortArrayRegion(lin, 0, dec_frame_size,
                             output_buffer);

    return (jint) dec_frame_size;

}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_encode(JNIEnv *env, jobject instance, jshortArray lin,
                                      jint offset, jbyteArray encoded, jint size) {
    jshort buffer[enc_frame_size];
    jbyte output_buffer[enc_frame_size];
    int nsamples = (size - 1) / enc_frame_size + 1;
    int i, tot_bytes = 0;

    if (!codec_open)
        return 0;

    speex_bits_reset(&ebits);

    for (i = 0; i < nsamples; i++) {
        env->GetShortArrayRegion(lin, offset + i * enc_frame_size, enc_frame_size, buffer);
        speex_encode_int(enc_state, buffer, &ebits);
    }

    tot_bytes = speex_bits_write(&ebits, (char *) output_buffer,
                                 enc_frame_size);
    env->SetByteArrayRegion(encoded, 0, tot_bytes,
                            output_buffer);

    return (jint) tot_bytes;
}

JNIEXPORT void JNICALL
Java_me_shetj_speex_SpeexUtils_close(JNIEnv *env, jobject instance) {

    if (--codec_open != 0)
        return;

    speex_bits_destroy(&ebits);
    speex_bits_destroy(&dbits);
    speex_decoder_destroy(dec_state);
    speex_encoder_destroy(enc_state);

}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_CancelNoiseInit(JNIEnv *env, jobject instance, jint frame_size,
                                               jint sample_rate) {

    int i;
    int count = 0;
    float f;
    st = speex_preprocess_state_init(frame_size / 2, sample_rate);

    i = 1;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DENOISE, &i);//降噪
    i = -25;//负的32位整数
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &i); //设置噪声的dB
    i = 1;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);//增益
    i = 24000;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
    i = 0;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
    f = .0;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
    f = .0;
    speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
    return 1;
}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_CancelNoisePreprocess(JNIEnv *env, jobject instance,
                                                     jbyteArray buffer) {


    jbyte *inbuffer = env->GetByteArrayElements(buffer, 0);


    int vad = speex_preprocess_run(st,  (spx_int16_t*)inbuffer);

    env->ReleaseByteArrayElements(buffer, inbuffer, 0);
    return vad;
}

JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_CancelNoiseDestroy(JNIEnv *env, jobject instance) {
    if (st != NULL) {
        speex_preprocess_state_destroy(st);
        st = NULL;
    }
    return 1;
}


//回声消除部分代码

//初始化回音消除参数
/*
 * jint frame_size        帧长      一般都是  80,160,320
 * jint filter_length     尾长      一般都是  80*25 ,160*25 ,320*25
 * jint sampling_rate     采样频率  一般都是  8000，16000，32000
 * 比如初始化
 *  InitAudioAEC(80, 80*25,8000)   //8K，10毫秒采样一次
 *  InitAudioAEC(160,160*25,16000) //16K，10毫秒采样一次
 *  InitAudioAEC(320,320*25,32000) //32K，10毫秒采样一次
 */
JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_InitAudioAEC(JNIEnv *env, jobject instance, jint frame_size,
                                            jint filter_length, jint sampling_rate) {
    if (nInitSuccessFlag == 1)
        return 1;

    if (frame_size <= 0 || filter_length <= 0 || sampling_rate <= 0) {
        m_nFrameSize = 160;
        m_nFilterLen = 160 * 8;
        m_nSampleRate = 8000;
    } else {
        m_nFrameSize = frame_size;
        m_nFilterLen = filter_length;
        m_nSampleRate = sampling_rate;
    }

    m_pState = speex_echo_state_init(m_nFrameSize, m_nFilterLen);
    if (m_pState == NULL)
        return -1;

    m_pPreprocessorState = speex_preprocess_state_init(m_nFrameSize, m_nSampleRate);
    if (m_pPreprocessorState == NULL)
        return -2;

    iArg = m_nSampleRate;
    speex_echo_ctl(m_pState, SPEEX_ECHO_SET_SAMPLING_RATE, &iArg);
    speex_preprocess_ctl(m_pPreprocessorState, SPEEX_PREPROCESS_SET_ECHO_STATE, m_pState);
    nInitSuccessFlag = 1;
    return 1;
}

/*
 参数：
  jbyteArray recordArray  录音数据
  jbyteArray playArray    放音数据
  jbyteArray szOutArray
*/
JNIEXPORT jint JNICALL
Java_me_shetj_speex_SpeexUtils_AudioAECProc(JNIEnv *env, jobject instance, jbyteArray recordArray,
                                            jbyteArray playArray, jbyteArray szOutArray) {
    if (nInitSuccessFlag == 0)
        return 0;

    jbyte *recordBuffer = env->GetByteArrayElements(recordArray, 0);
    jbyte *playBuffer = env->GetByteArrayElements(playArray, 0);
    jbyte *szOutBuffer = env->GetByteArrayElements(szOutArray, 0);

    speex_echo_cancellation(m_pState, (spx_int16_t *) recordBuffer,
                            (spx_int16_t *) playBuffer, (spx_int16_t *) szOutBuffer);
    int flag = speex_preprocess_run(m_pPreprocessorState, (spx_int16_t *) szOutBuffer);

    env->ReleaseByteArrayElements(recordArray, recordBuffer, 0);
    env->ReleaseByteArrayElements(playArray, playBuffer, 0);
    env->ReleaseByteArrayElements(szOutArray, szOutBuffer, 0);

    return 1;
}

//退出
JNIEXPORT jint JNICALL Java_me_shetj_speex_SpeexUtils_ExitSpeexDsp(JNIEnv *env, jobject instance) {
    if (nInitSuccessFlag == 0)
        return 0;

    if (m_pState != NULL) {
        speex_echo_state_destroy(m_pState);
        m_pState = NULL;
    }
    if (m_pPreprocessorState != NULL) {
        speex_preprocess_state_destroy(m_pPreprocessorState);
        m_pPreprocessorState = NULL;
    }

    nInitSuccessFlag = 0;

    return 1;
}
}
