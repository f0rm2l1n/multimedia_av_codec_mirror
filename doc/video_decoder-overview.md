# 视频软解能力开发概述

视频软解，简单来说，就是把视频编码后的媒体数据，解码成YUV文件或者送显。

目前系统API支持的解码能力如下：

| 封装格式 | 视频编解码类型        | 音频编解码类型   |
| -------- | --------------------- | ---------------- |
| mp4      | MPEG-4、 AVC（H.264） | AAC、MPEG（MP3） |
| m4a      | MPEG-4、 AVC（H.264） | AAC              |

**视频解码的使用场景**：

- 录像、录音
  
  保存录像、录音文件时，需要先对音视频流进行编码，然后封装送显

- 视频编辑
  
  保存编辑后的音视频文件，需要封装

- 视频转码

  转码后，保存文件时需要封装

# 使用avcodec开发视频解码功能

## 接口说明

**表1** avcodec开放能力接口

| 接口名称                                                     | 变更说明             |
| ------------------------------------------------------------ | -------------------- |
| OH_AVCodec \*OH_VideoDecoder_CreateByMime(const char \*mime); |  根据媒体类型创建解码器       |
| OH_AVCodec \*OH_VideoDecoder_CreateByName(const char \*name); |  根据名称创建解码器 |
| OH_AVErrCode OH_VideoDecoder_Destory(OH_AVCodec \*codec);     |  销毁已创建的解码器     |
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
| OH_AVErrCode OH_VideoDecoder_IsValid(OH_AVCodec \*codec, bool \*isValid);         |  查询当前codec实例是否有效, 可用于故障恢复       |


## 开发步骤

详细的API说明请参考avcodec native API
### 同步模式
1. 创建编解码器实例对象
   ``` c++
    // 通过 codecname 创建解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByName("video_decoder.avc");

    // 通过 mimetype 创建解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByMime("video/avc", false);
   ``` 

2. 设置解码分辨率（必须）

   ``` c++
    // 设置解码分辨率
    int32_t ret;
    constexpr uint32_t DEFAULT_WIDTH = 320;
    constexpr uint32_t DEFAULT_HEIGHT = 240;
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_WIDTH.data(), DEFAULT_WIDTH);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_HEIGHT.data(), DEFAULT_HEIGHT);
    ret = OH_VideoDecoder_Configure(videoDec, format);
   ```

3. 设置surface (surface模式下必须)
   ``` c++
    // 配置surface显示窗口信息
    OHNativeWindow *nativeWindow;
    ret = SetSurface(nativeWindow);
    ```  

