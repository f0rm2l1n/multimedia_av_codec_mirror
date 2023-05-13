/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef MEDIA_DESCRIPTION_H
#define MEDIA_DESCRIPTION_H

#include "format.h"

namespace OHOS {
namespace Media {
/**
 * @brief Provides the uniform container for storing the media description.
 */
using MediaDescription = Format;

/**
 * @brief Provides the key's definition for MediaDescription.
 */
class MediaDescriptionKey {
public:
    /**
     * Key for track index, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_TRACK_INDEX = "track_index";

    /**
     * Key for track type, value type is uint8_t, see {link @MediaTrackType}
     */
    static constexpr std::string_view MD_KEY_TRACK_TYPE = "track_type";

    /**
     * Key for codec mime type, value type is string
     */
    static constexpr std::string_view MD_KEY_CODEC_MIME = "codec_mime";

    /**
     * Key for codec name, value type is string
     */
    static constexpr std::string_view MD_KEY_CODEC_NAME = "codec_name";

    /**
     * Key for duration, value type is int64_t
     */
    static constexpr std::string_view MD_KEY_DURATION = "duration";

    /**
     * Key for bitrate, value type is int64_t
     */
    static constexpr std::string_view MD_KEY_BITRATE = "bitrate";

    /**
     * Key for max input size, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_MAX_INPUT_SIZE = "max_input_size";

    /**
     * Key for max input buffer count, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_MAX_INPUT_BUFFER_COUNT = "max_input_buffer_count";

    /**
     * Key for max output buffer count, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_MAX_OUTPUT_BUFFER_COUNT = "max_output_buffer_count";

    /**
     * Key for video width, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_WIDTH = "width";

    /**
     * Key for video height, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_HEIGHT = "height";

    /**
     * Key for video pixelformat, value type is int32_t, see {link @MediaPixelFormat}
     */
    static constexpr std::string_view MD_KEY_PIXEL_FORMAT = "pixel_format";

    /**
     * Key for video scale type, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_SCALE_TYPE = "scale_type";

    /**
     * Key for video rotation angle, value type is int32_t
     */
    static constexpr std::string_view MD_KEY_ROTATION_ANGLE = "rotation_angle";


    /**
     * Key for video frame rate, value type is double.
     */
    static constexpr std::string_view MD_KEY_FRAME_RATE = "frame_rate";

    /**
     * Key for video capture rate, value type is double
     */
    static constexpr std::string_view MD_KEY_CAPTURE_RATE = "capture_rate";

    /**
     * Key for the interval of key frame. value type is int32_t, the unit is milliseconds.
     * A negative value means no key frames are requested after the first frame. A zero
     * value means a stream containing all key frames is requested.
     */
    static constexpr std::string_view MD_KEY_I_FRAME_INTERVAL = "i_frame_interval";

    /**
     * Key for the request a I-Frame immediately. value type is boolean
     */
    static constexpr std::string_view MD_KEY_REQUEST_I_FRAME = "req_i_frame";

    /**
     * Key for audio channel count, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_CHANNEL_COUNT = "channel_count";

    /**
     * Key for audio sample rate, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_SAMPLE_RATE = "sample_rate";

    /**
     * Key for track count in the container, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_TRACK_COUNT = "track_count";

    /**
     * Key for container format type, value type is string
     */
    static constexpr std::string_view MD_KEY_CONTAINER_FORMAT = "container_format";

    /**
     * custom key prefix, media service will pass through to HAL.
     */
    static constexpr std::string_view MD_KEY_CUSTOM_PREFIX = "vendor.custom";

    /**
     * Key for codec specific data buffer, vlaue type is uint8_t*
    */
    static constexpr std::string_view MD_KEY_CODEC_CONFIG = "codec_config";

    /**
     * Key for audio channel layout, value type is int64_t
     */
    static constexpr std::string_view MD_KEY_CHANNEL_LAYOUT = "channel_layout";

    /**
     * Key for audio sample format, value type is uint32_t
     */
    static constexpr std::string_view MD_KEY_SAMPLE_FORMAT = "sample_format";

    /**
     * Key for the number of bits used to encode each sample, value type is uint32_t
     */
    static constexpr std::string_view MD_BITS_PER_CODED_SAMPLE_KEY = "bits_per_coded_sample";

    /**
     * Key for aac type, value type is uint32_t
     */
    static constexpr std::string_view MD_AAC_TYPE_KEY = "aac-type";

private:
    MediaDescriptionKey() = delete;
    ~MediaDescriptionKey() = delete;
};
} // namespace Media
} // namespace OHOS
#endif // MEDIA_DESCRIPTION_H
