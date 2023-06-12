# 音频解码能力开发概述

音频解码，简单来说，就是把音频编码后的媒体数据，解码成PCM码流的过程。

目前系统API支持的解码能力如下：

| 封装格式 | 音频解码类型                 |
| -------- | :--------------------------- |
| mp4      | AAC、MPEG(MP3)、Flac、Vorbis |
| m4a      | AAC                          |
| flac     | Flac                         |
| ogg      | Vorbis                       |
| aac      | AAC                          |
| mp3      | MPEG(MP3)                    |

**音频解码的使用场景**

- 音频播放或音效渲染场景

  播放音频或对音频进行音效处理之前，需要先对音频进行解码，然后送到硬件扬声器播放或者送给音效处理模块渲染。
- 音频编辑

  音频的编辑是针对PCM来进行的，所以需要首先对已编码的音频文件解码出PCM，然后进行编辑操作。

# 使用avcodec开发音频解码功能

## 接口说明

**表1** avcodec 音频解码开放能力接口

| 接口名称                                                                                                        | 描述                                                                                 |
| --------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ |
| OH_AVCodec\*OH_AudioDecoder_CreateByMime(const char \*mime);                                                    | 根据媒体类型创建解码器                                                               |
| OH_AVCodec\*OH_AudioDecoder_CreateByName(const char \*name);                                                    | 根据名称创建解码器                                                                   |
| OH_AVErrCode OH_AudioDecoder_Destroy(OH_AVCodec\*codec);                                                        | 销毁已创建的解码器                                                                   |
| OH_AVErrCode OH_AudioDecoder_SetCallback(OH_AVCodec\*codec, OH_AVCodecAsyncCallback callback, void \*userData); | 对解码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知 |
| OH_AVErrCode OH_AudioDecoder_Configure(OH_AVCodec\*codec, OH_AVFormat \*format);                                | 利用解封装得到的解码格式信息来配置解码器                                             |
| OH_AVErrCode OH_AudioDecoder_Prepare(OH_AVCodec\*codec);                                                        | 告知解码器已经准备好,Configure->Prepare->Start                                       |
| OH_AVErrCode OH_AudioDecoder_Start(OH_AVCodec\*codec);                                                          | 启动已经配置好的解码器                                                               |
| OH_AVErrCode OH_AudioDecoder_Stop(OH_AVCodec\*codec);                                                           | 停止解码器                                                                           |
| OH_AVErrCode OH_AudioDecoder_Flush(OH_AVCodec\*codec);                                                          | 刷新解码器的输入输出                                                                 |
| OH_AVErrCode OH_AudioDecoder_Reset(OH_AVCodec\*codec);                                                          | 重置解码器                                                                           |
| OH_AVFormat\*OH_AudioDecoder_GetOutputDescription(OH_AVCodec \*codec);                                          | 返回解码器输出的格式信息                                                             |
| OH_AVErrCode OH_AudioDecoder_SetParameter(OH_AVCodec\*codec, OH_AVFormat \*format);                             | 对解码器实例进行动态配置                                                             |
| OH_AVErrCode OH_AudioDecoder_PushInputData(OH_AVCodec\*codec, uint32_t index, OH_AVCodecBufferAttr attr);       | 发送特定的buffer给解码器处理                                                         |
| OH_AVErrCode OH_AudioDecoder_FreeOutputData(OH_AVCodec\*codec, uint32_t index);                                 | 返回已完成解码的解码器输出数据                                                       |
| OH_AVErrCode OH_AudioDecoder_IsValid(OH_AVCodec\*codec, bool \*isValid);                                        | 查询当前codec实例是否有效, 可用于故障恢复                                            |

## 开发步骤