4. 额外配置解码器实例 (仅支持surface模式)
    ``` c++
    // 配置PixelFormat模式
    VideoPixelFormat vpf = VideoPixelFormat::RGBA;
    // 配置显示旋转角度
    VideoRotation sr = VideoRotation::VIDEO_ROTATION_90;
    // 配置视频与显示屏匹配模式(缩放与显示窗口适配, 裁剪与显示窗口适配)
    ScalingMode scaleMode = ScalingMode::SCALING_MODE_SCALE_CROP;
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, vpf);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, sr);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SCALE_TYPE, scaleMode);
    ret = OH_VideoDecoder_SetParameter(videoDec, format);

5. 启动解码器
   ``` c++
    // 配置待解码文件路径
    inputFile_ = std::make_unique<std::ifstream>();
    inputFile_->open(inputFilePath.data(), std::ios::in | std::ios::binary);
    
    if(!isSurfaceMode_) {
        // buffer 模式: 配置解码文件输出路径
        outFile_ = std::make_unique<std::ofstream>();
        outFile_->open(outputFilePath.data(), std::ios::out | std::ios::binary);
    }
    // 启动解码器, 开始解码
    ret = OH_VideoDecoder_Start(videoDec);
    ```  

6. 写入解码码流
   ``` c++
    // 配置buffer info信息
    OH_AVCodecBufferAttr info;
    // 设置输入pkt尺寸、偏移量、时间戳等信息
    info.size = pkt_->size;
    info.offset = 0;
    info.pts = pkt_->pts;
    info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
    // 送入解码输入队列进行解码, index为对应队列下标
    int32_t ret = OH_VideoDecoder_PushInputData(videoDec, index, info);
    ```

7. 输出解码帧
   ``` c++
    // 将解码完成数据data写入到对应输出文件中
    outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
    // buffer 模式, 释放已完成写入的数据
    if (!isSurfaceMode_ && OH_VideoDecoder_FreeOutputData(videoDec, index) != AV_ERR_OK) {
        // 异常处理
    }
    // surface 模式, 对解码完成的数据进行送显
    if (isSurfaceMode_ && OH_VideoDecoder_RenderOutputData(videoDec, index) != AV_ERR_OK) {
        // 异常处理
    }
    ```

8. 刷新解码器
   ``` c++
    // 刷新解码器 videoDec
    if (OH_VideoDecoder_Flush(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    // 重新开始解码
    ret = OH_VideoDecoder_Start(videoDec);
    ```

9. 重置解码器
   ``` c++
    // 重置解码器 videoDec
    if (OH_VideoDecoder_Reset(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    // 重新配置解码器参数
    ret = OH_VideoDecoder_Configure(videoDec, format);
    ```

9. 停止解码器
   ``` c++
    // 终止解码器 videoDec
    if (OH_VideoDecoder_Stop(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
    ```
    
9. 注销解码器实例
   ``` c++
    // 调用OH_VideoDecoder_Destroy, 注销解码器
    if (OH_VideoDecoder_Destroy(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
    ```
    
### 异步模式
1. 创建编解码器实例对象

   ``` c++
    // 通过 codecname 创建解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByName("video_decoder.avc");

    // 通过 mimetype 创建解码器
    OH_AVCodec *videoDec = OH_VideoDecoder_CreateByMime("video/avc", false);
   ``` 

2. 设置回调函数（必须）
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
    // 解码输入帧送入InputBuffer 队列
    static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
    {
        (void)codec;
        // 解码输入帧送入InputBuffer 队列
    }
    // 解码完成帧送入OutputBuffer队列
    static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                        void *userData)
    {
        (void)codec;
        // 将对应输出buffer的 index 送入OutputQueue_队列
        // 将对应解码完成的数据data送入outBuffer队列
    }
    OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    // 配置异步回调
    int32_t ret = OH_VideoDecoder_SetCallback(videoDec, cb, userData);
   ```

3. 设置解码分辨率 (必须)
   ``` c++
    // 设置解码分辨率
    int32_t ret;
    // 配置视频帧宽度（必须）
    constexpr uint32_t DEFAULT_WIDTH = 320; 
    // 配置视频帧高度（必须）
    constexpr uint32_t DEFAULT_HEIGHT = 240;
    OH_AVFormat *format = OH_AVFormat_Create();
    // 写入format
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_WIDTH.data(), DEFAULT_WIDTH);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_HEIGHT.data(), DEFAULT_HEIGHT);
    // 配置解码器
    ret = OH_VideoDecoder_Configure(videoDec, format);
   ```

4. 设置surface (surface模式下必须)
   ``` c++
    // 配置送显窗口参数
    OHNativeWindow *window;
    ret = OH_VideoDecoder_SetSurface(videoDec, window);
    ```  

5. 额外配置解码器实例 (仅支持surface模式)
    ``` c++
    // 配置PixelFormat模式
    VideoPixelFormat vpf = VideoPixelFormat::RGBA;
    // 配置显示旋转角度
    VideoRotation sr = VideoRotation::VIDEO_ROTATION_90;
    // 配置视频与显示屏匹配模式(缩放与显示窗口适配, 裁剪与显示窗口适配)
    ScalingMode scaleMode = ScalingMode::SCALING_MODE_SCALE_CROP;
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, vpf);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, sr);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SCALE_TYPE, scaleMode);
    ret = OH_VideoDecoder_SetParameter(videoDec, format);

6. 启动解码器
   ``` c++
    inputFile_ = std::make_unique<std::ifstream>();
    // 打开待解码二进制文件路径
    inputFile_->open(inputFilePath.data(), std::ios::in | std::ios::binary); 
    // buffer模式下需要配置
    if(!isSurfaceMode_) {
        // buffer 模式: 配置解码文件输出路径
        outFile_ = std::make_unique<std::ofstream>();
        outFile_->open(outputFilePath.data(), std::ios::out | std::ios::binary);
    }
    // 开始解码
    ret = OH_VideoDecoder_Start(videoDec);
    ```

7. 写入解码码流
   ``` c++
    // 配置buffer info信息
    OH_AVCodecBufferAttr info;
    // 设置输入pkt尺寸、偏移量、时间戳等信息
    info.size = pkt_->size;
    info.offset = 0;
    info.pts = pkt_->pts;
    info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
    // 送入解码输入队列进行解码, index为对应队列下标
    int32_t ret = OH_VideoDecoder_PushInputData(videoDec, index, info);
    ```

8. 输出解码帧
   ``` c++
    // 将解码完成数据data写入到对应输出文件中
    outFile_->write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
    // buffer 模式, 释放已完成写入的数据
    if (!isSurfaceMode_ && OH_VideoDecoder_FreeOutputData(videoDec, index) != AV_ERR_OK) {
        // 异常处理
    }
    // surface 模式, 对解码完成的数据进行送显
    if (isSurfaceMode_ && OH_VideoDecoder_RenderOutputData(videoDec, index) != AV_ERR_OK) {
        // 异常处理
    }
    ```

9. 刷新解码器
   ``` c++
    // 刷新解码器 videoDec
    if (OH_VideoDecoder_Flush(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    // 重新开始解码
    ret = OH_VideoDecoder_Start(videoDec);
    ```

9. 重置解码器
   ``` c++
    // 重置解码器 videoDec
    if (OH_VideoDecoder_Reset(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    // 重新配置解码器参数
    ret = OH_VideoDecoder_Configure(videoDec, format);
    ```

9. 停止解码器
   ``` c++
    // 终止解码器 videoDec
    if (OH_VideoDecoder_Stop(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
    ```

9. 注销解码器实例
   ``` c++
    // 调用OH_VideoDecoder_Destroy, 注销解码器
    if (OH_VideoDecoder_Destroy(videoDec) != AV_ERR_OK) {
        // 异常处理
    }
    return AV_ERR_OK;
    ```






