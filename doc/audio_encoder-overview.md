# 音频编码能力开发概述

音频编码，简单来说就是把音频PCM编码压缩成不同的格式。

目前系统API支持的编码能力如下：

| 容器规格 | 音频编码类型 |
| -------- | :----------- |
| mp4      | AAC、Flac    |
| m4a      | AAC          |
| flac     | Flac         |
| aac      | AAC          |

**音频编码的使用场景**

- 录制

  通过录制传入PCM，然后编码出对应格式的码流，最后封装成想要的格式
- 音频编辑

  编辑PCM后导出音频文件的场景，需要编码成对应音频格式后再封装成文件
- 

# 使用avcodec开发音频编码功能

## 接口说明

**表1** avcodec 音频编码开放能力接口

| 接口名称                                                                                                        | 描述                                                                                 |
| --------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ |
| OH_AVCodec\*OH_AudioEncoder_CreateByMime(const char \*mime);                                                    | 根据媒体类型创建编码器                                                               |
| OH_AVCodec\*OH_AudioEncoder_CreateByName(const char \*name);                                                    | 根据名称创建编码器                                                                   |
| OH_AVErrCode OH_AudioEncoder_Destroy(OH_AVCodec\*codec);                                                        | 销毁已创建的编码器                                                                   |
| OH_AVErrCode OH_AudioEncoder_SetCallback(OH_AVCodec\*codec, OH_AVCodecAsyncCallback callback, void \*userData); | 对编码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知 |
| OH_AVErrCode OH_AudioEncoder_Configure(OH_AVCodec\*codec, OH_AVFormat \*format);                                | 利用解封装得到的解码格式信息来配置编码器                                             |
| OH_AVErrCode OH_AudioEncoder_Prepare(OH_AVCodec\*codec);                                                        | 告知编码器已经准备好,Configure->Prepare->Start                                       |
| OH_AVErrCode OH_AudioEncoder_Start(OH_AVCodec\*codec);                                                          | 启动已经配置好的编码器                                                               |
| OH_AVErrCode OH_AudioEncoder_Stop(OH_AVCodec\*codec);                                                           | 停止编码器                                                                           |
| OH_AVErrCode OH_AudioEncoder_Flush(OH_AVCodec\*codec);                                                          | 刷新编码器的输入输出                                                                 |
| OH_AVErrCode OH_AudioEncoder_Reset(OH_AVCodec\*codec);                                                          | 重置编码器                                                                           |
| OH_AVFormat\*OH_AudioEncoder_GetOutputDescription(OH_AVCodec \*codec);                                          | 返回编码器输出的格式信息                                                             |
| OH_AVErrCode OH_AudioEncoder_SetParameter(OH_AVCodec\*codec, OH_AVFormat \*format);                             | 对编码器实例进行动态配置                                                             |
| OH_AVErrCode OH_AudioEncoder_PushInputData(OH_AVCodec\*codec, uint32_t index, OH_AVCodecBufferAttr attr);       | 发送特定的buffer给编码器处理                                                         |
| OH_AVErrCode OH_AudioEncoder_FreeOutputData(OH_AVCodec\*codec, uint32_t index);                                 | 返回已完成解码的编码器输出数据                                                       |
| OH_AVErrCode OH_AudioEncoder_IsValid(OH_AVCodec\*codec, bool \*isValid);                                        | 查询当前codec实例是否有效, 可用于故障恢复                                            |

## 开发步骤

