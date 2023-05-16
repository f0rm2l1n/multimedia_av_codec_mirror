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

#ifndef AUDIO_FFMPEG_FLAC_DECODER_PLUGIN_H
#define AUDIO_FFMPEG_FLAC_DECODER_PLUGIN_H

#include "audio_ffmpeg_base_codec.h"
#include "audio_ffmpeg_decoder_plugin.h"
#include "avcodec_codec_key.h"

namespace OHOS {
namespace Media {
class AudioFFMpegFlacDecoderPlugin : public AudioFFMpegBaseCodec::CodecRegister<AudioFFMpegFlacDecoderPlugin> {
public:
    AudioFFMpegFlacDecoderPlugin();
    ~AudioFFMpegFlacDecoderPlugin() override;

    int32_t init(const Format &format) override;
    int32_t processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) override;
    int32_t processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) override;
    int32_t reset() override;
    int32_t release() override;
    int32_t flush() override;
    uint32_t getInputBufferSize() const override;
    uint32_t getOutputBufferSize() const override;
    Format GetFormat() const noexcept override;
    const static std::string identify()
    {
        return std::string(AVCodecCodecKey::AUDIO_DECODER_FLAC_NAME_KEY);
    }

private:
    std::unique_ptr<AudioFfmpegDecoderPlugin> basePlugin;
};
} // namespace Media
} // namespace OHOS
#endif