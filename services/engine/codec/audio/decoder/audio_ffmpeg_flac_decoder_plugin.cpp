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

bool isTrueSampleRate(int sample)
{
    for (auto i : ff_flac_sample_rate_table) {
        if (i == sample) {
            return true;
        }
    }
    return false;
}

bool isTrueBitsPerSample(int bits_per_coded_rate)
{
    for (auto i : sample_size_table) {
        if (i == bits_per_coded_rate) {
            return true;
        }
    }
    return false;
}

int32_t AudioFFMpegFlacDecoderPlugin::init(const Format &format)
{
    int channels, sample_rate, bits_per_coded_rate;
    int64_t bit_rate;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sample_rate);
    format.GetLongValue(MediaDescriptionKey::MD_KEY_BITRATE, bit_rate);
    format.GetIntValue("bits_per_coded-rate", bits_per_coded_rate);
    if (!isTrueSampleRate(sample_rate)) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    } else if (channels < minChannels || channels > maxChannels) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    } else if (!isTrueBitsPerSample(bits_per_coded_rate)) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }

    int32_t ret = basePlugin->AllocateContext("flac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        // std::cout << "init 1 OH error:" << ret << "\n";
        return ret;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        // std::cout << "init 2 OH error:" << ret << "\n";
        return ret;
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegFlacDecoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegFlacDecoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegFlacDecoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegFlacDecoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegFlacDecoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegFlacDecoderPlugin::getInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > 65536) {
        maxSize = 65536;
    }
    return maxSize;
}

uint32_t AudioFFMpegFlacDecoderPlugin::getOutputBufferSize() const
{
    return 65536;
}

Format AudioFFMpegFlacDecoderPlugin::GetFormat() const noexcept
{
    return basePlugin->GetFormat();
}

} // namespace Media
} // namespace OHOS