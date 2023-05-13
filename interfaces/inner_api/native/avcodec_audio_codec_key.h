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
#ifndef AVCODEC_AUDIO_CODEC_KEY_H
#define AVCODEC_AUDIO_CODEC_KEY_H
#include <string_view>

namespace OHOS {
namespace Media {

class AVCodecAudioCodecKey {
public:
    static constexpr std::string_view AUDIO_DECODER_MP3_NAME_KEY = "avdec_mp3";
    static constexpr std::string_view AUDIO_DECODER_AAC_NAME_KEY = "avdec_aac";
    static constexpr std::string_view AUDIO_DECODER_VORBIS_NAME_KEY = "avdec_vorbis";
    static constexpr std::string_view AUDIO_DECODER_FLAC_NAME_KEY = "avdec_flac";

    static constexpr std::string_view AUDIO_ENCODER_FLAC_NAME_KEY = "avenc_flac";
    static constexpr std::string_view AUDIO_ENCODER_AAC_NAME_KEY = "avenc_aac";

private:
    AVCodecAudioCodecKey() = delete;
    ~AVCodecAudioCodecKey() = delete;
};

} // namespace Media
} // namespace OHOS

#endif