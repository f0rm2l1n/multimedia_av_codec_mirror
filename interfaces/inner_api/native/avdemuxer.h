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


#ifndef AVDEMUXER_H
#define AVDEMUXER_H

#include <memory>
#include "avcodec_common.h"
#include "avsource.h"

namespace OHOS {
namespace Media {
class AVDemuxer {
public:
    ~AVDemuxer() = default;

    /**
     * @brief Add the sourceTrack by track id.
     * This function can only by called before {@link CopyNextSample}.
     * @param trackIndex The index of the track to add.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SelectSourceTrackByID(uint32_t trackIndex) = 0;

    /**
     * @brief Remove the sourceTrack by track id.
     * This function can only by called before {@link CopyNextSample}.
     * @param trackIndex The index of the track to remove.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t UnselectSourceTrackByID(uint32_t trackIndex) = 0;

    /**
     * @brief Copy the current sample to buffer, and save buffer attribute to attr.
     * @param trackIndex The param storing trackIndex of the buffer.
     * @param buffer The buffer pointer to store data.
     * @param attr The CodecBufferAttr pointer to store data attribute.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                   AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag) = 0;

    /**
     * @brief All selected tracks seek near to the requested time according to the seek mode.
     * @param mSeconds The millisecond for seeking, the timestamp is the position of
     * the file relative to the start of the file..
     * @param mode The mode for seeking. Value. For details, see {@link SeekMode}.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.0
     * @version 4.0
     */
    virtual int32_t SeekToTime(int64_t mSeconds, AVSeekMode mode) = 0;
};

class __attribute__((visibility("default"))) AVDemuxerFactory {
public:
#ifdef UNSUPPORT_DEMUXER
    static std::shared_ptr<AVDemuxer> CreateWithSource(AVSource& source)
    {
        return nullptr;
    }
#else
    /**
     * @brief Instantiate the preferred demuxer of the given source instance.
     * @param sourceAddr The address for source instance.
     * @return Returns the preferred demuxer.
     * @since 4.0
     * @version 4.0
     */
    static std::shared_ptr<AVDemuxer> CreateWithSource(AVSource& source);
#endif
private:
    AVDemuxerFactory() = default;
    ~AVDemuxerFactory() = default;
};
} // namespace Media
} // namespace OHOS
#endif // AVDEMUXER_H
