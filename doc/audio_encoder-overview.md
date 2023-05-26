# 音频编码能力开发概述

音频编码，简单来说，就是把音频PCM，编码成不同的格式。

目前系统API支持的编码能力如下：

| 封装格式 | 音频编码类型 |
| -------- | :----------- |
| mp4      | AAC、Flac    |
| m4a      | AAC          |
| flac     | Flac         |
| aac      | AAC          |

**音频编码的使用场景**

- 录制

  通过录制传入PCM，然后编码出对应格式的码流，最后封装成想要的格式

- 音频编辑

  PCM编辑之后需要到处音频进行分发，需要编码成不同的格式进行分发

- 

# 使用avcodec开发音频编码功能

## 接口说明

**表1** avcodec 音频编码开放能力接口

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| OH_AVCodec \*OH_AudioEncoder_CreateByMime(const char \*mime); | 根据媒体类型创建编码器                                       |
| OH_AVCodec \*OH_AudioEncoder_CreateByName(const char \*name); | 根据名称创建编码器                                           |
| OH_AVErrCode OH_AudioEncoder_Destroy(OH_AVCodec \*codec);    | 销毁已创建的编码器                                           |
| OH_AVErrCode OH_AudioEncoder_SetCallback(OH_AVCodec \*codec, OH_AVCodecAsyncCallback callback, void \*userData); | 对编码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知 |
| OH_AVErrCode OH_AudioEncoder_Configure(OH_AVCodec \*codec, OH_AVFormat \*format); | 利用解封装得到的解码格式信息来配置编码器                     |
| OH_AVErrCode OH_AudioEncoder_Prepare(OH_AVCodec \*codec);    | 告知编码器已经准备好,Configure->Prepare->Start               |
| OH_AVErrCode OH_AudioEncoder_Start(OH_AVCodec \*codec);      | 启动已经配置好的编码器                                       |
| OH_AVErrCode OH_AudioEncoder_Stop(OH_AVCodec \*codec);       | 停止编码器                                                   |
| OH_AVErrCode OH_AudioEncoder_Flush(OH_AVCodec \*codec);      | 刷新编码器的输入输出                                         |
| OH_AVErrCode OH_AudioEncoder_Reset(OH_AVCodec \*codec);      | 重置编码器                                                   |
| OH_AVFormat *OH_AudioEncoder_GetOutputDescription(OH_AVCodec \*codec); | 返回编码器输出的格式信息                                     |
| OH_AVErrCode OH_AudioEncoder_SetParameter(OH_AVCodec \*codec, OH_AVFormat \*format); | 对编码器实例进行动态配置                                     |
| OH_AVErrCode OH_AudioEncoder_PushInputData(OH_AVCodec \*codec, uint32_t index, OH_AVCodecBufferAttr attr); | 发送特定的buffer给编码器处理                                 |
| OH_AVErrCode OH_AudioEncoder_FreeOutputData(OH_AVCodec \*codec, uint32_t index); | 返回已完成解码的编码器输出数据                               |
| OH_AVErrCode OH_AudioEncoder_IsValid(OH_AVCodec \*codec, bool \*isValid); | 查询当前codec实例是否有效, 可用于故障恢复                    |

## 开发步骤

详细API说明请参考avcodec native API

### 异步模式

1. 创建编码器实例对象

``` c++
//通过 codecname 创建编码器
OH_AVCodec *audioEnc = OH_AudioEncoder_CreateByName("OH.Media.Codec.Encoder.Audio.AAC");

//通过 codecname 创建编码器
OH_AVCodec *audioEnc = OH_AudioEncoder_CreateByMime("audio/mp4a-lamt");
```

2. 设置回调函数(必须)

   ``` c++
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
       // 解码输入PCM送入InputBuffer 队列
       static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
       {
           (void)codec;
           // 解码输入码流送入InputBuffer 队列
       }
       // 解码完成的码流送入OutputBuffer队列
       static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                           void *userData)
       {
           (void)codec;
           // 将对应输出buffer的 index 送入OutputQueue_队列
           // 将对应解码完成的数据data送入outBuffer队列
       }
       OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
       // 配置异步回调
       int32_t ret = OH_AudioEncoder_SetCallback(audioEnc, cb, userData);
   ```

3. 设置采样率，码率，以及声道数（必须）、最大输入长度（可选）

           // 设置解码分辨率
           int32_t ret;
           // 配置音频采样率（必须）
           constexpr uint32_t DEFAULT_SMAPLERATE = 44100; 
           // 配置音频码率（必须）
           constexpr uint32_t DEFAULT_BITRATE = 32000;
           // 配置音频声道数（必须）
           constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
           // 配置最大输入长度（可选）
           constexpr uint32_t DEFAULT_MAX_INPUT_SIZE = 1152*2;
           OH_AVFormat *format = OH_AVFormat_Create();
           // 写入format
           OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SMAPLERATE);
           OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);
           OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), DEFAULT_CHANNEL_COUNT);
           OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE.data(), DEFAULT_MAX_INPUT_SIZE);
           // 配置编码器
           ret = OH_AudioEncoder_Configure(audioEnc, format);
           if (ret != AV_ERR_OK) {
                   // 异常处理
           }

　４.　prepare通知编码器已经就绪

``` c++
    OH_AudioEncoder_Prepare(audioEnc);
```

​    5.　启动编码器

``` c++
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

​    6.　写入PCM码流

``` c++
    // 配置buffer info信息
    OH_AVCodecBufferAttr info;
    // 设置输入pkt尺寸、偏移量、时间戳等信息
    info.size = pkt_->size;
    info.offset = 0;
    info.pts = pkt_->pts;
    info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
    // 送入解码输入队列进行解码, index为对应队列下标
    int32_t ret = OH_AudioEncoder_PushInputData(audioEnc, index, info);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
```

      7.　输出编码格式码流

``` c++
    // 将解码完成数据data写入到对应输出文件中
    outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
    // buffer 模式, 释放已完成写入的数据
    ret = OH_AudioEncoder_FreeOutputData(audioEnc, index);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
```

      8.　刷新编码器

``` c++
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

      9.　重置编码器

``` c++
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

      10.　停止编码器

``` c++
    // 终止编码器 audioEnc
    ret = OH_AudioEncoder_Stop(audioEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return ret;
```

      11.　注销编码器实例

``` c++
   // 调用OH_AudioEncoder_Destroy, 注销编码器
    ret = OH_AudioEncoder_Destroy(audioEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return ret;
```