详细API说明请参考[avcodec native API](https://gitee.com/openharmony/multimedia_av_codec/blob/master/test/nativedemo/audio_demo/avcodec_audio_decoder_demo.h)

### 异步模式

1. 创建解码器实例对象

   ```c++
   //通过 codecname 创建解码器
   OH_AVCapability *capability = OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_MPEG, false);
   const char *name = OH_AVCapability_GetName(capability);
   OH_AVCodec *audioDec = OH_AudioDecoder_CreateByName(name);

   //通过 Mimetype 创建解码器
   OH_AVCodec *audioDec = OH_AudioDecoder_CreateByMime(OH_AVCODEC_MIMETYPE_AUDIO_MPEG);

   class ADecSignal {
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
   ADecSignal *signal_;
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
   // 解码输入码流送入InputBuffer 队列
   static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
   {
       (void)codec;
       ADecSignal *signal = static_cast<ADecSignal *>(userData);
       unique_lock<mutex> lock(signal->inMutex_);
       signal->inQueue_.push(index);
       signal->inBufferQueue_.push(data);
       signal->inCond_.notify_all();
       // 解码输入码流送入InputBuffer 队列
   }
   // 解码完成的PCM送入OutputBuffer队列
   static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                           void *userData)
   {
       (void)codec;
       ADecSignal *signal = static_cast<ADecSignal *>(userData);
       unique_lock<mutex> lock(signal->outMutex_);
       signal->outQueue_.push(index);
       signal->outBufferQueue_.push(data);
       if (attr) {
           signal->attrQueue_.push(*attr);
       }
       signal->outCond_.notify_all();
       // 将对应输出buffer的 index 送入OutputQueue_队列
       // 将对应解码完成的数据data送入outBuffer队列
   }
   signal_ = new ADecSignal();
   OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
   // 配置异步回调
   int32_t ret = OH_AudioDecoder_SetCallback(audioDec, cb, signal_);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
3. 设置采样率，码率，以及声道数（必须）、最大输入长度（可选）

   AAC 需要额外标识是否为adts类型否则会被认为是latm类型

   vorbis需要额外标识ID Header和Setup Header数据

   ```c++
   enum AudioFormatType : int32_t {
       TYPE_AAC = 0,
       TYPE_FLAC = 1,
       TYPE_MP3 = 2,
       TYPE_VORBIS = 3,
   };  
   // 设置解码分辨率
   int32_t ret;
   // 配置音频采样率（必须）
   constexpr uint32_t DEFAULT_SMAPLERATE = 44100; 
   // 配置音频码率（必须）
   constexpr uint32_t DEFAULT_BITRATE = 32000;
   // 配置音频声道数（必须）
   constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
   // 配置最大输入长度（可选）
   constexpr uint32_t DEFAULT_MAX_INPUT_SIZE = 1152;
   OH_AVFormat *format = OH_AVFormat_Create();
   // 写入format
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SMAPLERATE);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), DEFAULT_CHANNEL_COUNT);
   OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE.data(), DEFAULT_MAX_INPUT_SIZE);
   if (audioType == TYPE_AAC) {
       OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_AAC_IS_ADTS.data(), DEFAULT_AAC_TYPE);
   }
   if (audioType == TYPE_VORBIS) {
       OH_AVFormat_SetStringValue(format, MediaDescriptionKey::MD_KEY_IDENTIFICATION_HEADER.data(), DEFAULT_ID_HEADER);
       OH_AVFormat_SetStringValue(format, MediaDescriptionKey::MD_KEY_SETUP_HEADER.data(), DEFAULT_SETUP_HEADER);
   }
   // 配置解码器
   ret = OH_AudioDecoder_Configure(audioDec, format);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
4. prepare通知解码器已经就绪

   ```c++
   ret = OH_AudioDecoder_Prepare(audioDec);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
5. 启动解码器

   ```c++
   inputFile_ = std::make_unique<std::ifstream>();
   // 打开待解码二进制文件路径
   inputFile_->open(inputFilePath.data(), std::ios::in | std::ios::binary); 
   //配置解码文件输出路径
   outFile_ = std::make_unique<std::ofstream>();
   outFile_->open(outputFilePath.data(), std::ios::out | std::ios::binary);
   // 开始解码
   ret = OH_AudioDecoder_Start(audioDec);
   if (ret != AV_ERR_OK) {
      // 异常处理
   }
   ```
6. 写入解码码流

   ```c++
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
   int32_t ret = OH_AudioDecoder_PushInputData(audioDec, index, info);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
7. 输出PCM码流

   ```c++
   OH_AVCodecBufferAttr attr = signal_->attrQueue_.front();
   OH_AVMemory *data = signal_->outBufferQueue_.front();
   uint32_t index = signal_->outQueue_.front();
   // 将解码完成数据data写入到对应输出文件中
   outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
   // buffer 模式, 释放已完成写入的数据
   ret = OH_AudioDecoder_FreeOutputData(audioDec, index);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
8. 刷新解码器(在文件流EOS标识发送之后需要刷新或者碰到可以继续执行的错误的时候(OH_AudioDecoder_IsValid 为true)，然后启动可以重新继续解码)

   ```c++
   // 刷新解码器 audioDec
   ret = OH_AudioDecoder_Flush(audioDec);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   // 重新开始解码
   ret = OH_AudioDecoder_Start(audioDec);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```
9. 重置解码器(碰到不可以继续执行的错误的时候(OH_AudioDecoder_IsValid 为false)或者需要重新配置时)

   ```c++
   // 重置解码器 audioDec
   ret = OH_AudioDecoder_Reset(audioDec);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   // 重新配置解码器参数
   ret = OH_AudioDecoder_Configure(audioDec, format);
   if (ret != AV_ERR_OK) {
      // 异常处理
   }
   ```
10. 停止解码器(编码结束执行)

    ```c++
    // 终止解码器 audioDec
    ret = OH_AudioDecoder_Stop(audioDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return ret;
    ```
11. 注销解码器实例

    ```c++
    // 调用OH_AudioDecoder_Destroy, 注销解码器
    ret = OH_AudioDecoder_Destroy(audioDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    } else {
        audioDec = NULL; //不可重复destroy
    }
    return ret;
    ```
