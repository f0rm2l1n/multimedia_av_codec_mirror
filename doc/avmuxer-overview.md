# 音视频封装能力开发概述

封装，简单来说，就是把音频、视频等编码后的媒体数据，按一定的格式存储到文件里。

一般来说，不同的封装格式，支持封装的编解码类型不同。目前系统API支持的封装能力如下：

| 封装格式 | 视频编解码类型        | 音频编解码类型   |
| -------- | --------------------- | ---------------- |
| mp4      | MPEG-4、 AVC（H.264） | AAC、MPEG（MP3） |
| m4a      | MPEG-4、 AVC（H.264） | AAC              |

**音视频封装的使用场景**：

- 录像、录音
  
  保存录像、录音文件时，需要先对音视频流进行编码，然后封装

- 音视频编辑
  
  保存编辑后的音视频文件，需要封装

- 音视频转码

  转码后，保存文件时需要封装

# 使用avmuxer开发音视频封装功能

## 接口说明

**表1** avmuxer开放能力接口

| 接口名                                                       | 描述                 |
| ------------------------------------------------------------ | -------------------- |
| OH_AVMuxer \*OH_AVMuxer_Create(int32_t fd, OH_AVOutputFormat format); | 创建OH_AVMuxer       |
| OH_AVErrCode OH_AVMuxer_SetLocation(OH_AVMuxer \*muxer, float latitude, float longitude); | 设置输出文件的经纬度 |
| OH_AVErrCode OH_AVMuxer_SetRotation(OH_AVMuxer \*muxer, int32_t rotation); | 设置视频旋转角度     |
| OH_AVErrCode OH_AVMuxer_AddTrack(OH_AVMuxer \*muxer, int32_t \*trackIndex, OH_AVFormat \*trackFormat); | 添加媒体轨           |
| OH_AVErrCode OH_AVMuxer_Start(OH_AVMuxer \*muxer);           | 开始封装             |
| OH_AVErrCode OH_AVMuxer_WriteSampleBuffer(OH_AVMuxer \*muxer, uint32_t trackIndex, uint8_t \*sampleBuffer, OH_AVCodecBufferAttr info); | 将数据写入封装器     |
| OH_AVErrCode OH_AVMuxer_Stop(OH_AVMuxer \*muxer);            | 停止封装             |
| OH_AVErrCode OH_AVMuxer_Destroy(OH_AVMuxer \*muxer);         | 销毁OH_AVMuxer       |

## 开发步骤

详细的API说明请参考avmuxer native API

1. 创建编解码器实例对象

   ``` c++
   // 设置封装格式为mp4
   OH_AVOutputFormat format = AV_OUTPUT_FORMAT_MPEG_4;
   // 以读写方式创建fd
   int32_t fd = open("test.mp4", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
   OH_AVMuxer *muxer = OH_AVMuxer_Create(fd, OH_AVOutputFormat format);
   ```



2. 设置经纬度、旋转角度（非必须）

   ``` c++
   // 经纬度
   OH_AVMuxer_SetLocation(muxer, 10, 10);
   // 旋转角度
   OH_AVMuxer_SetRotation(muxer, 0);
   ```

   

3. 添加音频轨

   ``` c++
   int audioTrackId = -1;
   OH_AVFormat *formatAudio = OH_AVFormat_Create();
   OH_AVFormat_SetStringValue(formatAudio, OH_MD_KEY_CODEC_MIME, OH_AVCODEC_MIMETYPE_AUDIO_AAC); // 必填
   OH_AVFormat_SetIntValue(formatAudio, OH_MD_KEY_AUD_SAMPLE_RATE, 44100); // 必填
   OH_AVFormat_SetIntValue(formatAudio, OH_MD_KEY_AUD_CHANNEL_COUNT, 2); // 必填
   
   int ret = OH_AVMuxer_AddTrack(muxer, &audioTrackId, formatAudio);
   if (ret != AV_ERR_OK || audioTrackId < 0) {
       // 音频轨添加失败
   }
   ```

   

4. 添加视频轨

   ``` c++
   int videoTrackId = -1;
   char *buffer = ...; // 编码config data，如果没有可以不传
   size_t size = ...;   // 编码config data的长度，根据实际情况配置
   OH_AVFormat *formatVideo = OH_AVFormat_Create();
   OH_AVFormat_SetStringValue(formatVideo, OH_MD_KEY_CODEC_MIME, OH_AVCODEC_MIMETYPE_VIDEO_MPEG4); // 必填
   OH_AVFormat_SetIntValue(formatVideo, OH_MD_KEY_WIDTH, 1280); // 必填
   OH_AVFormat_SetIntValue(formatVideo, OH_MD_KEY_HEIGHT, 720); // 必填
   OH_AVFormat_SetBuffer(formatVideo, OH_MD_KEY_CODEC_CONFIG, buffer, extraSize);
   
   int ret = OH_AVMuxer_AddTrack(muxer, &videoTrackId, formatVideo);
   if (ret != AV_ERR_OK || videoTrackId < 0) {
       // 视频轨添加失败
   }
   ```

   

5. 封装开始

   ``` c++
   // 调用start，写封装文件头。start后，不能设置媒体参数、不能添加媒体轨
   if (OH_AVMuxer_Start(muxer) != AV_ERR_OK) {
       // 异常处理
   }
   ```

   

6. 写入封装数据

   ``` c++
   // start后，才能开始写入数据
   char *sampleBuffer = ...; // 写入的媒体数据
   OH_AVCodecBufferAttr info;
   info.pts = ...; // 当前数据的开始播放的时间，单位微秒
   info.size = ...; // 当前数据的长度
   info.offset = 0; // 偏移，一般为0
   info.flags |= AVCODEC_BUFFER_FLAGS_SYNC_FRAME; // 当前数据的标志。具体参考OH_AVCodecBufferFlags
   int trackId = audioTrackId; // 选择写的媒体轨
   
   int ret = OH_AVMuxer_WriteSampleBuffer(muxer, trackId, sampleBuffer, info);
   if (ret != AV_ERR_OK) {
       // 异常处理
   }
   ```

   

7. 封装停止

   ``` c++
   // 调用stop，写封装文件尾。stop后不能写入媒体数据
   if (OH_AVMuxer_Stop(muxer) != AV_ERR_OK) {
       // 异常处理
   }
   ```

   

8. 销毁封装实例

   ``` cpp
   if (OH_AVMuxer_Destroy(muxer) != AV_ERR_OK) {
       // 异常处理
   }
   muxer = NULL;
   close(fd); // 关闭文件描述符
   ```

   
