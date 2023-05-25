# 音视频解封装能力开发概述

解封装，简单来说，就是从比特流数据中取出音频、视频等媒体帧数据的过程。

目前系统API支持的数据输入类型如下：

- 远程连接(Uniform Resource Identifier, URI)
- 文件描述符(File Descriptor, FD)

目前系统API支持的解封装格式如下：

- 视频：
  - mp4
  - mpeg-ts
- 音频：
  - m4a
  - aac
  - mp3
  - ogg
  - flac
  - wav

**音视频解封装的使用场景**：

- 播放
  
  播放媒体文件时，需要先对音视频流进行解封装，然后使用解封装获取的帧数据进行解码和播放。

- 音视频编辑
  
  编译媒体文件时，需要先对音视频流进行解封装，获取到指定帧进行编辑。

- 媒体文件格式转换（转封装）

  媒体文件格式转换时，需要先对音视频流进行解封装，然后按需将音视频流封装至新的格式文件内。

# 使用avdemuxer开发音视频解封装功能

## 接口说明

**表1 avsource开放能力接口**

| 接口名                                                       | 描述                 |
| ------------------------------------------------------------ | -------------------- |
| OH_AVSource *OH_AVSource_CreateWithURI(char *uri);  | 根据 URI 创建 OH_AVSource       |
| OH_AVSource *OH_AVSource_CreateWithFD(int32_t fd, int64_t offset, int64_t size);   | 根据 FD 创建OH_AVSource       |
| OH_AVErrCode OH_AVSource_Destroy(OH_AVSource *source);    | 销毁 OH_AVSource       |
| OH_AVFormat *OH_AVSource_GetSourceFormat(OH_AVSource *source);   | 获取 source 信息       |
| OH_AVFormat OH_AVSource_GetTrackFormat(OH_AVSource *source, uint32_t trackIndex);    | 获取 track 信息       |

**表2 avdemuxer开放能力接口**
| 接口名                                                       | 描述                 |
| --------------------------------------------------------------- | -------------------- |
|OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);    | 根据 source 创建 OH_AVDemuxer       |
|OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer);    | 销毁 OH_AVDemuxer       |
|OH_AVErrCode OH_AVDemuxer_SelectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);    | 选择需要解封装的轨道      |
|OH_AVErrCode OH_AVDemuxer_UnselectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);    | 取消选择需要解封装的轨道       |
|OH_AVErrCode OH_AVDemuxer_ReadSample(OH_AVDemuxer *demuxer, uint32_t trackIndex, OH_AVMemory *sample, OH_AVCodecBufferAttr *info);    | 读取 trackIndex 对应轨道的帧     |
|OH_AVErrCode OH_AVDemuxer_SeekToTime(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode);    | 跳转到指定时间       |


## 开发步骤

详细的API说明请参考avdemuxer native API、avsource native API

1. 创建解封装器实例对象

   ``` c
   // 以只读方式创建文件 fd
   int32_t fd = open("test.mp4", O_RDONLY);
   // 为 fd 资源文件创建 source 资源对象
   OH_AVSource *source = OH_AVSource_CreateWithFD(fd, 0, 0);
   // 为 uri 资源文件创建 source 资源对象
   // OH_AVSource *source = OH_AVSource_CreateWithURI(uri);
   // 为资源对象创建对应的解封装器
   OH_AVDemuxer *demuxer = *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);
   ```



2. 获取文件轨道数（非必须）

   ``` c
   // 获取文件 source 信息
   OH_AVFormat *sourceFormat = OH_AVSource_GetSourceFormat(OH_AVSource *source);
   uint32_t trackCount = 0;
   OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &trackCount);
   OH_AVFormat_Destroy(sourceFormat);
   ```

   

3. 获取轨道index及信息（非必须）

   ``` c
   uint32_t audioTrackIndex = 0;
   uint32_t videoTrackIndex = 0;
   uint32_t w = 0;
   uint32_t h = 0;
   OH_MediaType trackType;
   OH_AVFormat *trackFormat = OH_AVFormat_Create();
   for (uint32_t index=0; index<trackCount; index++) {
      // 获取轨道信息
      trackFormat = OH_AVSource_GetTrackFormat(source, index);
      OH_AVFormat_GetIntValue(trackFormat, &trackType);
      trackType == MEDIA_TYPE_AUD ? audioTrackIndex = index : videoTrackIndex = index;
      // 获取视频轨宽高
      if (trackType == MEDIA_TYPE_VID) {
         OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_WIDTH, &w);
         OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_HEIGHT, &h);
      }
   }
   OH_AVFormat_Destroy(trackFormat);
   ```

   

4. 添加解封装轨道

   ``` c
   // 若用户已知轨道信息，直接调用 OH_AVDemuxer_SelectTrackByID 选取轨道，无需通过 source format 和 track count 判断
   if(OH_AVDemuxer_SelectTrackByID(audioTrackIndex) != AV_ERR_OK){
      // 异常处理
   }
   if(OH_AVDemuxer_SelectTrackByID(videoTrackIndex) != AV_ERR_OK){
      // 异常处理
   }
   // 取消选择轨道(可选)
   // OH_AVDemuxer_UnselectTrackByID(audioTrackIndex);
   ```



5. 调整轨道到指定时间点(可选)

   ``` c
   // 调整轨道到指定时间点，后续从该时间点进行解封装
   OH_AVDemuxer_SeekToTime(demuxer, 0, AVSEEKMODE_CLOESEST);
   ```

6. 开始解封装，循环获取帧数据(以含音频、视频两轨的文件为例)

   ``` c
   // 创建 buffer，用户获取解封装数据
   OH_AVMemory *buffer = OH_AVMemory_Create(w * h * 3 >> 1);
   OH_AVCodecBufferAttr info; // 参考OH_AVCodecBufferFlags
   // 选择读取轨
   uint32_t trackIndex = audioTrackIndex; 
   bool isEnd = false;
   // 调整轨道到指定时间点，后续从该时间点进行解封装
   // OH_AVDemuxer_UnselectTrackByID(audioTrackIndex);
   while (!isEnd) {
      // 获取帧数据
      if(OH_AVDemuxer_ReadSample(demuxer, trackIndex, buffer, info)) {
         // 处理 buffer 数据
      }
      if (attr.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
         isEnd = true;
      }
      // 切换下次读取另一轨道数据
      trackIndex == videoTrackIndex ? trackIndex = audioTrackIndex : trackIndex = videoTrackIndex;
   }   
   ```

   

7. 销毁解封装实例

   ``` c
   if (OH_AVSource_Destroy(source) != AV_ERR_OK) {
       // 异常处理
   }
   source = NULL;
   if (OH_AVDemuxer_Destroy(demuxer) != AV_ERR_OK) {
       // 异常处理
   }
   demuxer = NULL;
   close(fd); // 关闭文件描述符
   ```

   
