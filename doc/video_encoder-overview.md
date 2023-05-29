# 视频编码能力开发概述

视频编码，简单来说，就是把未压缩的音视频数据通过音视频编码算法压缩成音视频码流的过程

目前系统API支持的编码能力如下：

| 封装格式 | 视频编码类型        | 音频编码类型   |
| -------- | --------------------- | ---------------- |
| mp4      | HEVC（H.265）、 AVC（H.264） | AAC、MPEG（MP3） |
| m4a      | HEVC（H.265）、 AVC（H.264） | AAC |

**视频编码的使用场景**：

- 录制

  通过录制传入音视频数据，然后编码出对应格式的码流，最后封装成想要的格式

- 视频编辑

  编辑音视频数据后导出音视频文件的场景，需要编码成对应的音视频格式后再封装成文件

# 使用avcodec开发视频编码功能

## 接口说明

**表1** avcodec开放能力接口

| 接口名称                                                            | 描述                                      |
| -----------------------------------------------------------------  | ----------------------------------------- |
| OH_AVCodec \*OH_VideoEncoder_CreateByMime(const char \*mime);      |  根据媒体类型创建编码器                     |
| OH_AVCodec \*OH_VideoEncoder_CreateByName(const char \*name);      |  根据名称创建编码器                         |
| OH_AVErrCode OH_VideoEncoder_Destroy(OH_AVCodec \*codec);          |  销毁已创建的编码器                         |
| OH_AVErrCode OH_VideoEncoder_SetCallback(OH_AVCodec \*codec, OH_AVCodecAsyncCallback callback, void \*userData); |  对编码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知           |
| OH_AVErrCode OH_VideoEncoder_GetSurface(OH_AVCodec \*codec, OHNativeWindow \*\*window);           |  从视频编码器获取输入Surface             |
| OH_AVErrCode OH_VideoEncoder_Configure(OH_AVCodec \*codec, OH_AVFormat \*format); |  利用解封装得到的编码格式来配置编码器     |
| OH_AVErrCode OH_VideoEncoder_Prepare(OH_AVCodec \*codec);          |  Configure->Surface(可选)->Prepare->         |
| OH_AVErrCode OH_VideoEncoder_Start(OH_AVCodec \*codec);            |  启动已经配置好的编码器                      |
| OH_AVErrCode OH_VideoEncoder_Stop(OH_AVCodec \*codec);             |  停止编码器                                |
| OH_AVErrCode OH_VideoEncoder_Flush(OH_AVCodec \*codec);            |  刷新编码器的输入输出                      |
| OH_AVErrCode OH_VideoEncoder_Reset(OH_AVCodec \*codec);            |  重置编码器                               |
| OH_AVErrCode \*OH_VideoEncoder_GetOutputDescription(OH_AVCodec \*codec);         |  返回编码器输出的格式信息    |
| OH_AVFormat \*OH_VideoEncoder_GetInputDescription(OH_AVCodec \*codec);         |  获取视频编码器接受的描述信息，Configure后调用此接口，返回的OH_AVFormat\*需自行手动释放      |
| OH_AVErrCode OH_VideoEncoder_SetParameter(OH_AVCodec \*codec, OH_AVFormat \*format);            |  对编码器实例进行额外配置             |
| OH_AVErrCode OH_VideoEncoder_PushInputData(OH_AVCodec \*codec, uint32_t index, OH_AVCodecBufferAttr attr);         | 发送特定的buffer给编码器处理       |
| OH_AVErrCode OH_VideoEncoder_NotifyEndOfStream(OH_AVCodec \*codec);         |  通知视频编码器输入流已结束。建议使用此接口进行通知       |
| OH_AVErrCode OH_VideoEncoder_FreeOutputData(OH_AVCodec \*codec, uint32_t index);         |  返回已完成编码的编码器输出数据       |
| OH_AVErrCode OH_VideoEncoder_IsValid(OH_AVCodec \*codec, bool \*isValid);         |  查询当前codec实例是否有效       |

## 开发步骤

详细的API说明请参考avcodec native API

### 异步模式

#### 1. 创建编编码器实例对象

   ``` c++
    // 通过codecname创建编码器
    OH_AVCodec *videoEnc = OH_VideoEncoder_CreateByName("video_encoder.avc");

    // 通过mimetype创建编码器
    OH_AVCodec *videoEnc = OH_VideoEncoder_CreateByMime("video/avc", true);
   ```

#### 2. 设置回调函数（必须）

   ``` c++
    // 设置OnError回调函数
    static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
    {
        (void)codec;
        (void)errorCode;
        (void)userData;
    }
    // 设置FormatChange回调函数
    static void OnOutputFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
    {
        (void)codec;
        (void)format;
        (void)userData;
    }
    // 编码输入帧送入InputBuffer队列
    static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
    {
        (void)codec;
        // 编码输入帧送入InputBuffer队列（surface输入不需要配置）
    }
    // 编码完成帧送入OutputBuffer队列
    static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                        void *userData)
    {
        (void)codec;
        // 将对应输出buffer的index送入OutputQueue队列
        // 将对应编码完成的数据data送入outBuffer队列
    }
    OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    // 配置异步回调
    int32_t ret = OH_VideoEncoder_SetCallback(videoEnc, cb, userData);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 3. 配置编码器格式（必须）

   ``` c++
    // 配置视频帧宽度（必须）
    constexpr uint32_t DEFAULT_WIDTH = 320; 
    // 配置视频帧高度（必须）
    constexpr uint32_t DEFAULT_HEIGHT = 240;
    // 配置视频像素格式（必须）
    constexpr VideoPixelFormat DEFAULT_PIXELFORMAT = VideoPixelFormat::RGBA;
    OH_AVFormat *format = OH_AVFormat_Create();
    // 写入format
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_WIDTH.data(), DEFAULT_WIDTH);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_HEIGHT.data(), DEFAULT_HEIGHT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_PIXEL_FORMAT.data(), DEFAULT_PIXELFORMAT);
    // 配置编码器
    int32_t ret = OH_VideoEncoder_Configure(videoEnc, format);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 4. 获取surface（surface模式下必须）

   ``` c++
    // 获取需要输入的surface，以进行编码
    OHNativeWindow *nativeWindow;
    int32_t ret =  OH_VideoEncoder_GetSurface(videoEnc, &nativeWindow);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    // 配置传入surface的数据
   ```  

