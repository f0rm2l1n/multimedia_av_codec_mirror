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

#include "native_avcapability.h"
#include <algorithm>
#include "native_avmagic.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

// namespace {
// constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "OH_AVCapability"};
// }

using namespace OHOS::Media;

OH_AVCapability::OH_AVCapability(const CapabilityData &capabilityData)
    : capabilityData_(capabilityData)
{
}

OH_AVCapability::~OH_AVCapability()
{

}

bool OH_AVCapability_IsHardware(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return false;
    }
    return capability->capabilityData_.isVendor;
}

const char *OH_AVCapability_GetMimeType(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        std::string empty;
        return empty.data();
    }

    return capability->capabilityData_.mimeType.data(); //TODO: string data是否安全 
}

int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return 0;
    }

    return capability->capabilityData_.maxInstance;
}

OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability,
    const int32_t **profiles, uint32_t *profileNum)
{
    if (capability == nullptr) {
        std::vector<int32_t> empty;
        *profiles = empty.data();
        *profileNum = empty.size();
        return AV_ERR_INVALID_VAL;
    }
    
    auto &vec = capability->capabilityData_.profiles;
    *profiles = vec.data();
    *profileNum = vec.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability,
    int32_t profile, const int32_t **levels, uint32_t *levelNum) 
{
    std::vector<int32_t> empty;
    *levels = empty.data();
    *levelNum = empty.size();
    if (capability == nullptr) {
        return AV_ERR_INVALID_VAL;
    }

    auto &profileLevelsMap = capability->capabilityData_.profileLevelsMap;
    auto levelsmatch = profileLevelsMap.find(profile);
    if (levelsmatch == profileLevelsMap.end()) {
        return AV_ERR_INVALID_VAL;
    }
    *levels = levelsmatch->second.data();
    *levelNum = levelsmatch->second.size();
    return AV_ERR_OK;
}

bool OH_AVCapability_ValidateProfileAndLevel(OH_AVCapability *capability,
    int32_t profile, int32_t level)
{
    if (capability == nullptr) {
        return false;
    }
    auto &profileLevelsMap = capability->capabilityData_.profileLevelsMap;
    auto levels = profileLevelsMap.find(profile);
    if (levels == profileLevelsMap.end()) {
        return false;
    }
 
    return find(levels->second.begin(), levels->second.end(), level) != levels->second.end();
}

OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability,
    OH_AVRange *bitrateRange)
{
    if (capability == nullptr) {
        bitrateRange->minVal = 0;
        bitrateRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    bitrateRange->minVal = capability->capabilityData_.bitrate.minVal;
    bitrateRange->maxVal = capability->capabilityData_.bitrate.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability,
    OH_AVRange *qualityRange)
{
    if (capability == nullptr) {
        qualityRange->minVal = 0;
        qualityRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    qualityRange->minVal = capability->capabilityData_.encodeQuality.minVal;
    qualityRange->maxVal = capability->capabilityData_.encodeQuality.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability,
    OH_AVRange *complexityRange)
{
    if (capability == nullptr) {
        complexityRange->minVal = 0;
        complexityRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    complexityRange->minVal = capability->capabilityData_.complexity.minVal;
    complexityRange->maxVal = capability->capabilityData_.complexity.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_ValidateVideoEncoderBitrateMode(OH_AVCapability *capability,
    OH_BitrateMode bitrateMode)
{
    if (capability == nullptr) {
        return false;
    }
    auto &bitrateModeVec = capability->capabilityData_.bitrateMode;
    return find(bitrateModeVec.begin(), bitrateModeVec.end(), bitrateMode) != bitrateModeVec.end();
}

OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability,
    const int32_t **sampleRates, uint32_t *sampleRateNum)
{
    if (capability == nullptr) {
        std::vector<int32_t> empty;
        *sampleRates = empty.data();
        *sampleRateNum = empty.size();
        return AV_ERR_INVALID_VAL;
    }
    
    auto &vec = capability->capabilityData_.sampleRate;
    *sampleRates = vec.data();
    *sampleRateNum = vec.size();
    return AV_ERR_OK;
}

bool OH_AVCapability_ValidateAudioSampleRate(OH_AVCapability *capability,
    int32_t sampleRate)
{
    if (capability == nullptr) {
        return false;
    }
    auto &sampeleRateVec = capability->capabilityData_.sampleRate;
    return find(sampeleRateVec.begin(), sampeleRateVec.end(), sampleRate) != sampeleRateVec.end();
}

OH_AVErrCode OH_AVCapability_GetAudioChannelsRange(OH_AVCapability *capability,
    OH_AVRange *channelsRange)
{
    if (capability == nullptr) {
        channelsRange->minVal = 0;
        channelsRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    channelsRange->minVal = capability->capabilityData_.channels.minVal;
    channelsRange->maxVal = capability->capabilityData_.channels.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoSupportedPixFormats(OH_AVCapability *capability,
    const int32_t **pixFormats, uint32_t *pixFormatNum)
{
    if (capability == nullptr) {
        std::vector<int32_t> empty;
        *pixFormats = empty.data();
        *pixFormatNum = empty.size();
        return AV_ERR_INVALID_VAL;
    }
    auto &vec = capability->capabilityData_.pixFormat;
    *pixFormats = vec.data();
    *pixFormatNum = vec.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability,
    int32_t *widthAlignment)
{
    if (capability == nullptr) {
        *widthAlignment = 0;
        return AV_ERR_INVALID_VAL;
    }
    *widthAlignment = capability->capabilityData_.alignment.width;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability,
    int32_t *heightAlignment)
{
    if (capability == nullptr) {
        *heightAlignment = 0;
        return AV_ERR_INVALID_VAL;
    }
    *heightAlignment = capability->capabilityData_.alignment.height;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability,
    int32_t height, OH_AVRange *widthRange)
{
    if (capability == nullptr) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    // TODO
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability,
    int32_t width, OH_AVRange *heightRange)
{
    if (capability == nullptr) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    // TODO
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability,
    OH_AVRange *widthRange)
{
    if (capability == nullptr) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    widthRange->minVal = capability->capabilityData_.width.minVal;
    widthRange->maxVal = capability->capabilityData_.width.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability,
    OH_AVRange *heightRange)
{
    if (capability == nullptr) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    heightRange->minVal = capability->capabilityData_.height.minVal;
    heightRange->maxVal = capability->capabilityData_.height.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_ValidateVideoSize(OH_AVCapability *capability,
    int32_t width, int32_t height)
{
    if (capability == nullptr) {
        return false;
    }

    auto &alignment = capability->capabilityData_.alignment;
    if (width == 0 || height == 0 ||
        alignment.width == 0 || alignment.height == 0 ||
        width % alignment.width != 0 || height % alignment.height != 0) {
        return false;
    }

    auto &widthRange = capability->capabilityData_.width;
    auto &heightRange = capability->capabilityData_.height;
    if (width < widthRange.minVal || width > widthRange.maxVal ||
        height < heightRange.minVal || height > heightRange.maxVal) {
        return false;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return false;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) * 
        ((height + blockSize.height - 1) / blockSize.height);
    
    auto &blockPerFrame = capability->capabilityData_.blockPerFrame;
    if (blockNum < blockPerFrame.minVal || blockNum > blockPerFrame.maxVal) {
        return false;
    }

    return true;
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability,
    OH_AVRange *frameRateRange)
{
    if (capability == nullptr) {
        frameRateRange->minVal = 0;
        frameRateRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    frameRateRange->minVal = capability->capabilityData_.frameRate.minVal;
    frameRateRange->maxVal = capability->capabilityData_.frameRate.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, 
    int32_t width, int32_t height, OH_AVRange *frameRateRange)
{
    frameRateRange->minVal = 0;
    frameRateRange->maxVal = 0;
    if (capability == nullptr) {
        return AV_ERR_INVALID_VAL;
    }

    if (OH_AVCapability_ValidateVideoSize(capability, width, height)!= AV_ERR_OK) {
        return AV_ERR_INVALID_VAL;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    auto &blockPerSecond = capability->capabilityData_.blockPerSecond;
    if (blockSize.width == 0 || blockSize.height == 0 ||
        blockPerSecond.minVal <= 0 || blockPerSecond.maxVal <= 0) {
        return AV_ERR_UNKNOWN;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);

    auto &fpsRange = capability->capabilityData_.frameRate;
    frameRateRange->minVal = std::max((blockPerSecond.minVal + blockNum - 1) / blockNum, fpsRange.minVal);
    frameRateRange->maxVal = std::min(blockPerSecond.maxVal / blockNum, fpsRange.maxVal);
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoMeasuredFrameRateRangeForSize(OH_AVCapability *capability,
    int32_t width, int32_t height, OH_AVRange *frameRateRange)
{
    frameRateRange->minVal = 0;
    frameRateRange->maxVal = 0;
    if (capability == nullptr) {
        return AV_ERR_INVALID_VAL;
    }

    if (OH_AVCapability_ValidateVideoSize(capability, width, height)!= AV_ERR_OK) {
        return AV_ERR_INVALID_VAL;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return AV_ERR_UNKNOWN;
    }
    int32_t blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);

    int32_t minDiff = INT32_MAX;
    Range fpsRange;
    float factor = 1.0f;
    float blockNumFloat{blockNum};
    auto &measureFps = capability->capabilityData_.measuredFrameRate;
    auto iter = measureFps.begin();
    while (iter != measureFps.end()) {
        int32_t curBlockNum = (iter->first.width + blockSize.width - 1) / blockSize.width *
            ((iter->first.height + blockSize.height - 1) / blockSize.height);
        int32_t curDiff = std::abs(curBlockNum - blockNum);
        if (curDiff < minDiff) {
            minDiff = curDiff;
            fpsRange = iter->second;
            factor = blockNumFloat / curBlockNum;
        }
        iter++;
    }

    frameRateRange->minVal = static_cast<int32_t>(fpsRange.minVal * factor);
    frameRateRange->maxVal = static_cast<int32_t>(fpsRange.maxVal * factor);
    return AV_ERR_OK;
}

bool OH_AVCapability_ValidateVideoSizeAndFrameRate(OH_AVCapability *capability,
    int32_t width, int32_t height, int32_t frameRate)
{
    if (capability == nullptr) {
        return false;
    }

    auto &alignment = capability->capabilityData_.alignment;
    if (width == 0 || height == 0 ||
        alignment.width == 0 || alignment.height == 0 ||
        width % alignment.width != 0 || height % alignment.height != 0) {
        return false;
    }

    auto &widthRange = capability->capabilityData_.width;
    auto &heightRange = capability->capabilityData_.height;
    auto &fpsRange = capability->capabilityData_.frameRate;
    if (width < widthRange.minVal || width > widthRange.maxVal ||
        height < heightRange.minVal || height > heightRange.maxVal ||
        frameRate < fpsRange.minVal || frameRate > fpsRange.maxVal) {
        return false;
    }

    auto &blockSize = capability->capabilityData_.blockSize;
    if (blockSize.width == 0 || blockSize.height == 0) {
        return false;
    }
    int blockNum = ((width + blockSize.width - 1) / blockSize.width) *
        ((height + blockSize.height - 1) / blockSize.height);
    int blockOneSecond = blockNum * frameRate;
    auto &blockPerFrame = capability->capabilityData_.blockPerFrame;
    auto &blockPerSecond = capability->capabilityData_.blockPerSecond;
    if (blockNum < blockPerFrame.minVal || blockNum > blockPerFrame.maxVal ||
        blockOneSecond < blockPerSecond.minVal || blockOneSecond > blockPerSecond.maxVal ) {
        return false;
    }
    return true;
}

