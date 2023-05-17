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

#ifndef NATIVE_AVDEMUXER_H
#define NATIVE_AVDEMUXER_H

#include <stdint.h>
#include "native_avcodec_base.h"
#include "native_averrors.h"
#include "native_avsource.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AVDemuxer OH_AVDemuxer;

/**
 * @brief Creates a demuxer to demux media from source.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param source Pointer to an OH_AVSource instance.
 * @return Returns a pointer to an OH_AVDemuxer instance
 * @since 10
 * @version 4.0
*/
OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source);

/**
 * @brief Destroy the demuxer and free its resources.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer);

/**
 * @brief Add a track to the demuxer. Subsequent calls to `OH_AVDemuxer_CopyNextSample`
 * only retrieve information for the subset of tracks selected. One track can only be added once,
 * and add the same track multiple times has no effect.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param trackIndex Enter the index value corresponding to the track.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_SelectSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);

/**
 * @brief Remove a track from the demuxer. Subsequent calls to `OH_AVDemuxer_CopyNextSample`
 * only retrieve information for the subset of tracks selected.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param trackIndex Enter the index value corresponding to the Track.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_UnselectSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex);

/**
 * @brief Retrieve the current sample in selected tracks and store it in buffer, and store buffer's info to attr.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param trackIndex The param storing trackIndex of the buffer.
 * @param buffer The empty buffer for storing data.
 * @param attr The empty OH_AVCodecBufferAttr struct for storing buffer info.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_CopyNextSample(OH_AVDemuxer *demuxer, uint32_t *trackIndex,
                                         uint8_t *buffer, uint32_t bufferSize, OH_AVCodecBufferAttr *bufferInfo);

/**
 * @brief All selected tracks seek near to the requested time according to the seek mode.
 * @syscap SystemCapability.Multimedia.Media.Spliter
 * @param demuxer Pointer to an OH_AVDemuxer instance.
 * @param mSeconde The millisecond for seeking, the timestamp is the position of
 * the file relative to the start of the file.
 * @param mode The mode for seeking. Value is:
 *             SEEK_MODE_NEXT_SYNC     > seek to sync sample after the time.
 *             SEEK_MODE_PREVIOUS_SYNC > seek to sync sample before the time.
 *             SEEK_MODE_CLOSEST_SYNC  > seek to sync sample closest to time.
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 4.0
*/
OH_AVErrCode OH_AVDemuxer_SeekToTime(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVDEMUXER_H
