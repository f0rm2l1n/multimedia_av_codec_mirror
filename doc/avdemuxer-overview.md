# 音视频解封装能力开发概述

解封装，简单来说，就是从比特流数据中取出音频、视频等媒体帧数据的过程。

目前系统API支持的数据输入类型如下：

- 远程连接(http协议)
- 文件描述符

目前系统API支持的解封装格式如下：

| 媒体格式  | 封装格式                      |
| -------- | :----------------------------|
| 视频     | mp4、mpeg-ts                  |
| 音频      | m4a、aac、mp3、ogg、flac、wav |



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
| OH_AVFormat *OH_AVSource_GetTrackFormat(OH_AVSource *source, uint32_t trackIndex);    | 获取 track 信息       |

**表2 avdemuxer开放能力接口**
| 接口名                                                       | 描述                 |
| --------------------------------------------------------------- | -------------------- |
|OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);    | 根据 source 创建 OH_AVDemuxer       |
|OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer);    | 销毁 OH_AVDemuxer       |
|OH_AVErrCode OH_AVDemuxer_SelectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);    | 选择需要解封装的轨道      |
|OH_AVErrCode OH_AVDemuxer_UnselectTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);    | 取消选择需要解封装的轨道       |
|OH_AVErrCode OH_AVDemuxer_ReadSample(OH_AVDemuxer *demuxer, uint32_t trackIndex, OH_AVMemory *sample, OH_AVCodecBufferAttr *info);    | 读取 trackIndex 对应轨道的帧     |
|OH_AVErrCode OH_AVDemuxer_SeekToTime(OH_AVDemuxer *demuxer, int64_t millisecond, OH_AVSeekMode mode);    | 跳转到指定时间       |


## 开发步骤

详细的API说明请参考 avdemuxer native API、avsource native API

1. 创建解封装器实例对象

   ``` c
   // 以只读方式创建文件操作符 fd，打开时对文件句柄必须有读权限
   std::string fileName = "/data/test/media/test.mp4";
   int32_t fd = open(fileName.c_str(), O_RDONLY);
   struct stat fileStatus {};
   size_t fileSize = 0;
   if (stat(fileName.c_str(), &fileStatus) == 0) {
      fileSize = static_cast<size_t>(fileStatus.st_size);
   } else {
      printf("get stat failed");
      return;
   }
   stat(fileName.c_str(), &fileStatus);
   // 为 fd 资源文件创建 source 资源对象, 传入 offset 不为文件起始位置 或 size 不为文件大小时，可能会因不能获取完整数据导致 source 创建失败、或后续解封装失败等问题
   OH_AVSource *source = OH_AVSource_CreateWithFD(fd, 0, fileSize);
   if(source==nullptr){
      printf("create source error: fd=%d, size=%zu", fd, fileSize);
      return;
   }
   // 为 uri 资源文件创建 source 资源对象
   // OH_AVSource *source = OH_AVSource_CreateWithURI(uri);
   // 为资源对象创建对应的解封装器
   OH_AVDemuxer *demuxer = OH_AVDemuxer_CreateWithSource(source);
   if(demuxer==nullptr){
      printf("create demuxer error");
      return;
   }
   ```



2. 获取文件轨道数（可选，若用户已知轨道信息，可跳过此步）

   ``` c
   // 从文件 source 信息获取文件轨道数
   OH_AVFormat *sourceFormat = OH_AVSource_GetSourceFormat(source);
   int32_t trackCount = 0;
   OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &trackCount);
   OH_AVFormat_Destroy(sourceFormat);
   ```

   

3. 获取轨道index及信息（可选，若用户已知轨道信息，可跳过此步）

   ``` c
   uint32_t audioTrackIndex = 0;
   uint32_t videoTrackIndex = 0;
   int32_t w = 0;
   int32_t h = 0;
   int32_t trackType;
   for (uint32_t index = 0; index < (static_cast<uint32_t>(trackCount)); index++) {
      // 获取轨道信息
      OH_AVFormat *trackFormat = OH_AVSource_GetTrackFormat(source, index);
      OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &trackType);
      static_cast<OH_MediaType>(trackType) == OH_MediaType::MEDIA_TYPE_AUD ? audioTrackIndex=index : videoTrackIndex=index;
      // 获取视频轨宽高
      if (trackType == OH_MediaType::MEDIA_TYPE_VID) {
         OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_WIDTH, &w);
         OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_HEIGHT, &h);
      }
      OH_AVFormat_Destroy(trackFormat);
   }
   ```

   

4. 添加解封装轨道

   ``` c
   if(OH_AVDemuxer_SelectTrackByID(demuxer, audioTrackIndex) != AV_ERR_OK){
      printf("select audio track error:%d", audioTrackIndex);
      return;
   }
   if(OH_AVDemuxer_SelectTrackByID(demuxer, videoTrackIndex) != AV_ERR_OK){
      printf("select video track error:%d", videoTrackIndex);
      return;
   }
   // 取消选择轨道(可选)
   // OH_AVDemuxer_UnselectTrackByID(demuxer, audioTrackIndex);
   ```



5. 调整轨道到指定时间点(可选)

   ``` c
   // 调整轨道到指定时间点，后续从该时间点进行解封装
   OH_AVDemuxer_SeekToTime(demuxer, 0, OH_AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
   ```

6. 开始解封装，循环获取帧数据(以含音频、视频两轨的文件为例)

   ``` c
   // 创建 buffer，用与保存用户解封装得到的数据
   OH_AVMemory *buffer = OH_AVMemory_Create(w * h * 3 >> 1);
   if (buffer == nullptr) {
      printf("buffer set error");
      return;
   }
   OH_AVCodecBufferAttr info;
   bool videoIsEnd = false;
   bool audioIsEnd = false;
   int32_t ret;
   while (!audioIsEnd||!videoIsEnd) {
      // 在调用 OH_AVDemuxer_ReadSample 接口获取数据前，需要先调用 OH_AVDemuxer_SelectTrackByID 选中需要获取数据的轨道
      // 获取音频帧数据
      if(!audioIsEnd) {
         ret = OH_AVDemuxer_ReadSample(demuxer, audioTrackIndex, buffer, &info);
         if (ret == AV_ERR_OK) {
            // 可通过 buffer 获取并处理音频帧数据
            printf("audio info.size: %d\n", info.size);
            if (info.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
               audioIsEnd = true;
            }
         }
      }
      if(!videoIsEnd) {
         ret = OH_AVDemuxer_ReadSample(demuxer, videoTrackIndex, buffer, &info);
         if (ret == AV_ERR_OK) {
            // 可通过 buffer 获取并处理视频帧数据
            printf("video info.size: %d\n", info.size);
            if (info.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
               videoIsEnd = true;
            }
         }
      }
   }
   OH_AVMemory_Destroy(buffer);
   ```

   

7. 销毁解封装实例

   ``` c
   // 需要用户调用 OH_AVSource_Destroy 接口成功后，手动将对象置为 NULL，对同一对象重复调用 OH_AVSource_Destroy 会导致程序错误
   if (OH_AVSource_Destroy(source) != AV_ERR_OK) {
      printf("destroy source pointer error");
   }
   source = NULL;
   // 需要用户调用 OH_AVDemuxer_Destroy 接口成功后，手动将对象置为 NULL，对同一对象重复调用 OH_AVDemuxer_Destroy 会导致程序错误
   if (OH_AVDemuxer_Destroy(demuxer) != AV_ERR_OK) {
      printf("destroy demuxer pointer error");
   }
   demuxer = NULL;
   ```