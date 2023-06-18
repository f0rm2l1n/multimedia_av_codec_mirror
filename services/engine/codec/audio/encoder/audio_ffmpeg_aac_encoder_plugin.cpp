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

#include "audio_ffmpeg_aac_encoder_plugin.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "media_description.h"
#include "avcodec_mime_type.h"
#include "ffmpeg_converter.h"
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegAacEncoderPlugin"};
static const uint64_t AAC_CHANNEL_LAYOUT_TABLE[] = {
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_5POINT0_BACK,
    AV_CH_LAYOUT_5POINT1_BACK,
    AV_CH_LAYOUT_7POINT1,
};
constexpr std::string_view AUDIO_CODEC_NAME = "aac";
constexpr int32_t INPUT_BUFFER_SIZE_DEFAULT = 4 * 1024 * 8;
constexpr int32_t OUTPUT_BUFFER_SIZE_DEFAULT = 8192;
constexpr uint32_t ADTS_HEADER_SIZE = 7;
constexpr uint8_t SAMPLE_FREQUENCY_INDEX_DEFAULT = 4;
constexpr int32_t MIN_CHANNELS = 1;
constexpr int32_t MAX_CHANNELS = 8;
static std::map<int32_t, uint8_t> sampleFreqMap = {{96000, 0},  {88200, 1}, {64000, 2}, {48000, 3}, {44100, 4},
                                                   {32000, 5},  {24000, 6}, {22050, 7}, {16000, 8}, {12000, 9},
                                                   {11025, 10}, {8000, 11}, {7350, 12}};
}

namespace OHOS {
namespace MediaAVcodec {
AudioFFMpegAacEncoderPlugin::AudioFFMpegAacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>()) {}

AudioFFMpegAacEncoderPlugin::~AudioFFMpegAacEncoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static int32_t GetAdtsHeader(std::string &adtsHeader, uint32_t &headerSize, std::shared_ptr<AVCodecContext> ctx,
                             int aacLength)
{
    uint8_t freqIdx = SAMPLE_FREQUENCY_INDEX_DEFAULT; // 0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    auto iter = sampleFreqMap.find(ctx->sample_rate);
    if (iter != sampleFreqMap.end()) {
        freqIdx = iter->second;
    }
    uint8_t chanCfg = ctx->channels;
    uint32_t frameLength = aacLength + ADTS_HEADER_SIZE;
    adtsHeader += 0xFF;
    adtsHeader += 0xF1;
    adtsHeader += ((ctx->profile) << 0x6) + (freqIdx << 0x2) + (chanCfg >> 0x2);
    adtsHeader += (((chanCfg & 0x3) << 0x6) + (frameLength >> 0xB));
    adtsHeader += ((frameLength & 0x7FF) >> 0x3);
    adtsHeader += (((frameLength & 0x7) << 0x5) + 0x1F);
    adtsHeader += 0xFC;
    headerSize = ADTS_HEADER_SIZE;

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

static bool CheckSampleRate(const std::shared_ptr<AVCodec> &codec, const int sampleRate)
{
    return sampleFreqMap.find(sampleRate) != sampleFreqMap.end() ? true : false;
}

static bool CheckChannelLayout(const uint64_t channelLayout)
{
    for (auto i : AAC_CHANNEL_LAYOUT_TABLE) {
        if (i == channelLayout) {
            return true;
        }
    }
    return false;
}

bool AudioFFMpegAacEncoderPlugin::CheckFormat(const Format &format) const
{
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_RATE) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT)) {
        AVCODEC_LOGE("Format parameter missing");
        return false;
    }

    auto avCodec = basePlugin->GetAVCodec();
    int sampleRate;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate);
    if (!CheckSampleRate(avCodec, sampleRate)) {
        AVCODEC_LOGE("Sample rate not supported");
        return false;
    }

    int64_t channelLayout;
    format.GetLongValue(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT, channelLayout);
    auto ffChannelLayout =
        FFMpegConverter::ConvertOHAudioChannelLayoutToFFMpeg(static_cast<AudioChannelLayout>(channelLayout));
    if (!CheckChannelLayout(ffChannelLayout)) {
        AVCODEC_LOGE("Channel layout not supported");
        return false;
    }

    int channels;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels);
    if (channels < MIN_CHANNELS || channels > MAX_CHANNELS) {
        return false;
    }

    return true;
}

int32_t AudioFFMpegAacEncoderPlugin::Init(const Format &format)
{
    int32_t ret = basePlugin->AllocateContext("aac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Allocat aac context failed, ret = %{publid}d", ret);
        return ret;
    }
    if (!CheckFormat(format)) {
        AVCODEC_LOGE("Format check failed.");
        return AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_AUD_PARAMS;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Init context failed, ret = %{publid}d", ret);
        return ret;
    }
    ret = basePlugin->OpenContext();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Open context failed, ret = %{publid}d", ret);
        return ret;
    }

    ret = basePlugin->InitFrame();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Init frame failed, ret = %{publid}d", ret);
        return ret;
    }

    basePlugin->RegisterHeaderFunc(GetAdtsHeader);

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAacEncoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::Reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegAacEncoderPlugin::Release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegAacEncoderPlugin::Flush()
{
    return basePlugin->Flush();
}

int32_t AudioFFMpegAacEncoderPlugin::GetInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > INPUT_BUFFER_SIZE_DEFAULT) {
        maxSize = INPUT_BUFFER_SIZE_DEFAULT;
    }
    return maxSize;
}

int32_t AudioFFMpegAacEncoderPlugin::GetOutputBufferSize() const
{
    return OUTPUT_BUFFER_SIZE_DEFAULT;
}

Format AudioFFMpegAacEncoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC);
    return format;
}

std::string_view AudioFFMpegAacEncoderPlugin::GetCodecType() const noexcept
{
    return AUDIO_CODEC_NAME;
}
} // namespace MediaAVCodec
} // namespace OHOS