#### 5. 额外配置编码器实例（仅支持surface模式）

   ``` c++
    OH_AVFormat *format = OH_AVFormat_Create();
    // 配置视频帧速率
    double frameRate = 30.0;
    // 配置视频YUV值范围标志
    bool rangeFlag = false;
    // 配置视频原色
    int32_t primary = static_cast<int32_t>(OH_ColorPrimary::COLOR_PRIMARY_BT709);
    // 配置传输特性
    int32_t transfer = static_cast<int32_t>(OH_TransferCharacteristic::TRANSFER_CHARACTERISTIC_BT709);
    // 配置最大矩阵系数
    int32_t matrix = static_cast<int32_t>(OH_MaxtrixCoefficient::MATRIX_COFFICIENT_IDENTITY);
    // 配置编码Profile
    int32_t profile = static_cast<int32_t>(AVCProfile::AVC_PROFILE_BASELINE);
    // 配置编码比特率模式
    int32_t rateMode = static_cast<int32_t>(VideoEncodeBitrateMode::CBR);
    // 配置关键帧的间隔，单位为毫秒。
    int32_t iFrameInterval = 23000;
    // 配置所需的编码质量。只有在恒定质量模式下配置的编码器才支持此配置
    int32_t quality = 0;
    // 配置比特率
    int64_t bitRate = 300000000;
    // 写入format
    OH_AVFormat_SetDoubleValue(format, MediaDescriptionKey::MD_KEY_FRAME_RATE, frameRate);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_RANGE_FLAG, rangeFlag);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_COLOR_PRIMARIES, primary);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_TRANSFER_CHARACTERISTICS, transfer);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_MATRIX_COEFFICIENTS, matrix);
    
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_I_FRAME_INTERVAL, iFrameInterval);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_PROFILE, profile);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_VIDEO_ENCODE_BITRATE_MODE, rateMode);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE, bitRate);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_QUALITY, quality);

    int32_t ret = OH_VideoEncoder_SetParameter(videoEnc, format);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 6. 启动编码器

   ``` c++
    // 配置待编码文件路径
    string_view inputFilePath = "/*yourpath*.yuv";
    string_view outputFilePath = "/*yourpath*.mp4";
    std::unique_ptr<std::ifstream> inputFile = nullptr;
    std::unique_ptr<std::ofstream> outputFile = std::make_unique<std::ofstream>();
    outputFile->open(outputFilePath.data(), std::ios::out | std::ios::binary);
    if(!isSurfaceMode) {
        // buffer 模式：配置编码文件输入路径
        inputFile = std::make_unique<std::ifstream>();
        inputFile->open(inputFilePath.data(), std::ios::in | std::ios::binary);
    }
    // 启动编码器，开始编码
    int32_t ret = OH_VideoEncoder_Start(videoEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 7. 写入编码码流

   ``` c++
    // 配置buffer info信息
    OH_AVCodecBufferAttr info;
    // 调用Ffmpeg接口av_packet_alloc进行初始化并返回一个容器pkt
    AVPacket pkt = av_packet_alloc();
    // 配置info的输入尺寸、偏移量、时间戳等字段信息
    info.size = pkt->size;
    info.offset = 0;
    info.pts = pkt->pts;
    info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
    // buffer 模式：送入编码输入队列进行编码,index为对应队列下标
    if(!isSurfaceMode) {
        int32_t ret = OH_VideoEncoder_PushInputData(videoEnc, index, info);
        if (ret != AV_ERR_OK) {
            // 异常处理
        }
    }
   ```

#### 8. 输出编码帧

   ``` c++
    int32_t ret;
    // 将编码完成数据data写入到对应输出文件中
    outFile->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), data.size);
    // buffer/surface 模式：释放已完成写入的数据,index为对应surface/buffer队列下标
    ret = OH_VideoEncoder_FreeOutputData(videoEnc, index);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 9. 刷新编码器

   ``` c++
    int32_t ret;
    // 刷新编码器videoEnc
    ret = OH_VideoEncoder_Flush(videoEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    // 重新开始编码
    ret = OH_VideoEncoder_Start(videoEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 10. 重置编码器

   ``` c++
    int32_t ret;
    // 重置编码器videoEnc
    ret = OH_VideoEncoder_Reset(videoEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    // 重新配置编码器参数
    ret = OH_VideoEncoder_Configure(videoEnc, format);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 11. 停止编码器

   ``` c++
    int32_t ret;
    // 终止编码器videoEnc
    ret = OH_VideoEncoder_Stop(videoEnc);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
   ```

#### 12. 注销编码器实例

   ``` c++
    int32_t ret;
    // 调用OH_VideoEncoder_Destroy，注销编码器
    ret = OH_VideoEncoder_Destroy(videoEnc);
    videoEnc = nullptr;
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
   ```
