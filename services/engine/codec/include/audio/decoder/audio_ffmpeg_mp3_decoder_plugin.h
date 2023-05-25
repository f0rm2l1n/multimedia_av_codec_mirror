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

#ifndef AUDIO_FFMPEG_MP3_DECODER_PLUGIN_H
#define AUDIO_FFMPEG_MP3_DECODER_PLUGIN_H

#include "audio_ffmpeg_base_codec.h"
#include "audio_ffmpeg_decoder_plugin.h"
#include "avcodec_codec_name.h"

namespace OHOS {
namespace Media {
class AudioFFMpegMp3DecoderPlugin : public AudioFFMpegBaseCodec::CodecRegister<AudioFFMpegMp3DecoderPlugin> {
public:
    AudioFFMpegMp3DecoderPlugin();
    ~AudioFFMpegMp3DecoderPlugin() override;

    int32_t Init(const Format &format) override;
    int32_t ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) override;
    int32_t ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t Flush() override;
    uint32_t GetInputBufferSize() const override;
    uint32_t GetOutputBufferSize() const override;
    Format GetFormat() const noexcept override;

    const static std::string Identify()
    {
        return std::string(AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY);
    }

private:
    int32_t Checkinit(const Format &format);
    int channels;
    int sample_rate;
    int64_t bit_rate;
    std::unique_ptr<AudioFfmpegDecoderPlugin> basePlugin;
};
} // namespace Media
} // namespace OHOS
#endif