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
const std::vector<int32_t> AUDIO_SMAPLE_RATE = {8000,  11025, 12000, 16000, 22050, 24000,
                                                32000, 44100, 48000, 64000, 88200, 96000};
constexpr int MAX_AUDIO_CHANNEL_COUNT = 8;

constexpr int MAX_BIT_RATE_MP3 = 320000;
constexpr int MAX_CHANNEL_COUNT_MP3 = 2;

constexpr int MIN_BIT_RATE_AAC = 8000;
constexpr int MAX_BIT_RATE_AAC = 960000;
const std::vector<int32_t> AUDIO_VORBIS_SMAPLE_RATE = {8000,  11025, 12000, 16000, 22050, 24000,
                                                       32000, 44100, 48000, 64000, 88200, 96000};

constexpr int MAX_BIT_RATE_FLAC = 2100000;

constexpr int MIN_BIT_RATE_VORBIS = 32000;
constexpr int MAX_BIT_RATE_VORBIS = 500000;

constexpr int MIN_BIT_RATE_AAC_ENCODER = 8000;
constexpr int MAX_BIT_RATE_AAC_ENCODER = 448000;

CapabilityData GetMP3DecoderCapbility()
{
    CapabilityData AudioMp3Capbility;
    AudioMp3Capbility.codecName = AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY;
    AudioMp3Capbility.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AudioMp3Capbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_MPEG;
    AudioMp3Capbility.isVendor = false;
    AudioMp3Capbility.bitrate = Range(1, MAX_BIT_RATE_MP3);
    AudioMp3Capbility.channels = Range(1, MAX_CHANNEL_COUNT_MP3);
    AudioMp3Capbility.sampleRate = AUDIO_SMAPLE_RATE;
    return AudioMp3Capbility;
}

CapabilityData GetAACDecoderCapbility()
{
    CapabilityData AudioAACCapbility;
    AudioAACCapbility.codecName = AVCodecCodecName::AUDIO_DECODER_AAC_NAME_KEY;
    AudioAACCapbility.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AudioAACCapbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    AudioAACCapbility.isVendor = false;
    AudioAACCapbility.bitrate = Range(MIN_BIT_RATE_AAC, MAX_BIT_RATE_AAC);
    AudioAACCapbility.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    AudioAACCapbility.sampleRate = AUDIO_SMAPLE_RATE;
    return AudioAACCapbility;
}

CapabilityData GetFlacDecoderCapbility()
{
    CapabilityData AudioFlacCapbility;
    AudioFlacCapbility.codecName = AVCodecCodecName::AUDIO_DECODER_FLAC_NAME_KEY;
    AudioFlacCapbility.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AudioFlacCapbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    AudioFlacCapbility.isVendor = false;
    AudioFlacCapbility.bitrate = Range(1, MAX_BIT_RATE_FLAC);
    AudioFlacCapbility.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    AudioFlacCapbility.sampleRate = AUDIO_SMAPLE_RATE;
    return AudioFlacCapbility;
}

CapabilityData GetVorbisDecoderCapbility()
{
    CapabilityData AudioVorbisCapbility;
    AudioVorbisCapbility.codecName = AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME_KEY;
    AudioVorbisCapbility.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AudioVorbisCapbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_VORBIS;
    AudioVorbisCapbility.isVendor = false;
    AudioVorbisCapbility.bitrate = Range(MIN_BIT_RATE_VORBIS, MAX_BIT_RATE_VORBIS);
    AudioVorbisCapbility.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    AudioVorbisCapbility.sampleRate = AUDIO_VORBIS_SMAPLE_RATE;
    return AudioVorbisCapbility;
}

CapabilityData GetAACEncoderCapbility()
{
    CapabilityData AudioAACCapbility;
    AudioAACCapbility.codecName = AVCodecCodecName::AUDIO_ENCODER_AAC_NAME_KEY;
    AudioAACCapbility.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    AudioAACCapbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    AudioAACCapbility.isVendor = false;
    AudioAACCapbility.bitrate = Range(MIN_BIT_RATE_AAC_ENCODER, MAX_BIT_RATE_AAC_ENCODER);
    AudioAACCapbility.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    AudioAACCapbility.sampleRate = AUDIO_SMAPLE_RATE;
    return AudioAACCapbility;
}

CapabilityData GetFlacEncoderCapbility()
{
    CapabilityData AudioFlacCapbility;
    AudioFlacCapbility.codecName = AVCodecCodecName::AUDIO_ENCODER_FLAC_NAME_KEY;
    AudioFlacCapbility.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    AudioFlacCapbility.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    AudioFlacCapbility.isVendor = false;
    AudioFlacCapbility.bitrate = Range(1, MAX_BIT_RATE_FLAC);
    AudioFlacCapbility.channels = Range(1, MAX_AUDIO_CHANNEL_COUNT);
    AudioFlacCapbility.sampleRate = AUDIO_SMAPLE_RATE;
    return AudioFlacCapbility;
}

AudioCodeclistInfo::AudioCodeclistInfo()
{
    audioCapbilities_ = {GetMP3DecoderCapbility(),    GetAACDecoderCapbility(), GetFlacDecoderCapbility(),
                         GetVorbisDecoderCapbility(), GetAACEncoderCapbility(), GetFlacEncoderCapbility()};
}

AudioCodeclistInfo::~AudioCodeclistInfo()
{
    audioCapbilities_.clear();
}

AudioCodeclistInfo &AudioCodeclistInfo::GetInstance()
{
    static AudioCodeclistInfo audioCodecList;
    return audioCodecList;
}

std::vector<CapabilityData> AudioCodeclistInfo::GetAudioCapabilities() const noexcept
{
    return audioCapbilities_;
}
} // namespace Media
} // namespace OHOS