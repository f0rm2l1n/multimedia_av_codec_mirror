/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NATIVE_AVCAPABILITY_H
#define NATIVE_AVCAPABILITY_H

#include <stdint.h>
#include "native_averrors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AVCapability OH_AVCapability;
/**
 * @brief The bitrate mode of video encoder.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @since 10
 */
typedef enum OH_BitrateMode {
    /* constant bit rate mode. */
    CBR = 0,
    /* variable bit rate mode. */
    VBR = 1,
    /* constant quality mode. */
    CQ = 2,
    /* Constrained VariableBit Rate. */
    VCBR = 3,
    /* Average Bit Rate. */
    ABR = 4
} OH_BitrateMode;

/**
 * @brief Range contain min and max value
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @since 10
 */
typedef struct OH_AVRange {
    int32_t minVal;
    int32_t maxVal;
} OH_AVRange;

/**
 * @brief The codec category.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @since 10
 */
typedef enum OH_AVCodecCategory : int32_t {
    HARDWARE = 0,
    SOFTWARE = 1,
} OH_AVCodecCategory;

/**
 * @brief Get a a system-recommended codec's capability
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param mime Mime type
 * @param isEncoder True for encoder, false for decoder
 * @return A OH_AVCapability instance if the execution is successful, otherwise returns nullptr
 * @since 10
 */
OH_AVCapability *OH_AVCodec_GetCapability(const char *mime, bool isEncoder);

/**
 * @brief Get a a system-recommended codec's capability
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param mime Mime type
 * @param isEncoder True for encoder, false for decoder
 * @param category The codec category
 * @return A OH_AVCapability instance if the execution is successful, otherwise returns nullptr
 * @since 10
 */
OH_AVCapability *OH_AVCodec_GetCapabilityByCategory(const char *mime, bool isEncoder, OH_AVCodecCategory category);

/**
 * @brief Check whether the codec is hardware.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return true indicate vendor codec, false indicate software codec
 * @since 10
 */
bool OH_AVCapability_IsHardware(OH_AVCapability *capability);

/**
 * @brief Get codec name
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return codec name string
 * @since 10
 */
const char *OH_AVCapability_GetName(OH_AVCapability *capability);

/**
 * @brief Get Supported max instance
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return instance count
 * @since 10
 */
int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability);

/**
 * @brief Get encoder bitrate range
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param bitrateRange encoder bitrate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability, OH_AVRange *bitrateRange);

/**
 * @brief Check whether is this bitrate mode supported, only used for video encoder codecs.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param mode bitrateMode
 * @return true indicate supported, false indicate not supported
 * @since 10
 */
bool OH_AVCapability_IsEncoderBitrateModeSupported(OH_AVCapability *capability, OH_BitrateMode bitrateMode);

/**
 * @brief Get encoder quality range
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param bitrateRange encoder quality range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability, OH_AVRange *qualityRange);

/**
 * @brief Get encoder complexity range
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param complexityRange encoder complexity range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability, OH_AVRange *complexityRange);

/**
 * @brief Get sampleRate array, only used for audio codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param sampleRates sampleRates array pointer
 * @param sampleRateNum sampleRate num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability, const int32_t **sampleRates,
                                                          uint32_t *sampleRateNum);

/**
 * @brief Get channels range, only used for audio codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param channelsRange channels range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetAudioChannelCountRange(OH_AVCapability *capability, OH_AVRange *channelCountRange);

/**
 * @brief Get width alignment, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthAlignment width alignment
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability, int32_t *widthAlignment);

/**
 * @brief Get height alignment, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param heightAlignment height alignment
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability, int32_t *heightAlignment);

/**
 * @brief Get width range for a specified height, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange width range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability, int32_t height,
                                                         OH_AVRange *widthRange);

/**
 * @brief Get height range for a specified width, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange height range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability, int32_t width,
                                                         OH_AVRange *heightRange);

/**
 * @brief Get width range, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange width range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability, OH_AVRange *widthRange);

/**
 * @brief Get height range, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param heightRange height range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability, OH_AVRange *heightRange);

/**
 * @brief Check whether is this video size supported, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @return true indicate supported, false indicate not supported
 * @since 10
 */
bool OH_AVCapability_IsVideoSizeSupported(OH_AVCapability *capability, int32_t width, int32_t height);

/**
 * @brief Get frame rate range, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param frameRateRange frame rate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability, OH_AVRange *frameRateRange);

/**
 * @brief Get frame rate range for a specified video size, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @param frameRateRange frame rate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, int32_t width, int32_t height,
                                                           OH_AVRange *frameRateRange);

/**
 * @brief Check whether are this video size and fps supported.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height vidoe height
 * @param frameRate frames every second
 * @return true indicate supported, false indicate not supported
 * @since 10
 */
bool OH_AVCapability_AreVideoSizeAndFrameRateSupported(OH_AVCapability *capability, int32_t width, int32_t height,
                                                       int32_t frameRate);

/**
 * @brief Get supported pixFormat array, only used for video codecs
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param pixelFormats pixelFormats array pointer
 * @param pixelFormatNum pixelFormat num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetVideoSupportedPixelFormats(OH_AVCapability *capability, const int32_t **pixFormats,
                                                           uint32_t *pixFormatNum);

/**
 * @brief Get Supported profiles of this codec
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profiles profiles array pointer
 * @param profileNum profiles num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability, const int32_t **profiles,
                                                  uint32_t *profileNum);

/**
 * @brief Get levels array for a specified profile.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profile codec profiles
 * @param levels levels array pointer
 * @param levelNum levels num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 */
OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability, int32_t profile,
                                                          const int32_t **levels, uint32_t *levelNum);

/**
 * @brief Check whether the profile and level is supported.
 * @syscap SystemCapability.Multimedia.Media.CodecBase
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profile AVCProfile if codec is h264, HEVCProfile array if codec is h265
 * @param level level id
 * @return true indicate supported, false indicate not supported
 * @since 10
 */
bool OH_AVCapability_AreProfileAndLevelSupported(OH_AVCapability *capability, int32_t profile, int32_t level);
#ifdef __cplusplus
}
#endif
#endif // NATIVE_AVCAPABILITY_H