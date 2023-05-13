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
extern "C"{
#endif

typedef struct OH_AVCapability OH_AVCapability;

/**
 * @brief The bitrate mode of video encoder.
 * @since 10
 * @version 1.0
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
 * @since 10
 * @version 1.0
 */
typedef struct OH_AVRange {
    int32_t minVal;
    int32_t maxVal;
} OH_AVRange;

/**
 * @brief Check whether the codec is accelerated by hardware.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return true indicate vendor codec, false indicate software codec
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_IsHardwareAccelerated(OH_AVCapability *capability);

/**
 * @brief Get mime type
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return mime type string
 * @since 10
 * @version 1.0
*/
const char *OH_AVCapability_GetMimeType(OH_AVCapability *capability);

/**
 * @brief Get Supported max instance
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return instance count
 * @since 10
 * @version 1.0
*/
int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability);

/**
 * @brief Get Supported profiles of this codec
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profiles profiles array pointer
 * @param profileNum profiles num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability,
    const int32_t **profiles, uint32_t *profileNum);

/**
 * @brief Get levels array for a specified profile.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profile codec profiles
 * @param levels levels array pointer
 * @param levelNum levels num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability,
    int32_t profile, const int32_t **levels, uint32_t *levelNum);

/**
 * @brief Check whether the profile and level is supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param profile AVCProfile if codec is h264, HEVCProfile array if codec is h265
 * @param level level id
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_ValidateProfileAndLevel(OH_AVCapability *capability,
    int32_t profile, int32_t level);

/**
 * @brief Get encoder bitrate range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param bitrateRange encoder bitrate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability,
    OH_AVRange *bitrateRange);

/**
 * @brief Get encoder quality range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param bitrateRange encoder quality range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability,
    OH_AVRange *qualityRange);

/**
 * @brief Get encoder complexity range
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param complexityRange encoder complexity range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability,
    OH_AVRange *complexityRange);

/**
 * @brief Check whether is this bitrate mode supported, only used for video encoder codecs.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param mode bitrateMode
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_ValidateVideoEncoderBitrateMode(OH_AVCapability *capability,
    OH_BitrateMode bitrateMode);

/**
 * @brief Get sampleRate array, only used for audio codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param sampleRates sampleRates array pointer
 * @param sampleRateNum sampleRate num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability,
    const int32_t **sampleRates, uint32_t *sampleRateNum);

/**
 * @brief Check whether is this sampleRate supported, only used for audio codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param sampleRate sample rate
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_ValidateAudioSampleRate(OH_AVCapability *capability,
    int32_t sampleRate);

/**
 * @brief Get channels range, only used for audio codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param channelsRange channels range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetAudioChannelsRange(OH_AVCapability *capability,
    OH_AVRange *channelsRange);


/**
 * @brief Get supported pixFormat array, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param pixFormats pixFormats array pointer
 * @param pixFormatNum pixFormat num
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoSupportedPixFormats(OH_AVCapability *capability,
    const int32_t **pixFormats, uint32_t *pixFormatNum);

/**
 * @brief Get width alignment, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthAlignment width alignment
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability,
    int32_t *widthAlignment);

/**
 * @brief Get height alignment, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param heightAlignment height alignment
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability,
    int32_t *heightAlignment);

/**
 * @brief Get width range for a specified height, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange width range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability,
    int32_t height, OH_AVRange *widthRange);

/**
 * @brief Get height range for a specified width, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange height range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability,
    int32_t width, OH_AVRange *heightRange);

/**
 * @brief Get width range, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param widthRange width range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability,
    OH_AVRange *widthRange);

/**
 * @brief Get height range, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param heightRange height range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability,
    OH_AVRange *heightRange);

/**
 * @brief Check whether is this video size supported, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_ValidateVideoSize(OH_AVCapability *capability,
    int32_t width, int32_t height);

/**
 * @brief Get frame rate range, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param frameRateRange frame rate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability,
    OH_AVRange *frameRateRange);

/**
 * @brief Get frame rate range for a specified video size, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @param frameRateRange frame rate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, 
    int32_t width, int32_t height, OH_AVRange *frameRateRange);

/**
 * @brief Get measured frame rate range for a specified video size, 
 * these frame rates is reachable, only used for video codecs
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height video height
 * @param frameRateRange frame rate range
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
*/
OH_AVErrCode OH_AVCapability_GetVideoMeasuredFrameRateRangeForSize(OH_AVCapability *capability,
    int32_t width, int32_t height, OH_AVRange *frameRateRange);

/**
 * @brief Check whether are this video size and fps supported.
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @param width video width
 * @param height vidoe height
 * @param frameRate frames every second
 * @return true indicate supported, false indicate not supported
 * @since 10
 * @version 1.0
*/
bool OH_AVCapability_ValidateVideoSizeAndFrameRate(OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t frameRate);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCAPABILITY_H