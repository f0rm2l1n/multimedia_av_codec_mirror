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
| avcodec  | OH_AVErrCode OH_VideoDecoder_IsValid(OH_AVCodec \*codec, bool \*isValid);         | 新增     | 查询当前codec实例是否有效      |
