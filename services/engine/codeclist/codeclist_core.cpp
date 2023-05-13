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

#include <cmath> // fabs
#include "codec_ability_singleton.h"
#include "avcodec_log.h"
#include "codeclist_core.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecListCore"};
constexpr float EPSINON = 0.0001;
} // namespace

namespace OHOS {
namespace Media {
CodecListCore::CodecListCore()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecListCore::~CodecListCore()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool CodecListCore::CheckBitrate(const Format &format, const CapabilityData &data)
{
    int32_t targetBitrate;
    if (!format.ContainKey("bitrate")) {
        AVCODEC_LOGD("The bitrate of the format are not specified");
        return true;
    }
    (void)format.GetIntValue("bitrate", targetBitrate);
    if (data.bitrate.minVal > targetBitrate || data.bitrate.maxVal < targetBitrate) {
        return false;
    }
    return true;
}

bool CodecListCore::CheckVideoResolution(const Format &format, const CapabilityData &data)
{
    int32_t targetWidth;
    int32_t targetHeight;
    if ((!format.ContainKey("width")) || (!format.ContainKey("height"))) {
        AVCODEC_LOGD("The width and height of the format are not specified");
        return true;
    }
    (void)format.GetIntValue("width", targetWidth);
    (void)format.GetIntValue("height", targetHeight);
    if (data.width.minVal > targetWidth || data.width.maxVal < targetWidth || data.height.minVal > targetHeight ||
        data.height.maxVal < targetHeight) {
        return false;
    }
    return true;
}

bool CodecListCore::CheckVideoPixelFormat(const Format &format, const CapabilityData &data)
{
    int32_t targetPixelFormat;
    if (!format.ContainKey("pixel_format")) {
        AVCODEC_LOGD("The pixel_format of the format are not specified");
        return true;
    }
    (void)format.GetIntValue("pixel_format", targetPixelFormat);
    if (find(data.pixFormat.begin(), data.pixFormat.end(), targetPixelFormat) == data.pixFormat.end()) {
        return false;
    }
    return true;
}

bool CodecListCore::CheckVideoFrameRate(const Format &format, const CapabilityData &data)
{
    if (!format.ContainKey("frame_rate")) {
        AVCODEC_LOGD("The frame_rate of the format are not specified");
        return true;
    }

    switch (format.GetValueType(std::string_view("frame_rate"))) {
    case FORMAT_TYPE_INT32: {
        int32_t targetFrameRateInt;
        (void)format.GetIntValue("frame_rate", targetFrameRateInt);
        if (data.frameRate.minVal > targetFrameRateInt || data.frameRate.maxVal < targetFrameRateInt) {
            return false;
        }
        break;
    }
    case FORMAT_TYPE_DOUBLE: {
        double targetFrameRateDouble;
        (void)format.GetDoubleValue("frame_rate", targetFrameRateDouble);
        double minValDouble{data.frameRate.minVal};
        double maxValDouble{data.frameRate.maxVal};
        if ((minValDouble > targetFrameRateDouble && fabs(minValDouble - targetFrameRateDouble) >= EPSINON) ||
            (maxValDouble < targetFrameRateDouble && fabs(maxValDouble - targetFrameRateDouble) >= EPSINON)) {
            return false;
        }
        break;
    }
    default:
        break;
    }
    return true;
}

bool CodecListCore::CheckAudioChannel(const Format &format, const CapabilityData &data)
{
    int32_t targetChannel;
    if (!format.ContainKey("channel_count")) {
        AVCODEC_LOGD("The channel_count of the format are not specified");
        return true;
    }
    (void)format.GetIntValue("channel_count", targetChannel);
    if (data.channels.minVal > targetChannel || data.channels.maxVal < targetChannel) {
        return false;
    }
    return true;
}

bool CodecListCore::CheckAudioSampleRate(const Format &format, const CapabilityData &data)
{
    int32_t targetSampleRate;
    if (!format.ContainKey("samplerate")) {
        AVCODEC_LOGD("The samplerate of the format are not specified");
        return true;
    }
    (void)format.GetIntValue("samplerate", targetSampleRate);
    if (find(data.sampleRate.begin(), data.sampleRate.end(), targetSampleRate) == data.sampleRate.end()) {
        return false;
    }
    return true;
}

bool CodecListCore::IsVideoCapSupport(const Format &format, const CapabilityData &data)
{
    return CheckVideoResolution(format, data) && CheckVideoPixelFormat(format, data) &&
           CheckVideoFrameRate(format, data) && CheckBitrate(format, data);
}

bool CodecListCore::IsAudioCapSupport(const Format &format, const CapabilityData &data)
{
    return CheckAudioChannel(format, data) && CheckAudioSampleRate(format, data) && CheckBitrate(format, data);
}

// mime是必要参数
std::string CodecListCore::FindCodec(const Format &format, bool isEncoder)
{
    (void)isEncoder;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!format.ContainKey("codec_mime")) {
        AVCODEC_LOGD("Get MimeType from format failed");
        return "";
    }
    std::string targetMimeType;
    (void)format.GetStringValue("codec_mime", targetMimeType);

    AVCodecType codecType = AVCODEC_TYPE_NONE;
    bool isVideo = targetMimeType.find("video") != std::string::npos;
    if (isVideo) {
        codecType = isEncoder ? AVCODEC_TYPE_VIDEO_ENCODER : AVCODEC_TYPE_VIDEO_DECODER;
    } else {
        codecType = isEncoder ? AVCODEC_TYPE_AUDIO_ENCODER : AVCODEC_TYPE_AUDIO_DECODER;
    }

    int isVendor = -1;
    bool isVendorKey = format.ContainKey("codec_vendor_flag");
    if (isVendorKey) {
        (void)format.GetIntValue("codec_vendor_flag", isVendor);
    }
    std::vector<CapabilityData> capabilityDataArray = CodecAbilitySingleton::GetInstance().GetCapabilityArray();
    auto iter = capabilityDataArray.begin();
    while (iter != capabilityDataArray.end()) {
        if ((*iter).codecType != codecType || (*iter).mimeType != targetMimeType ||
            (isVendorKey && (*iter).isVendor != isVendor)) {
            ++iter;
            continue;
        }

        if (isVideo) {
            if (IsVideoCapSupport(format, *iter)) {
                return (*iter).codecName;
            }
        } else {
            if (IsAudioCapSupport(format, *iter)) {
                return (*iter).codecName;
            }
        }
        ++iter;
    }
    return "";
}

std::string CodecListCore::FindEncoder(const Format &format)
{
    return FindCodec(format, true);
}

std::string CodecListCore::FindDecoder(const Format &format)
{
    return FindCodec(format, false);
}

CapabilityData CodecListCore::CreateCapability(std::string codecName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CapabilityData capData;
    if (codecName.empty()) {
        return capData;
    }
    std::vector<CapabilityData> capabilityDataArray = CodecAbilitySingleton::GetInstance().GetCapabilityArray();
    auto iter = capabilityDataArray.begin();
    while (iter != capabilityDataArray.end()) {
        if (codecName == ((*iter).codecName)) {
            capData = (*iter);
            break;
        }
        ++iter;
    }
    return capData;
}

} // namespace Media
} // namespace OHOS