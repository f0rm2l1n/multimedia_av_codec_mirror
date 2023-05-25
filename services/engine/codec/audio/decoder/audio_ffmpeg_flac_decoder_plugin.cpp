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

#include "audio_ffmpeg_flac_decoder_plugin.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegFlacDecoderPlugin"};
constexpr int32_t GET_INPUT_BUFFER_SIZE = 65536;
constexpr int32_t GET_OUTPUT_BUFFER_SIZE = 65536;
constexpr int32_t MIN_CHANNELS = 1;
constexpr int32_t MAX_CHANNELS = 8;
static const uint32_t flacEncoderSampleRateTable[] = {
    0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
};
static const uint32_t flacEncoderBitsSampleTable[] = {16, 24, 32};
}

namespace OHOS {
namespace Media {
AudioFFMpegFlacDecoderPlugin::AudioFFMpegFlacDecoderPlugin() : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>())
{
}

AudioFFMpegFlacDecoderPlugin::~AudioFFMpegFlacDecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static bool CheckSampleRate(uint32_t sampleRate)
{
    for (auto i : flacEncoderSampleRateTable) {
        if (i == sampleRate) {
            return true;
        }
    }
    return false;
}

static bool CheckBitsPerSample(uint32_t bitsPerCodedSample)
{
    for (auto i : flacEncoderBitsSampleTable) {
        if (i == bitsPerCodedSample) {
            return true;
        }
    }
    return false;
}

int32_t AudioFFMpegFlacDecoderPlugin::CheckFormat(const Format &format) const
{
    int32_t channels;
    int32_t sampleRate;
    int32_t bitsPerCodedSample;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE, bitsPerCodedSample);
    if (!CheckSampleRate(sampleRate)) {
        AVCODEC_LOGE("init failed, because sampleRate=%{public}d not in table.", sampleRate);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    } else if (channels < MIN_CHANNELS || channels > MAX_CHANNELS) {
        AVCODEC_LOGE("init failed, because channels=%{public}d not support.", channels);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    } else if (!CheckBitsPerSample(bitsPerCodedSample)) {
        AVCODEC_LOGE("init failed, because bitsPerCodedSample=%{public}d not support.", bitsPerCodedSample);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_BIT_RATE;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacDecoderPlugin::Init(const Format &format)
{
    int32_t ret = basePlugin->AllocateContext("flac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because AllocateContext failed. ret=%{public}d", ret);
        return ret;
    }
    
    ret = CheckFormat(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because CheckFormat failed. ret=%{public}d", ret);
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
    
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacDecoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegFlacDecoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegFlacDecoderPlugin::Reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegFlacDecoderPlugin::Release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegFlacDecoderPlugin::Flush()
{
    return basePlugin->Flush();
}

int32_t AudioFFMpegFlacDecoderPlugin::GetInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > GET_INPUT_BUFFER_SIZE) {
        maxSize = GET_INPUT_BUFFER_SIZE;
    }
    return maxSize;
}

int32_t AudioFFMpegFlacDecoderPlugin::GetOutputBufferSize() const
{
    return GET_OUTPUT_BUFFER_SIZE;
}

Format AudioFFMpegFlacDecoderPlugin::GetFormat() const noexcept
{
    return basePlugin->GetFormat();
}
} // namespace Media
} // namespace OHOS