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

#include <memory>
#include "avdemuxer.h"
#include "native_avmagic.h"
#include "avcodec_errors.h"
#include "native_object.h"
#include "avcodec_log.h"
#include "av_common.h"
#include "native_avdemuxer.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVDemuxer"};
}

using namespace OHOS::Media;

struct DemuxerObject : public OH_AVDemuxer {
    explicit DemuxerObject(const std::shared_ptr<AVDemuxer> &demuxer)
        : OH_AVDemuxer(AVMagic::AVCODEC_MAGIC_AVDEMUXER), demuxer_(demuxer) {}
    ~DemuxerObject() = default;

    const std::shared_ptr<AVDemuxer> demuxer_;
};

struct OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "Create demuxer failed because input source is nullptr!");

    struct AVSourceObject *sourceObj = reinterpret_cast<AVSourceObject *>(source);

    std::shared_ptr<AVDemuxer> demuxer = AVDemuxerFactory::CreateWithSource(*(sourceObj->source_));
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, nullptr, "New demuxer with fd by AVDemuxerFactory failed!");

    struct DemuxerObject *object = new(std::nothrow) DemuxerObject(demuxer);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "New demuxerObject failed when create demuxer!");

    return object;
}

OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL,
        "Destroy demuxer failed because input demuxer is nullptr!");

    delete demuxer;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_SelectSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL,
        "Add sourceTrack failed because input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(demuxer->magic_ == AVMagic::AVCODEC_MAGIC_AVDEMUXER, AV_ERR_INVALID_VAL, "magic error!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, AV_ERR_INVALID_VAL,
        "New DemuxerObject failed when add sourceTrack!");

    int32_t ret = demuxerObj->demuxer_->SelectSourceTrackByID(trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer_ SelectSourceTrackByID failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_UnselectSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL,
        "Remove sourceTrack failed because input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(demuxer->magic_ == AVMagic::AVCODEC_MAGIC_AVDEMUXER, AV_ERR_INVALID_VAL, "magic error!");
    
    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, AV_ERR_INVALID_VAL,
        "New DemuxerObject failed when remove sourceTrack!");

    int32_t ret = demuxerObj->demuxer_->UnselectSourceTrackByID(trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer_ UnselectSourceTrackByID failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_CopyNextSample(OH_AVDemuxer *demuxer, uint32_t *trackIndex,
                                         uint8_t *buffer, OH_AVCodecBufferAttr *bufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL,
        "Copy sample failed because input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(demuxer->magic_ == AVMagic::AVCODEC_MAGIC_AVDEMUXER, AV_ERR_INVALID_VAL, "magic error!");
    
    CHECK_AND_RETURN_RET_LOG(trackIndex != nullptr, AV_ERR_INVALID_VAL,
        "Copy sample failed because input trackIndex is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AV_ERR_INVALID_VAL,
        "Copy sample failed because input buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(bufferInfo != nullptr, AV_ERR_INVALID_VAL,
        "Copy sample failed because input attr is nullptr!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, AV_ERR_INVALID_VAL,
        "New DemuxerObject failed when copy sample!");

    struct AVCodecBufferInfo bufferInfoInner;
    enum AVCodecBufferFlag *flag = nullptr;
    *flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    int32_t ret = demuxerObj->demuxer_->CopyNextSample(*trackIndex, buffer, bufferInfoInner, *flag);
    bufferInfo->pts = bufferInfoInner.presentationTimeUs;
    bufferInfo->size = bufferInfoInner.size;
    bufferInfo->offset = bufferInfoInner.offset;
    bufferInfo->flags =static_cast<uint32_t>(*flag);
    CHECK_AND_RETURN_RET_LOG(ret == AV_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer_ CopyNextSample failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_SeekToTime(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "Seek failed because input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(demuxer->magic_ == AVMagic::AVCODEC_MAGIC_AVDEMUXER, AV_ERR_INVALID_VAL, "magic error!");

    CHECK_AND_RETURN_RET_LOG(mSeconds >= 0, AV_ERR_INVALID_VAL, "Seek failed because input mSeconds is negative!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, AV_ERR_INVALID_VAL,
        "New DemuxerObject failed when seek!");

    int32_t ret = demuxerObj->demuxer_->SeekToTime(mSeconds, static_cast<AVSeekMode>(mode));
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer_ SeekToTime failed!");

    return AV_ERR_OK;
}