# Native API 差异报告

OpenHarmony 4.0.8.1 版本相较于OpenHarmony 之前的版本的API变更如下:

## 标准系统接口变更

| 模块名称 | 接口名称                                                     | 变更类型 | 变更说明         |
| -------- | ------------------------------------------------------------ | -------- | ---------------- |
| avmuxer  | OH_AVMuxer \*OH_AVMuxer_Create(int32_t fd, OH_AVOutputFormat format); | 新增     | 创建OH_AVMuxer   |
| avmuxer  | OH_AVErrCode OH_AVMuxer_SetRotation(OH_AVMuxer \*muxer, int32_t rotation); | 新增     | 设置视频旋转角度 |
| avmuxer  | OH_AVErrCode OH_AVMuxer_AddTrack(OH_AVMuxer \*muxer, int32_t \*trackIndex, OH_AVFormat \*trackFormat); | 新增     | 添加媒体轨       |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Start(OH_AVMuxer \*muxer);           | 新增     | 开始封装         |
| avmuxer  | OH_AVErrCode OH_AVMuxer_WriteSample(OH_AVMuxer *muxer, uint32_t trackIndex, OH_AVMemory *sample, OH_AVCodecBufferAttr info); | 新增     | 将数据写入封装器 |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Stop(OH_AVMuxer \*muxer);            | 新增     | 停止封装         |
| avmuxer  | OH_AVErrCode OH_AVMuxer_Destroy(OH_AVMuxer \*muxer);         | 新增     | 销毁OH_AVMuxer   |
| avsource  | OH_AVSource *OH_AVSource_CreateWithURI(char *uri);         | 新增     | 根据 URI 创建 OH_AVSource       |
| avsource  | OH_AVSource *OH_AVSource_CreateWithFD(int32_t fd, int64_t offset, int64_t size);         | 新增     | 根据 FD 创建OH_AVSource       |
| avsource  | OH_AVErrCode OH_AVSource_Destroy(OH_AVSource *source);         | 新增     | 销毁 OH_AVSource       |
| avsource  | OH_AVFormat *OH_AVSource_GetSourceFormat(OH_AVSource *source);         | 新增     | 获取 source 信息       |
| avsource  | OH_AVFormat OH_AVSource_GetTrackFormat(OH_AVSource *source, uint32_t trackCount);         | 新增     | 获取 track 信息       |
| avdemuxer  | OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);         | 新增     | 根据 source 创建 OH_AVDemuxer       |
| avdemuxer  | OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer);         | 新增     | 销毁 OH_AVDemuxer       |
| avdemuxer  | OH_AVErrCode OH_AVDemuxer_SelectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);         | 新增     | 选择需要解封装的轨道      |
| avdemuxer  | OH_AVErrCode OH_AVDemuxer_UnselectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);         | 新增     | 取消选择需要解封装的轨道       |
| avdemuxer  | OH_AVErrCode OH_AVDemuxer_ReadSample(OH_AVDemuxer *demuxer, uint32_t trackIndex, OH_AVMemory *sample, OH_AVCodecBufferAttr *info);         | 新增     | 读取 trackIndex 对应轨道的帧     |
| avdemuxer  | OH_AVErrCode OH_AVDemuxer_SeekToTime(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode);         | 新增     | 跳转到指定时间       |
