# native_avmuxer.h


## 概述

声明用于音视频封装的Native API。

**Since:**

10

**相关模块:**

[AVMuxer](_a_v_muxer.md)


## 汇总


### 函数

| 名称 | 描述 | 
| -------- | -------- |
| [OH_AVMuxer_Create](_a_v_muxer.md#oh_avmuxer_create) (int32_t fd, OH_AVOutputFormat format) | 通过文件描述符fd和封装格式创建OH_AVMuxer实例。 | 
| [OH_AVMuxer_SetLocation](_a_v_muxer.md#oh_avmuxer_setlocation) (OH_AVMuxer \*muxer, float latitude, float longitude) | 设置输出文件的经纬度。 | 
| [OH_AVMuxer_SetRotation](_a_v_muxer.md#oh_avmuxer_setrotation) (OH_AVMuxer \*muxer, int32_t rotation) | 设置视频的旋转角度。 | 
| [OH_AVMuxer_AddTrack](_a_v_muxer.md#oh_avmuxer_addtrack) (OH_AVMuxer \*muxer, int32_t \*trackIndex, OH_AVFormat \*trackFormat) | 向封装器添加媒体轨。 | 
| [OH_AVMuxer_Start](_a_v_muxer.md#oh_avmuxer_start) (OH_AVMuxer \*muxer) | 开始封装。 | 
| [OH_AVMuxer_WriteSampleBuffer](_a_v_muxer.md#oh_avmuxer_writesamplebuffer) (OH_AVMuxer \*muxer, uint32_t trackIndex, uint8_t \*sampleBuffer, OH_AVCodecBufferAttr info) | 将数据写入封装器。 | 
| [OH_AVMuxer_Stop](_a_v_muxer.md#oh_avmuxer_stop) (OH_AVMuxer \*muxer) | OH_AVErrCode | 
| [OH_AVMuxer_Destroy](_a_v_muxer.md#oh_avmuxer_destroy) (OH_AVMuxer \*muxer) | OH_AVErrCode | 
