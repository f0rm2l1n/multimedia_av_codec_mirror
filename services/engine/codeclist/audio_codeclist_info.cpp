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
CapabilityData GetMP3DecoderCapacity()
{
    CapabilityData AUDIO_MP3_DECODER_CAPABILITY;
    AUDIO_MP3_DECODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY;
    AUDIO_MP3_DECODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AUDIO_MP3_DECODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_MPEG;
    AUDIO_MP3_DECODER_CAPABILITY.isVendor = false;
    AUDIO_MP3_DECODER_CAPABILITY.bitrate = Range(1, 320000);
    AUDIO_MP3_DECODER_CAPABILITY.channels = Range(1, 2);
    AUDIO_MP3_DECODER_CAPABILITY.sampleRate = {8000,  11025, 12000, 16000, 22050, 24000,
                                               32000, 44100, 48000, 64000, 88200, 96000};
    return AUDIO_MP3_DECODER_CAPABILITY;
}

CapabilityData GetAACDecoderCapacity()
{
    CapabilityData AUDIO_AAC_DECODER_CAPABILITY;
    AUDIO_AAC_DECODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_DECODER_AAC_NAME_KEY;
    AUDIO_AAC_DECODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AUDIO_AAC_DECODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    AUDIO_AAC_DECODER_CAPABILITY.isVendor = false;
    AUDIO_AAC_DECODER_CAPABILITY.bitrate = Range(8000, 960000);
    AUDIO_AAC_DECODER_CAPABILITY.channels = Range(1, 8);
    AUDIO_AAC_DECODER_CAPABILITY.sampleRate = {8000,  11025, 12000, 16000, 22050, 24000,
                                               32000, 44100, 48000, 64000, 88200, 96000};
    return AUDIO_AAC_DECODER_CAPABILITY;
}

CapabilityData GetFlacDecoderCapacity()
{
    CapabilityData AUDIO_FLAC_DECODER_CAPABILITY;
    AUDIO_FLAC_DECODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_DECODER_FLAC_NAME_KEY;
    AUDIO_FLAC_DECODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AUDIO_FLAC_DECODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    AUDIO_FLAC_DECODER_CAPABILITY.isVendor = false;
    AUDIO_FLAC_DECODER_CAPABILITY.bitrate = Range(1, 2100000);
    AUDIO_FLAC_DECODER_CAPABILITY.channels = Range(1, 8);
    AUDIO_FLAC_DECODER_CAPABILITY.sampleRate = {8000,  11025, 12000, 16000, 22050, 24000,
                                                32000, 44100, 48000, 64000, 88200, 96000};
    return AUDIO_FLAC_DECODER_CAPABILITY;
}

CapabilityData GetVorbisDecoderCapacity()
{
    CapabilityData AUDIO_VORBIS_DECODER_CAPABILITY;
    AUDIO_VORBIS_DECODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_DECODER_VORBIS_NAME_KEY;
    AUDIO_VORBIS_DECODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_DECODER;
    AUDIO_VORBIS_DECODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_VORBIS;
    AUDIO_VORBIS_DECODER_CAPABILITY.isVendor = false;
    AUDIO_VORBIS_DECODER_CAPABILITY.bitrate = Range(32000, 500000);
    AUDIO_VORBIS_DECODER_CAPABILITY.channels = Range(1, 8);
    AUDIO_VORBIS_DECODER_CAPABILITY.sampleRate = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};
    return AUDIO_VORBIS_DECODER_CAPABILITY;
}

CapabilityData GetAACEncoderCapacity()
{
    CapabilityData AUDIO_AAC_ENCODER_CAPABILITY;
    AUDIO_AAC_ENCODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_ENCODER_AAC_NAME_KEY;
    AUDIO_AAC_ENCODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    AUDIO_AAC_ENCODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC;
    AUDIO_AAC_ENCODER_CAPABILITY.isVendor = false;
    AUDIO_AAC_ENCODER_CAPABILITY.bitrate = Range(8000, 448000);
    AUDIO_AAC_ENCODER_CAPABILITY.channels = Range(1, 8);
    AUDIO_AAC_ENCODER_CAPABILITY.sampleRate = {8000,  12000, 16000, 22050, 24000, 32000,
                                               44100, 48000, 64000, 88200, 96000};
    return AUDIO_AAC_ENCODER_CAPABILITY;
}

CapabilityData GetFlacEncoderCapacity()
{
    CapabilityData AUDIO_FLAC_ENCODER_CAPABILITY;
    AUDIO_FLAC_ENCODER_CAPABILITY.codecName = AVCodecCodecName::AUDIO_ENCODER_FLAC_NAME_KEY;
    AUDIO_FLAC_ENCODER_CAPABILITY.codecType = AVCODEC_TYPE_AUDIO_ENCODER;
    AUDIO_FLAC_ENCODER_CAPABILITY.mimeType = AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC;
    AUDIO_FLAC_ENCODER_CAPABILITY.isVendor = false;
    AUDIO_FLAC_ENCODER_CAPABILITY.bitrate = Range(1, 2100000);
    AUDIO_FLAC_ENCODER_CAPABILITY.channels = Range(1, 8);
    AUDIO_FLAC_ENCODER_CAPABILITY.sampleRate = {8000,  11025, 12000, 16000, 22050, 24000,
                                                32000, 44100, 48000, 64000, 88200, 96000};
    return AUDIO_FLAC_ENCODER_CAPABILITY;
}

AudioCodeclistInfo::AudioCodeclistInfo()
{
    audioCapcities_ = {GetMP3DecoderCapacity(),    GetAACDecoderCapacity(), GetFlacDecoderCapacity(),
                       GetVorbisDecoderCapacity(), GetAACEncoderCapacity(), GetFlacEncoderCapacity()};
}

AudioCodeclistInfo::~AudioCodeclistInfo()
{
    audioCapcities_.clear();
}

AudioCodeclistInfo &AudioCodeclistInfo::GetInstance()
{
    static AudioCodeclistInfo audioCodecList;
    return audioCodecList;
}

std::vector<CapabilityData> AudioCodeclistInfo::GetAudioCapacities() const noexcept
{
    return audioCapcities_;
}
} // namespace Media
} // namespace OHOS