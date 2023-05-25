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

#include <algorithm>
#include "native_avmagic.h"
#include "avcodec_list.h"
#include "avcodec_info.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "native_avcapability.h"

using namespace OHOS::Media;
OH_AVCapability::OH_AVCapability(const CapabilityData &capabilityData) : capabilityData_(capabilityData) {}

OH_AVCapability::~OH_AVCapability() {}

OH_AVCapability *OH_AVCodec_GetCapability(const char *mime, bool isEncoder)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    CapabilityData capabilityData = codeclist->GetCapability(mime, isEncoder, AVCodecCategory::AVCODEC_NONE);
    return new (std::nothrow) OH_AVCapability(capabilityData);
}

OH_AVCapability *OH_AVCodec_GetCapabilityByCategory(const char *mime, bool isEncoder, OH_AVCodecCategory category)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    AVCodecCategory innerCategory;
    if (category == HARDWARE) {
        innerCategory = AVCodecCategory::AVCODEC_HARDWARE;
    } else {
        innerCategory = AVCodecCategory::AVCODEC_SOFTWARE;
    }
    CapabilityData capabilityData = codeclist->GetCapability(mime, isEncoder, innerCategory);
    return new (std::nothrow) OH_AVCapability(capabilityData);
}

const char *OH_AVCapability_GetName(OH_AVCapability *capability)
{
    std::string name;
    if (capability == nullptr) {
        return name.data();
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    name = codecInfo->GetName();
    return name.data();
}

bool OH_AVCapability_IsHardware(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return false;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    return codecInfo->IsHardwareAccelerated();
}

int32_t OH_AVCapability_GetMaxSupportedInstances(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return 0;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    return codecInfo->GetMaxSupportedInstances();
}

OH_AVErrCode OH_AVCapability_GetSupportedProfiles(OH_AVCapability *capability, const int32_t **profiles,
                                                  uint32_t *profileNum)
{
    std::vector<int32_t> ret;
    if (capability == nullptr) {
        *profiles = ret.data();
        *profileNum = ret.size();
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    ret = codecInfo->GetSupportedProfiles();
    *profiles = ret.data();
    *profileNum = ret.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetSupportedLevelsForProfile(OH_AVCapability *capability, int32_t profile,
                                                          const int32_t **levels, uint32_t *levelNum)
{
    std::vector<int32_t> empty;
    *levels = empty.data();
    *levelNum = empty.size();
    if (capability == nullptr) {
        return AV_ERR_INVALID_VAL;
    }

    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    auto profileLevelsMap = codecInfo->GetSupportedLevelsForProfile();
    auto levelsmatch = profileLevelsMap.find(profile);
    if (levelsmatch == profileLevelsMap.end()) {
        return AV_ERR_INVALID_VAL;
    }
    *levels = levelsmatch->second.data();
    *levelNum = levelsmatch->second.size();
    return AV_ERR_OK;
}

bool OH_AVCapability_AreProfileAndLevelSupported(OH_AVCapability *capability, int32_t profile, int32_t level)
{
    if (capability == nullptr) {
        return false;
    }
    std::shared_ptr<AVCodecInfo> codecInfo = std::make_shared<AVCodecInfo>(capability->capabilityData_);
    auto profileLevelsMap = codecInfo->GetSupportedLevelsForProfile();
    auto levels = profileLevelsMap.find(profile);
    if (levels == profileLevelsMap.end()) {
        return false;
    }

    return find(levels->second.begin(), levels->second.end(), level) != levels->second.end();
}

OH_AVErrCode OH_AVCapability_GetEncoderBitrateRange(OH_AVCapability *capability, OH_AVRange *bitrateRange)
{
    if (capability == nullptr) {
        bitrateRange->minVal = 0;
        bitrateRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    auto bitrate = codecInfo->GetSupportedBitrate();
    bitrateRange->minVal = bitrate.minVal;
    bitrateRange->maxVal = bitrate.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderQualityRange(OH_AVCapability *capability, OH_AVRange *qualityRange)
{
    if (capability == nullptr) {
        qualityRange->minVal = 0;
        qualityRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto quality = codecInfo->GetSupportedEncodeQuality();
    qualityRange->minVal = quality.minVal;
    qualityRange->maxVal = quality.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetEncoderComplexityRange(OH_AVCapability *capability, OH_AVRange *complexityRange)
{
    if (capability == nullptr) {
        complexityRange->minVal = 0;
        complexityRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto complexity = codecInfo->GetSupportedComplexity();
    complexityRange->minVal = complexity.minVal;
    complexityRange->maxVal = complexity.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_IsEncoderBitrateModeSupported(OH_AVCapability *capability, OH_BitrateMode bitrateMode)
{
    if (capability == nullptr) {
        return false;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto bitrateModeVec = codecInfo->GetSupportedBitrateMode();
    return find(bitrateModeVec.begin(), bitrateModeVec.end(), bitrateMode) != bitrateModeVec.end();
}

OH_AVErrCode OH_AVCapability_GetAudioSupportedSampleRates(OH_AVCapability *capability, const int32_t **sampleRates,
                                                          uint32_t *sampleRateNum)
{
    if (capability == nullptr) {
        std::vector<int32_t> empty;
        *sampleRates = empty.data();
        *sampleRateNum = empty.size();
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    auto vec = codecInfo->GetSupportedSampleRates();
    *sampleRates = vec.data();
    *sampleRateNum = vec.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetAudioChannelsRange(OH_AVCapability *capability, OH_AVRange *channelsRange)
{
    if (capability == nullptr) {
        channelsRange->minVal = 0;
        channelsRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    auto channels = codecInfo->GetSupportedChannel();
    channelsRange->minVal = channels.minVal;
    channelsRange->maxVal = channels.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoSupportedPixelFormats(OH_AVCapability *capability, const int32_t **pixFormats,
                                                           uint32_t *pixFormatNum)
{
    if (capability == nullptr) {
        std::vector<int32_t> empty;
        *pixFormats = empty.data();
        *pixFormatNum = empty.size();
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<AudioCaps> codecInfo = std::make_shared<AudioCaps>(capability->capabilityData_);
    auto vec = codecInfo->GetSupportedFormats();
    *pixFormats = vec.data();
    *pixFormatNum = vec.size();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthAlignment(OH_AVCapability *capability, int32_t *widthAlignment)
{
    if (capability == nullptr) {
        *widthAlignment = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    *widthAlignment = codecInfo->GetSupportedWidthAlignment();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightAlignment(OH_AVCapability *capability, int32_t *heightAlignment)
{
    if (capability == nullptr) {
        *heightAlignment = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    *heightAlignment = codecInfo->GetSupportedHeightAlignment();
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRangeForHeight(OH_AVCapability *capability, int32_t height,
                                                         OH_AVRange *widthRange)
{
    if (capability == nullptr) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto width = codecInfo->GetVideoWidthRangeForHeight(height);
    widthRange->minVal = width.minVal;
    widthRange->maxVal = width.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRangeForWidth(OH_AVCapability *capability, int32_t width,
                                                         OH_AVRange *heightRange)
{
    if (capability == nullptr) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto height = codecInfo->GetVideoHeightRangeForWidth(width);
    heightRange->minVal = height.minVal;
    heightRange->maxVal = height.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoWidthRange(OH_AVCapability *capability, OH_AVRange *widthRange)
{
    if (capability == nullptr) {
        widthRange->minVal = 0;
        widthRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto width = codecInfo->GetSupportedWidth();
    widthRange->minVal = width.minVal;
    widthRange->maxVal = width.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoHeightRange(OH_AVCapability *capability, OH_AVRange *heightRange)
{
    if (capability == nullptr) {
        heightRange->minVal = 0;
        heightRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }

    std::shared_ptr<VideoCaps> codecInfo = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto height = codecInfo->GetSupportedHeight();
    heightRange->minVal = height.minVal;
    heightRange->maxVal = height.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_IsVideoSizeSupported(OH_AVCapability *capability, int32_t width, int32_t height)
{
    if (capability == nullptr) {
        return false;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    return videoCap->IsSizeSupported(width, height);
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRange(OH_AVCapability *capability, OH_AVRange *frameRateRange)
{
    if (capability == nullptr) {
        frameRateRange->minVal = 0;
        frameRateRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto ret = videoCap->GetSupportedFrameRate();
    frameRateRange->minVal = ret.minVal;
    frameRateRange->maxVal = ret.maxVal;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVCapability_GetVideoFrameRateRangeForSize(OH_AVCapability *capability, int32_t width, int32_t height,
                                                           OH_AVRange *frameRateRange)
{
    if (capability == nullptr) {
        frameRateRange->minVal = 0;
        frameRateRange->maxVal = 0;
        return AV_ERR_INVALID_VAL;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    auto ret = videoCap->GetSupportedFrameRatesFor(width, height);
    frameRateRange->minVal = ret.minVal;
    frameRateRange->maxVal = ret.maxVal;
    return AV_ERR_OK;
}

bool OH_AVCapability_AreVideoSizeAndFrameRateSupported(OH_AVCapability *capability, int32_t width, int32_t height,
                                                       int32_t frameRate)
{
    if (capability == nullptr) {
        return false;
    }
    std::shared_ptr<VideoCaps> videoCap = std::make_shared<VideoCaps>(capability->capabilityData_);
    return videoCap->IsSizeAndRateSupported(width, height, frameRate);
}