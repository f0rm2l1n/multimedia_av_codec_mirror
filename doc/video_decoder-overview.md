# 视频软硬件解码能力开发概述

视频解码，简单来说，就是把视频编码后的媒体数据，解码成YUV文件或者送显。

目前系统API支持的解码能力如下：

| 封装格式 | 视频编解码类型        | 音频编解码类型   |
| -------- | --------------------- | ---------------- |
| mp4      | MPEG-4、 AVC（H.264） | AAC、MPEG（MP3） |
| m4a      | MPEG-4、 AVC（H.264） | AAC              |

**视频解码的使用场景**：

- 视频解码
  
  将码流解码成yuv



# 使用avcodec开发视频解码功能

## 接口说明

**表1** avcodec开放能力接口

| 接口名称                                                     | 描述             |
| ------------------------------------------------------------ | -------------------- |
| OH_AVCodec \*OH_VideoDecoder_CreateByMime(const char \*mime); |  根据媒体类型创建解码器       |
| OH_AVCodec \*OH_VideoDecoder_CreateByName(const char \*name); |  根据名称创建解码器 |
| OH_AVErrCode OH_VideoDecoder_Destroy(OH_AVCodec \*codec);     |  销毁已创建的解码器     |
| OH_AVErrCode OH_VideoDecoder_SetCallback(OH_AVCodec \*codec, OH_AVCodecAsyncCallback callback, void \*userData); |  对解码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知           |
| OH_AVErrCode OH_VideoDecoder_SetSurface(OH_AVCodec \*codec, OHNativeWindow \*window);           |  动态设置解码器的输出Surface             |
| OH_AVErrCode OH_VideoDecoder_Configure(OH_AVCodec \*codec, OH_AVFormat \*format); |  利用解封装得到的解码格式来配置解码器     |
| OH_AVErrCode OH_VideoDecoder_Prepare(OH_AVCodec \*codec);            |  Configure->Surface(可选)->Prepare->             |
| OH_AVErrCode OH_VideoDecoder_Start(OH_AVCodec \*codec);         |  启动已经配置好的解码器       |
| OH_AVErrCode OH_VideoDecoder_Stop(OH_AVCodec \*codec);            |  停止解码器             |
| OH_AVErrCode OH_VideoDecoder_Flush(OH_AVCodec \*codec);         |  刷新解码器的输入输出       |
| OH_AVErrCode OH_VideoDecoder_Reset(OH_AVCodec \*codec);            |  重置解码器             |
| OH_AVErrCode \*OH_VideoDecoder_GetOutputDescription(OH_AVCodec \*codec);         |  返回解码器输出的格式信息       |
| OH_AVErrCode OH_VideoDecoder_SetParameter(OH_AVCodec \*codec, OH_AVFormat \*format);            |  对解码器实例进行额外配置             |
| OH_AVErrCode OH_VideoDecoder_PushInputData(OH_AVCodec \*codec, uint32_t index, OH_AVCodecBufferAttr attr);         | 发送特定的buffer给解码器处理       |
| OH_AVErrCode OH_VideoDecoder_RenderOutputData(OH_AVCodec \*codec, uint32_t index);         |  返回已完成解码的解码器输出数据并送显       |
| OH_AVErrCode OH_VideoDecoder_FreeOutputData(OH_AVCodec \*codec, uint32_t index);         |  返回已完成解码的解码器输出数据       |
| OH_AVErrCode OH_VideoDecoder_IsValid(OH_AVCodec \*codec, bool \*isValid);         |  查询当前codec实例是否有效       |

#### 软/硬件解码的异同点：
- 相同点：软件解码与硬件解码接口保持一致
- 差异点：基于MimeType创建解码器时，软解当前仅支持 H264 ("video/avc")，硬解则支持 H264 ("video/avc") 和 H265 ("video/hevc")

## 开发步骤

详细的API说明请参考avcodec native API



#### 1. 创建编解码器实例对象
   ``` c++
    // 通过 codecname 创建解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByName("video_decoder.avc");

    // 通过 mimetype 创建解码器
    // 软/硬解: 创建 H264 解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByMime("video/avc", false);
    // 硬解: 创建 H265 解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByMime("video/hevc", false);
   ``` 

