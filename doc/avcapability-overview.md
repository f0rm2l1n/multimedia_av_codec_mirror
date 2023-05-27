# 编解码器能力获取开发概述

编解码器能力获取，通过查询对应接口，可获得对应音视频的软硬件编解码能力以及进行部分参数的校验。

**编解码器能力获取的使用场景**：

调用编解码器或者配置编解码器前，查询当前系统支持的编解码器规格。

# 使用编解码器能力获取功能

## 接口说明

**表1** codeclist开放能力接口
| 接口名                                                     | 描述         |
| -------------------------------------------------------- | -------------------- |
|OH_AVCapability *OH_AVCodec_GetCapability(const char *mime, bool isEncoder);|获取系统推荐的能力句柄|
|OH_AVCapability *OH_AVCodec_GetCapabilityByCategory(const char *mime, bool isEncoder, OH_AVCodecCategory category);|获取系统指定软硬件的能力句柄|
|bool OH_AVCapability_IsHardware(OH_AVCapability *capability);|确认是否是硬件编解码器|
|const char *OH_AVCapability_GetName(OH_AVCapability *capability);|获取codec名字|
|int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability);|获取最大支持的实例数|
|OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability, OH_AVRange *bitrateRange);|获取编码支持的码率范围|
|bool OH_AVCapability_IsEncoderBitrateModeSupported(OH_AVCapability *capability, OH_BitrateMode bitrateMode);|确认码控模式是否支持|
|OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability, OH_AVRange *qualityRange);|获取编码质量范围|
|OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability, OH_AVRange *complexityRange);|获取编码复杂度范围|
|OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability, const int32_t **sampleRates, uint32_t *sampleRateNum);|获取支持的音频采样率|
|OH_AVErrCode OH_AVCapability_GetAudioChannelCountRange(OH_AVCapability *capability, OH_AVRange *channelCountRange);|获取音频通道数范围|
|OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability, int32_t *widthAlignment);|获取视频宽对齐|
|OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability, int32_t *heightAlignment);|获取视频高对齐|
|OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability, int32_t height, OH_AVRange *widthRange);|获取特定高情况下视频宽范围|
|OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability, int32_t width, OH_AVRange *heightRange);|获取特定宽情况下视频高范围|
|OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability, OH_AVRange *widthRange);|获取视频宽范围|
|OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability, OH_AVRange *heightRange);|获取视频高范围|
|bool OH_AVCapability_IsVideoSizeSupported(OH_AVCapability *capability, int32_t width, int32_t height);|确认当前视频尺寸是否支持|
|OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability, OH_AVRange *frameRateRange);|获取视频帧率范围|
|OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, int32_t width, int32_t height, OH_AVRange *frameRateRange);|获取特定尺寸下视频帧率范围|
|bool OH_AVCapability_AreVideoSizeAndFrameRateSupported(OH_AVCapability *capability, int32_t width, int32_t height, int32_t frameRate);|确认当前视频尺寸和帧率是否支持|
|OH_AVErrCode OH_AVCapability_GetVideoSupportedPixelFormats(OH_AVCapability *capability, const int32_t **pixFormats, uint32_t *pixFormatNum);|获取支持的视频像素格式|
|OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability, const int32_t **profiles, uint32_t *profileNum);|获取支持的模板|
|OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability, int32_t profile, const int32_t **levels,uint32_t *levelNum);|获取特定模板情况下的等级范围|
|bool OH_AVCapability_AreProfileAndLevelSupported(OH_AVCapability *capability, int32_t profile, int32_t level);|确认当前模板和等级是否支持|

## 开发步骤
1. 获得能力

    ```c
    // 根据mime type、是否编码器获得能力
    OH_AVCapability *capability = OH_AVCodec_GetCapability("video/avc", false);

    // 根据mime type、是否编码器以及软硬件类别获得能力
    OH_AVCapability *capability = OH_AVCodec_GetCapabilityByCategory("video/avc", false, SOFTWARE);
    ```

2. 查询参数
    ```c
    // 查询当前能力是否支持硬件
    bool isHardware = OH_AVCapability_IsHardware(capability);

    // 查询当前能力codec名称
    const char *codecName = OH_AVCapability_GetName(capability);

    // 查询当前能力中，宽的范围
    Range widthRange;
    int32_t ret = OH_AVCapability_GetVideoWidthRange(capability, &widthRange);
    if(ret != AV_ERR_OK) {
        // 处理异常
    }

    // 查询当前能力中，高为1080时，宽的范围
    ret = OH_AVCapability_GetVideoWidthRangeForHeight(capability, 1080, &widthRange);
    if(ret != AV_ERR_OK) {
        // 处理异常
    }

    // 查询当前能力中，宽的对齐值
    int32_t widthAlignment;
    ret = OH_AVCapability_GetVideoWidthAlignment(capability, &widthAlignment);
    if(ret != AV_ERR_OK) {
        // 处理异常
    }

    // 查询当前能力中，支持的颜色格式以及个数
    int32_t *pixFormats;
    uint32_t pixFormatNum = 0;
    ret = OH_AVCapability_GetVideoSupportedPixelFormats(capability, &pixFormats, &pixFormatNum);
    if(ret != AV_ERR_OK) {
        // 处理异常
    }

    // 校验当前能力是否支持分辨率1080p
    bool isVideoSizeSupported = OH_AVCapability_IsVideoSizeSupported(capability, 1920, 1080);

    // 校验当前能力是否支持分辨率1080p、帧率30的场景
    bool areVideoSizeAndFrameRateSupported = OH_AVCapability_AreVideoSizeAndFrameRateSupported(capability, 1920, 1080, 30);
    // 其他能力查询参见接口定义...
    ```