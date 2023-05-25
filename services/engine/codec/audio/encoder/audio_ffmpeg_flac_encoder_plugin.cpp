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

#include "audio_ffmpeg_flac_encoder_plugin.h"
#include "media_description.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "avcodec_mime_type.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegFlacEncoderPlugin"};
constexpr int minChannel = 1;
constexpr int maxChannel = 8;
constexpr int getInputBufferSize_ = 65536;
constexpr int getOutputBufferSize_ = 65536;
static const int flac_encoder_sample_rate_table[] = {
    0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
};
static const int flac_encoder_bits_sample_table[] = {16, 24, 32};
} // namespace

namespace OHOS {
namespace Media {
AudioFFMpegFlacEncoderPlugin::AudioFFMpegFlacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>())
{
}

AudioFFMpegFlacEncoderPlugin::~AudioFFMpegFlacEncoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static bool isTrueSampleRate(int sample)
{
    for (auto i : flac_encoder_sample_rate_table) {
        if (i == sample) {
            return true;
        }
    }
    return false;
}

static bool isTrueBitsPerSample(int bits_per_coded_sample)
{
    for (auto i : flac_encoder_bits_sample_table) {
        if (i == bits_per_coded_sample) {
            return true;
        }
    }
    return false;
}

int32_t AudioFFMpegFlacEncoderPlugin::init(const Format &format)
{
    int32_t channels, sample_rate;
    int32_t bits_per_coded_sample;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sample_rate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE, bits_per_coded_sample);
    if (!isTrueSampleRate(sample_rate)) {
        AVCODEC_LOGE("init failed, because sample rate=%{public}d not in table.", sample_rate);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    } else if (channels < minChannel || channels > maxChannel) {
        AVCODEC_LOGE("init failed, because channels=%{public}d not support.", channels);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    } else if (!isTrueBitsPerSample(bits_per_coded_sample)) {
        AVCODEC_LOGE("init failed, because bits_per_coded_sample=%{public}d not support.", bits_per_coded_sample);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_BIT_RATE;
    }

    int32_t ret = basePlugin->AllocateContext("flac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because AllocateContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because InitContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->OpenContext();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because OpenContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->InitFrame();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because InitFrame failed. ret=%{public}d", ret);
        return ret;
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacEncoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegFlacEncoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegFlacEncoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegFlacEncoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegFlacEncoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegFlacEncoderPlugin::getInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > getInputBufferSize_) {
        maxSize = getInputBufferSize_;
    }
    return maxSize;
}

uint32_t AudioFFMpegFlacEncoderPlugin::getOutputBufferSize() const
{
    return getOutputBufferSize_;
}

Format AudioFFMpegFlacEncoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC);
    return format;
}
} // namespace Media
} // namespace OHOS