# Native API 差异报告

OpenHarmony 4.0.8.1 版本相较于OpenHarmony 之前的版本的API变更如下:

## 标准系统接口变更

| 模块名称 | 接口名称                                                     | 变更类型 | 变更说明             |
| -------- | ------------------------------------------------------------ | -------- | -------------------- |
| avmuxer  | OH_AVMuxer \*OH_AVMuxer_Create(int32_t fd, OH_AVOutputFormat format); | 新增     | 创建OH_AVMuxer       |
| avmuxer  | OH_AVErrCode OH_AVMuxer_SetLocation(OH_AVMuxer \*muxer, float latitude, float longitude); | 新增     | 设置输出文件的经纬度 |
| avmuxer  | OH_AVErrCode OH_AVMuxer_SetRotation(OH_AVMuxer \*muxer, int32_t rotation); | 新增     | 设置视频旋转角度     |
| avmuxer  | OH_AVErrCode OH_AVMuxer_AddTrack(OH_AVMuxer \*muxer, int32_t \*trackIndex, OH_AVFormat \*trackFormat); | 新增     | 添加媒体轨           |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Start(OH_AVMuxer \*muxer);           | 新增     | 开始封装             |
| avmuxer  | OH_AVErrCode OH_AVMuxer_WriteSampleBuffer(OH_AVMuxer \*muxer, uint32_t trackIndex, uint8_t \*sampleBuffer, OH_AVCodecBufferAttr info); | 新增     | 将数据写入封装器     |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Stop(OH_AVMuxer \*muxer);            | 新增     | 停止封装             |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Destroy(OH_AVMuxer \*muxer);         | 新增     | 销毁OH_AVMuxer       |
| avcodec  | OH_AVCodec \*OH_VideoDecoder_CreateByMime(const char \*mime); | 新增     | 根据媒体类型创建解码器       |
| avcodec  | OH_AVCodec \*OH_VideoDecoder_CreateByName(const char \*name); | 新增     | 根据名称创建解码器 |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Destory(OH_AVCodec \*codec);     | 新增     | 销毁已创建的解码器     |
| avcodec  | OH_AVErrCode OH_VideoDecoder_SetCallback(OH_AVCodec \*codec, OH_AVCodecAsyncCallback callback, void \*userData); | 新增     | 对解码器事件设置异步回调函数，包括异常通知、输出格式变更通知、输入输出数据可获取通知           |
| avcodec  | OH_AVErrCode OH_VideoDecoder_SetSurface(OH_AVCodec \*codec, OHNativeWindow \*window);           | 新增     | 动态设置解码器的输出Surface             |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Configure(OH_AVCodec \*codec, OH_AVFormat \*format); | 新增     | 利用解封装得到的解码格式来配置解码器     |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Prepare(OH_AVCodec \*codec);            | 新增     | Configure->Surface(可选)->Prepare->             |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Start(OH_AVCodec \*codec);         | 新增     | 启动已经配置好的解码器       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Stop(OH_AVCodec \*codec);            | 新增     | 停止解码器             |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Flush(OH_AVCodec \*codec);         | 新增     | 刷新解码器的输入输出       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_Reset(OH_AVCodec \*codec);            | 新增     | 重置解码器             |
| avcodec  | OH_AVErrCode \*OH_VideoDecoder_GetOutputDescription(OH_AVCodec \*codec);         | 新增     | 返回解码器输出的格式信息       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_SetParameter(OH_AVCodec \*codec, OH_AVFormat \*format);            | 新增     | 对解码器实例进行额外配置             |
| avcodec  | OH_AVErrCode OH_VideoDecoder_PushInputData(OH_AVCodec \*codec, uint32_t index, OH_AVCodecBufferAttr attr);         | 新增     | 发送特定的buffer给解码器处理       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_RenderOutputData(OH_AVCodec \*codec, uint32_t index);         | 新增     | 返回已完成解码的解码器输出数据并送显       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_FreeOutputData(OH_AVCodec \*codec, uint32_t index);         | 新增     | 返回已完成解码的解码器输出数据       |
| avcodec  | OH_AVErrCode OH_VideoDecoder_IsValid(OH_AVCodec \*codec, bool \*isValid);         | 新增     | 查询当前codec实例是否有效, 可用于故障恢复       |