详细API说明请参考[avcodec native API](https://gitee.com/openharmony/multimedia_av_codec/tree/master/test/nativedemo/audio_demo)

1. 创建编码器实例对象

```c++
//通过 codecname 创建编码器
OH_AVCapability *capability = OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_AAC, true);
const char *name = OH_AVCapability_GetName(capability);
OH_AVCodec *audioEnc = OH_AudioEncoder_CreateByName(name);

//通过 codecname 创建编码器
OH_AVCodec *audioEnc = OH_AudioEncoder_CreateByMime(OH_AVCODEC_MIMETYPE_AUDIO_AAC); 
class AEncSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::mutex startMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::condition_variable startCond_;
    std::queue<uint32_t> inQueue_;
    std::queue<uint32_t> outQueue_;
    std::queue<OH_AVMemory *> inBufferQueue_;
    std::queue<OH_AVMemory *> outBufferQueue_;
    std::queue<OH_AVCodecBufferAttr> attrQueue_;
};
AEncSignal *signal_ = new AEncSignal();
OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
// 配置异步回调
int32_t ret = OH_AudioEncoder_SetCallback(audioEnc, cb, userData);
```

2. 设置回调函数(必须)

```c++
// 设置 OnError 回调函数
static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    (void)errorCode;
    (void)userData;
}
// 设置 FormatChange 回调函数
static void OnOutputFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    (void)codec;
    (void)format;
    (void)userData;
}
// 编码输入PCM送入InputBuffer 队列
static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
{
    (void)codec;
    // 编码输入码流送入InputBuffer 队列
    AEncSignal *signal = static_cast<AEncSignal *>(userData);
    cout << "OnInputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal->inMutex_);
    signal->inQueue_.push(index);
    signal->inBufferQueue_.push(data);
    signal->inCond_.notify_all();
}
// 编码完成的码流送入OutputBuffer队列
static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                           void *userData)
{
    (void)codec;
    // 将对应输出buffer的 index 送入OutputQueue_队列
    // 将对应编码完成的数据data送入outBuffer队列
    AEncSignal *signal = static_cast<AEncSignal *>(userData);
    unique_lock<mutex> lock(signal->outMutex_);
    signal->outQueue_.push(index);
    signal->outBufferQueue_.push(data);
    if (attr) {
        signal->attrQueue_.push(*attr);
    }
}
```

3. 设置采样率，码率，以及声道数，声道类型（必须）、位深、最大输入长度（可选）

   flac 需要额外标识兼容性级别(Compliance Level)和采样精度

   ```c++
   enum AudioFormatType : int32_t {
       TYPE_AAC = 0,
       TYPE_FLAC = 1,
   };  
   // 设置解码分辨率
   int32_t ret;
   // 配置音频采样率（必须）
   constexpr uint32_t DEFAULT_SMAPLERATE = 44100; 
   // 配置音频码率（必须）
   constexpr uint32_t DEFAULT_BITRATE = 32000;
   // 配置音频声道数（必须）
   constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
   // 配置音频声道类型（必须）
   constexpr AudioChannelLayout CHANNEL_LAYOUT = AudioChannelLayout::STEREO;
   // 配置音频位深（必须） flac 只有SAMPLE_S16LE和SAMPLE_S32LE
   constexpr OH_BitsPerSample SAMPLE_FORMAT = OH_BitsPerSample::SAMPLE_S32LE;
   // 配置音频位深（必须）aac只有SAMPLE_S32P
   constexpr OH_BitsPerSample SAMPLE_AAC_FORMAT = OH_BitsPerSample::SAMPLE_S32P;
   // 配置音频compliance level (默认值0，取值范围-2~2)
   constexpr int32_t COMPLIANCE_LEVEL = 0;
   // 配置音频精度（必须） SAMPLE_S16LE和SAMPLE_S24LE和SAMPLE_S32LE
   constexpr BITS_PER_CODED_SAMPLE BITS_PER_CODED_SAMPLE = OH_BitsPerSample::SAMPLE_S24LE;
   // 配置最大输入长度（可选）
   constexpr uint32_t DEFAULT_MAX_INPUT_SIZE = 1024*2*4;//aac
   OH_AVFormat *format = OH_AVFormat_Create();
   // 写入format
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SMAPLERATE);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), DEFAULT_CHANNEL_COUNT);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE.data(), DEFAULT_MAX_INPUT_SIZE);
   OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT.data(), CHANNEL_LAYOUT);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT.data(), SAMPLE_FORMAT);
   if(audioType == TYPE_AAC){
       OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT.data(), SAMPLE_AAC_FORMAT);
   }
   if (audioType == TYPE_FLAC) {
       OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(), BITS_PER_CODED_SAMPLE);
       OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_COMPLIANCE_LEVEL.data(), COMPLIANCE_LEVEL);
   }
   // 配置编码器
   ret = OH_AudioEncoder_Configure(audioEnc, format);
   if (ret != AV_ERR_OK) {
        // 异常处理
   }
   ```
4. prepare通知编码器已经就绪

   ```c++
   OH_AudioEncoder_Prepare(audioEnc);
   ```
5. 启动编码器

   ```c++
   inputFile_ = std::make_unique<std::ifstream>();
   // 打开待解码二进制文件路径
   inputFile_->open(inputFilePath.data(), std::ios::in | std::ios::binary); 
   //配置解码文件输出路径
   outFile_ = std::make_unique<std::ofstream>();
   outFile_->open(outputFilePath.data(), std::ios::out | std::ios::binary);

   // 开始解码
   ret = OH_AudioEncoder_Start(audioEnc);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
6. 写入PCM码流

   ```c++
   constexpr int32_t INPUT_FRAME_BYTES = 2 * 1024 * 4;
   // 配置buffer info信息
   OH_AVCodecBufferAttr info;
   // 设置输入pkt尺寸、偏移量、时间戳等信息
   info.size = pkt_->size;
   info.offset = 0;
   info.pts = pkt_->pts;
   info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
   auto buffer = signal_->inBufferQueue_.front();
   if (inputFile_->eof()){
        info.size = 0;
        info.flags = AVCODEC_BUFFER_FLAGS_EOS;
   }else{
       inputFile_->read((char *)OH_AVMemory_GetAddr(buffer), INPUT_FRAME_BYTES);
   }
   uint32_t index = signal_->inQueue_.front();
   // 送入解码输入队列进行解码, index为对应队列下标
   int32_t ret = OH_AudioEncoder_PushInputData(audioEnc, index, info);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
7. 输出编码格式码流

   ```c++
   OH_AVCodecBufferAttr attr = signal_->attrQueue_.front();
   OH_AVMemory *data = signal_->outBufferQueue_.front();
   uint32_t index = signal_->outQueue_.front();
   // 将解码完成数据data写入到对应输出文件中
   outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
   // buffer 模式, 释放已完成写入的数据
   ret = OH_AudioEncoder_FreeOutputData(audioEnc, index);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
       cout << "decode eos" << endl;
       isRunning_.store(false);
       break;
   }
   ```
8. 刷新编码器(在文件流EOS标识发送之后需要刷新或者碰到可以继续执行的错误的时候(OH_AudioDecoder_IsValid 为true)，然后启动可以重新继续编码)

   ```c++
   // 刷新编码器 audioEnc
   ret = OH_AudioEncoder_Flush(audioEnc);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   // 重新开始解码
   ret = OH_AudioEncoder_Start(audioEnc);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
9. 重置编码器(碰到不可以继续执行的错误的时候(OH_AudioDecoder_IsValid 为false)或者需要重新配置时)

   ```c++
   // 重置编码器 audioEnc
   ret = OH_AudioEncoder_Reset(audioEnc);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   // 重新配置编码器参数
   ret = OH_AudioEncoder_Configure(audioEnc, format);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
10. 停止编码(编码结束执行)

    ```c++
    // 终止编码器 audioEnc
    ret = OH_AudioEncoder_Stop(audioEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
        return ret;
    ```
11. 注销编码器实例

    ```c++
    // 调用OH_AudioEncoder_Destroy, 注销编码器
    ret = OH_AudioEncoder_Destroy(audioEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    } else {
        audioEnc = NULL; //不可重复destroy
    }
    return ret;
    ```
