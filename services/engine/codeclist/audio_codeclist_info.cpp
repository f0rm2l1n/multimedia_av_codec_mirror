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

#include "audio_codeclist_info.h"
#include "avcodec_mime_type.h"
#include "avcodec_codec_name.h"

namespace OHOS {
namespace Media {
const std::vector<int32_t> AUDIO_SAMPLE_RATE = {8000,  11025, 12000, 16000, 22050, 24000,
                                                32000, 44100, 48000, 64000, 88200, 96000};
constexpr int MAX_AUDIO_CHANNEL_COUNT = 8;

constexpr int MAX_BIT_RATE_MP3 = 320000;
constexpr int MAX_CHANNEL_COUNT_MP3 = 2;

constexpr int MIN_BIT_RATE_AAC = 8000;
constexpr int MAX_BIT_RATE_AAC = 960000;
const std::vector<int32_t> AUDIO_VORBIS_SAMPLE_RATE = {8000,  11025, 12000, 16000, 22050, 24000,
                                                       32000, 44100, 48000, 64000, 88200, 96000};

constexpr int MAX_BIT_RATE_FLAC = 2100000;

constexpr int MIN_BIT_RATE_VORBIS = 32000;
constexpr int MAX_BIT_RATE_VORBIS = 500000;

constexpr int MIN_BIT_RATE_AAC_ENCODER = 8000;
constexpr int MAX_BIT_RATE_AAC_ENCODER = 448000;

CapabilityData GetMP3DecoderCapability()
{
    CapabilityData audioMp3Capability;
    audioMp3Capability.codecName = AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY;
    audioMp3Capability.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    audioMp3Capability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_MPEG;
    audioMp3Capability.isVendor = false;
    audioMp3Capability.bitrate = Range(1, MAX_BIT_RATE_MP3);
    audioMp3Capability.channels = Range(1, MAX_CHANNEL_COUNT_MP3);
    audioMp3Capability.sampleRate = AUDIO_SAMPLE_RATE;
    return audioMp3Capability;
}

CapabilityData GetAACDecoderCapability()
{
    CapabilityData audioAACCapability;
    audioAACCapability.codecName = AVCodecCodecName::AUDIO_DECODER_AAC_NAME_KEY;
    audioAACCapability.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    audioAACCapability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    audioAACCapability.isVendor = false;
    audioAACCapability.bitrate = Range(MIN_BIT_RATE_AAC, MAX_BIT_RATE_AAC);
    audioAACCapability.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    audioAACCapability.sampleRate = AUDIO_SAMPLE_RATE;
    return audioAACCapability;
}

CapabilityData GetFlacDecoderCapability()
{
    CapabilityData audioFlacCapability;
    audioFlacCapability.codecName = AVCodecCodecName::AUDIO_DECODER_FLAC_NAME_KEY;
    audioFlacCapability.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    audioFlacCapability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    audioFlacCapability.isVendor = false;
    audioFlacCapability.bitrate = Range(1, MAX_BIT_RATE_FLAC);
    audioFlacCapability.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    audioFlacCapability.sampleRate = AUDIO_SAMPLE_RATE;
    return audioFlacCapability;
}

CapabilityData GetVorbisDecoderCapability()
{
    CapabilityData audioVorbisCapability;
    audioVorbisCapability.codecName = AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME_KEY;
    audioVorbisCapability.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    audioVorbisCapability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_VORBIS;
    audioVorbisCapability.isVendor = false;
    audioVorbisCapability.bitrate = Range(MIN_BIT_RATE_VORBIS, MAX_BIT_RATE_VORBIS);
    audioVorbisCapability.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    audioVorbisCapability.sampleRate = AUDIO_VORBIS_SAMPLE_RATE;
    return audioVorbisCapability;
}

CapabilityData GetAACEncoderCapability()
{
    CapabilityData audioAACCapability;
    audioAACCapability.codecName = AVCodecCodecName::AUDIO_ENCODER_AAC_NAME_KEY;
    audioAACCapability.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    audioAACCapability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    audioAACCapability.isVendor = false;
    audioAACCapability.bitrate = Range(MIN_BIT_RATE_AAC_ENCODER, MAX_BIT_RATE_AAC_ENCODER);
    audioAACCapability.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    audioAACCapability.sampleRate = AUDIO_SAMPLE_RATE;
    return audioAACCapability;
}

CapabilityData GetFlacEncoderCapability()
{
    CapabilityData audioFlacCapability;
    audioFlacCapability.codecName = AVCodecCodecName::AUDIO_ENCODER_FLAC_NAME_KEY;
    audioFlacCapability.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    audioFlacCapability.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    audioFlacCapability.isVendor = false;
    audioFlacCapability.bitrate = Range(1, MAX_BIT_RATE_FLAC);
    audioFlacCapability.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    audioFlacCapability.sampleRate = AUDIO_SAMPLE_RATE;
    return audioFlacCapability;
}

AudioCodeclistInfo::AudioCodeclistInfo()
{
    audioCapabilities_ = {GetMP3DecoderCapability(),    GetAACDecoderCapability(), GetFlacDecoderCapability(),
                          GetVorbisDecoderCapability(), GetAACEncoderCapability(), GetFlacEncoderCapability()};
}

AudioCodeclistInfo::~AudioCodeclistInfo()
{
    audioCapabilities_.clear();
}

AudioCodeclistInfo &AudioCodeclistInfo::GetInstance()
{
    static AudioCodeclistInfo audioCodecList;
    return audioCodecList;
}

std::vector<CapabilityData> AudioCodeclistInfo::GetAudioCapabilities() const noexcept
{
    return audioCapabilities_;
}
} // namespace Media
} // namespace OHOS