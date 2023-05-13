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

#include "avsource.h"
#include "native_avmagic.h"
#include "avcodec_errors.h"
#include "native_object.h"
#include "native_avformat.h"
#include "avcodec_log.h"
#include "native_avsource.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVSource"};
}

using namespace OHOS::Media;

struct AVSourceTrackObject : public OH_AVSourceTrack {
    explicit AVSourceTrackObject(const std::shared_ptr<AVSourceTrack> &sourceTrack)
        : OH_AVSourceTrack(AVMagic::AVCODEC_MAGIC_AVSOURCETRACK), sourceTrack_(sourceTrack) {}
    ~AVSourceTrackObject() = default;

    const std::shared_ptr<AVSourceTrack> sourceTrack_;
};

struct OH_AVSource *OH_AVSource_CreateWithURI(char *uri)
{
    CHECK_AND_RETURN_RET_LOG(uri != nullptr, nullptr, "Create source with uri failed because input uri is nullptr!");
    
    std::shared_ptr<AVSource> source = AVSourceFactory::CreateWithURI(uri);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "New source with uri failed by AVSourceFactory!");

    struct AVSourceObject *object = new(std::nothrow) AVSourceObject(source);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "New AVSourceObject failed when create source with uri!");
    
    return object;
}

struct OH_AVSource *OH_AVSource_CreateWithFD(int32_t fd, int64_t offset, int64_t size)
{
    // 0-err, 1-in, 2-out
    CHECK_AND_RETURN_RET_LOG(fd > 2, nullptr,
                           "Create source with uri failed because input fd is illegal, fd must be greater than 2!");
    CHECK_AND_RETURN_RET_LOG(size >= 0, nullptr, "Create source with uri failed because input size is negative");

    std::shared_ptr<AVSource> source = AVSourceFactory::CreateWithFD(fd, offset, size);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "New source with fd failed by AVSourceFactory!");

    struct AVSourceObject *object = new(std::nothrow) AVSourceObject(source);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "New AVSourceObject failed when create source with fd!");

    return object;
}

OH_AVErrCode OH_AVSource_Destroy(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, AV_ERR_INVALID_VAL,
                           "Destroy source failed because input source is nullptr!");

    delete source;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVSource_GetTrackCount(OH_AVSource *source, uint32_t *trackCount)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, AV_ERR_INVALID_VAL,
                           "Get track count failed because input source is nullptr!");
    CHECK_AND_RETURN_RET_LOG(source->magic_ == AVMagic::AVCODEC_MAGIC_AVSOURCE, AV_ERR_INVALID_VAL, "magic error!");

    CHECK_AND_RETURN_RET_LOG(trackCount != nullptr, AV_ERR_INVALID_VAL,
                           "Get track count failed because input trackCount is nullptr!");
    
    struct AVSourceObject *sourceObj = reinterpret_cast<AVSourceObject *>(source);
    CHECK_AND_RETURN_RET_LOG(sourceObj->source_ != nullptr, AV_ERR_INVALID_VAL,
                           "New AVSourceObject failed when get track count!");

    int32_t ret = sourceObj->source_->GetTrackCount(*trackCount);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "source_ GetTrackCount failed!");

    return AV_ERR_OK;
}

struct OH_AVSourceTrack *OH_AVSource_GetSourceTrackByID(OH_AVSource *source, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "Load track failed because input source is nullptr!");
    CHECK_AND_RETURN_RET_LOG(source->magic_ == AVMagic::AVCODEC_MAGIC_AVSOURCE, nullptr, "magic error!");

    struct AVSourceObject *sourceObj = reinterpret_cast<AVSourceObject *>(source);
    CHECK_AND_RETURN_RET_LOG(sourceObj->source_ != nullptr, nullptr, "New AVSourceObject failed when load track!");

    std::shared_ptr<AVSourceTrack> sourceTrack = sourceObj->source_->GetSourceTrackByID(trackIndex);
    CHECK_AND_RETURN_RET_LOG(sourceTrack != nullptr, nullptr, "source_ GetSourceTrackByID failed!");

    struct AVSourceTrackObject *object = new(std::nothrow) AVSourceTrackObject(sourceTrack);

    return object;
}

OH_AVFormat *OH_AVSource_GetSourceFormat(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "Get source format failed because input source is nullptr!");
    CHECK_AND_RETURN_RET_LOG(source->magic_ == AVMagic::AVCODEC_MAGIC_AVSOURCE, nullptr, "magic error!");

    struct AVSourceObject *sourceObj = reinterpret_cast<AVSourceObject *>(source);
    CHECK_AND_RETURN_RET_LOG(sourceObj->source_ != nullptr, nullptr,
                           "New AVSourceObject failed when get source format!");

    Format format;
    int32_t ret = sourceObj->source_->GetSourceFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "source_ GetSourceFormat failed!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = format;
    
    return avFormat;
}


OH_AVErrCode OH_AVSourceTrack_SetTrackFormat(OH_AVSourceTrack *sourceTrack, OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(sourceTrack != nullptr, AV_ERR_INVALID_VAL,
                           "Set format failed because input sourceTrack is nullptr!");
    CHECK_AND_RETURN_RET_LOG(sourceTrack->magic_ == AVMagic::AVCODEC_MAGIC_AVSOURCETRACK, AV_ERR_INVALID_VAL,
                           "magic error!");

    CHECK_AND_RETURN_RET_LOG(format != nullptr, AV_ERR_INVALID_VAL,
                           "Set format failed because input param is nullptr!");

    struct AVSourceTrackObject *sourceTrackObj = reinterpret_cast<AVSourceTrackObject *>(sourceTrack);
    CHECK_AND_RETURN_RET_LOG(sourceTrackObj->sourceTrack_ != nullptr, AV_ERR_INVALID_VAL,
                           "New AVSourceObject failed when set track format!");

    int32_t ret = sourceTrackObj->sourceTrack_->SetTrackFormat(format->format_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "sourceTrack SetTrackFormat failed!");

    return AV_ERR_OK;
}

OH_AVFormat *OH_AVSourceTrack_GetTrackFormat(OH_AVSourceTrack *sourceTrack)
{
    CHECK_AND_RETURN_RET_LOG(sourceTrack != nullptr, nullptr,
                           "Set format failed because input sourceTrack is nullptr!");
    CHECK_AND_RETURN_RET_LOG(sourceTrack->magic_ == AVMagic::AVCODEC_MAGIC_AVSOURCETRACK, nullptr, "magic error!");

    struct AVSourceTrackObject *sourceTrackObj = reinterpret_cast<AVSourceTrackObject *>(sourceTrack);
    CHECK_AND_RETURN_RET_LOG(sourceTrackObj->sourceTrack_ != nullptr, nullptr,
                           "New AVSourceObject failed when get track format!");
    
    Format format;
    int32_t ret = sourceTrackObj->sourceTrack_->GetTrackFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "sourceTrack GetTrackFormat failed!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = format;
    
    return avFormat;
}