#### 2. 设置回调函数（必须）
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
    // 解码输入帧送入 InputBuffer 队列
    static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
    {
        (void)codec;
        // 解码输入帧送入 InputBuffer 队列
    }
    // 解码完成帧送入 OutputBuffer 队列
    static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                        void *userData)
    {
        (void)codec;
        // 将对应输出 buffer 的 index 送入 OutputQueue 队列
        // 将对应解码完成的数据 data 送入 outBuffer 队列
    }
    OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    // 配置异步回调
    int32_t ret = OH_VideoDecoder_SetCallback(videoDec, cb, userData);
   ```

#### 3. 设置解码分辨率 (必须)
   ``` c++
    // 配置视频帧宽度（必须）
    constexpr uint32_t DEFAULT_WIDTH = 320; 
    // 配置视频帧高度（必须）
    constexpr uint32_t DEFAULT_HEIGHT = 240;
    OH_AVFormat *format = OH_AVFormat_Create();
    // 写入 format
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_WIDTH.data(), DEFAULT_WIDTH);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_HEIGHT.data(), DEFAULT_HEIGHT);
    // 配置解码器
    int32_t ret = OH_VideoDecoder_Configure(videoDec, format);
   ```

#### 4. 设置surface ( surface 模式下必须)
   ``` c++
    // 配置送显窗口参数
    OHNativeWindow *window;
    int32_t ret = OH_VideoDecoder_SetSurface(videoDec, window);
   ```  

#### 5. 额外配置解码器实例 (仅支持surface模式)
   ``` c++

    Format format;
    // 配置 PixelFormat 模式
    VideoPixelFormat pixelFormat = VideoPixelFormat::RGBA;
    // 配置显示旋转角度
    VideoRotation rotation = VideoRotation::VIDEO_ROTATION_90;
    // 配置视频与显示屏匹配模式(缩放与显示窗口适配, 裁剪与显示窗口适配)
    ScalingMode scaleMode = ScalingMode::SCALING_MODE_SCALE_CROP;
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, pixelFormat);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, rotation);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SCALE_TYPE, scaleMode);
    int32_t ret = OH_VideoDecoder_SetParameter(videoDec, format);
   ``` 

#### 6. 启动解码器
   ``` c++
    string_view outputFilePath = "/*yourpath*.yuv";
    std::unique_ptr<std::ifstream> inputFile = std::make_unique<std::ifstream>();
    // 打开待解码二进制文件路径
    inputFile->open(inputFilePath.data(), std::ios::in | std::ios::binary); 
    // buffer 模式下需要配置
    if(!isSurfaceMode) {
        // buffer 模式: 配置解码文件输出路径
        std::unique_ptr<std::ofstream> outFile = std::make_unique<std::ofstream>();
        outFile->open(outputFilePath.data(), std::ios::out | std::ios::binary);
    }
    // 开始解码
    int32_t ret = OH_VideoDecoder_Start(videoDec);
   ```

#### 7. 写入解码码流
   ``` c++
    // 配置 buffer info 信息
    OH_AVCodecBufferAttr info;
    // 调用 Ffmpeg 接口 av_packet_alloc 进行初始化并返回一个容器 pkt
    AVPacket pkt = av_packet_alloc();
    // 配置 info 的输入尺寸、偏移量、时间戳等字段信息
    info.size = pkt->size;
    info.offset = 0;
    info.pts = pkt->pts;
    info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
    // 送入解码输入队列进行解码, index 为对应队列下标
    int32_t ret = OH_VideoDecoder_PushInputData(videoDec, index, info);
   ```

#### 8. 输出解码帧
   ``` c++
    int32_t ret;
    // 将解码完成数据 data 写入到对应输出文件中
    outFile->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), data.size);
    // buffer 模式, 释放已完成写入的数据, index 为对应 surface/buffer 队列下标
    if (isSurfaceMode) {
        ret = OH_VideoDecoder_RenderOutputData(videoDec, index);
    } else {
        ret = OH_VideoDecoder_FreeOutputData(videoDec, index);
    }
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
   ```

#### 9. 刷新解码器
   ``` c++
    int32_t ret;
    // 刷新解码器 videoDec
    ret = OH_VideoDecoder_Flush(videoDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    // 重新开始解码
    ret = OH_VideoDecoder_Start(videoDec);
   ```

#### 10. 重置解码器
   ``` c++
    int32_t ret;
    // 重置解码器 videoDec
    ret = OH_VideoDecoder_Reset(videoDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    // 重新配置解码器参数
    ret = OH_VideoDecoder_Configure(videoDec, format);
   ```

#### 11. 停止解码器
   ``` c++
    int32_t ret;
    // 终止解码器 videoDec
    ret = OH_VideoDecoder_Stop(videoDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
   ```

#### 12. 注销解码器实例
   ``` c++
    int32_t ret;
    // 调用 OH_VideoDecoder_Destroy, 注销解码器
    ret = OH_VideoDecoder_Destroy(videoDec);
    if (ret != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
   ```






