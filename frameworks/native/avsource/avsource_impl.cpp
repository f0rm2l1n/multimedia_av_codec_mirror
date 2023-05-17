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
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "avcodec_common.h"
#include "i_avcodec_service.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "avsource_impl.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVSourceImpl"};
}

namespace OHOS {
namespace Media {
std::vector<std::string_view> setTrackFormatSupportedList = {
    AVSourceTrackFormat::VIDEO_BIT_STREAM_FORMAT,
};

std::shared_ptr<AVSource> AVSourceFactory::CreateWithURI(const std::string &uri)
{
    AVCodecTrace trace("AVSourceFactory::CreateWithURI");
    
    AVCODEC_LOGI("create source with uri: uri=%{private}s", uri.c_str());

    std::shared_ptr<AVSourceImpl> sourceImpl = std::make_shared<AVSourceImpl>();
    CHECK_AND_RETURN_RET_LOG(sourceImpl != nullptr, nullptr, "New AVSourceImpl failed when create source with uri");

    int32_t ret = sourceImpl->Init(uri);

    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVSourceImpl failed when create source with uri");

    return sourceImpl;
}

std::shared_ptr<AVSource> AVSourceFactory::CreateWithFD(int32_t fd, int64_t offset, int64_t size)
{
    AVCodecTrace trace("AVSourceFactory::CreateWithFD");

    AVCODEC_LOGI("create source with fd: fd=%{private}d, offset=%{public}" PRId64 ", size=%{public}" PRId64,
        fd, offset, size);

    CHECK_AND_RETURN_RET_LOG(fd > 2, nullptr,
        "Create source with uri failed because input fd is illegal, fd must be greater than 2!");
    CHECK_AND_RETURN_RET_LOG(size >= 0, nullptr, "Create source with uri failed because input size is negative");

    CHECK_AND_RETURN_RET_LOG((fcntl(fd, F_GETFL, 0) & O_RDWR) == O_RDWR, nullptr, "No permission to read and write fd");
    CHECK_AND_RETURN_RET_LOG(lseek(fd, 0, SEEK_CUR) != -1, nullptr, "The fd is not seekable");

    std::string uri = "fd://" + std::to_string(fd) + "?offset=" + \
        std::to_string(offset) + "&size=" + std::to_string(size);

    std::shared_ptr<AVSourceImpl> sourceImpl = std::make_shared<AVSourceImpl>();
    CHECK_AND_RETURN_RET_LOG(sourceImpl != nullptr, nullptr, "New AVSourceImpl failed when create source with uri");

    int32_t ret = sourceImpl->Init(uri);

    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVSourceImpl failed when create source with uri");

    return sourceImpl;
}

int32_t AVSourceImpl::Init(const std::string &uri)
{
    AVCodecTrace trace("AVSource::Init");
    
    sourceClient_ = AVCodecServiceFactory::GetInstance().CreateSourceService();
    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Create source service failed when init sourceImpl");
    
    int32_t ret = sourceClient_->Init(uri);
    if (ret == AVCS_ERR_OK) {
        ret = sourceClient_->GetTrackCount(trackCount_);
    }
    return ret;
}

AVSourceImpl::AVSourceImpl()
{
    AVCODEC_LOGI("init sourceImpl");
    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVSourceImpl::~AVSourceImpl()
{
    AVCODEC_LOGI("uninit sourceImpl for source %{public}s", sourceUri.c_str());
    if (sourceClient_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroySourceService(sourceClient_);
        sourceClient_ = nullptr;
    }

    for (auto track : tracks_) {
        track = nullptr;
    }
    tracks_.clear();
    
    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVSourceImpl::GetSourceAddr(uintptr_t &addr)
{
    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when get source addr!");
    
    return sourceClient_->GetSourceAddr(addr);
}

int32_t AVSourceImpl::GetTrackCount(uint32_t &trackCount)
{
    AVCodecTrace trace("AVSource::GetTrackCount");

    AVCODEC_LOGI("get track count by source");

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when get source track!");

    trackCount = trackCount_;
    return AVCS_ERR_OK;
}

std::shared_ptr<AVSourceTrack> AVSourceImpl::GetSourceTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("AVSource::GetSourceTrackByID");

    AVCODEC_LOGI("get source track from source: trackIndex=%{public}d", trackIndex);

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, nullptr, "source service died when load track!");

    bool isValid = TrackIndexIsValid(trackIndex);
    CHECK_AND_RETURN_RET_LOG(isValid, nullptr, "track index is invalid!");

    std::shared_ptr<AVSourceTrack> track = std::make_shared<AVSourceTrackImpl>(this, trackIndex);
    tracks_.emplace_back(track);
    return track;
}

int32_t AVSourceImpl::GetSourceFormat(Format &format)
{
    AVCodecTrace trace("AVSource::GetSourceFormat");

    AVCODEC_LOGI("get source format: format=%{public}s", format.Stringify().c_str());

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when get source format!");
    
    return sourceClient_->GetSourceFormat(format);
}

int32_t AVSourceImpl::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    AVCodecTrace trace("AVSource::GetTrackFormat");

    AVCODEC_LOGI("get source track format: trackIndex=%{public}d, format=%{public}s",
        trackIndex, format.Stringify().c_str());

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
                            "source service died when get track format!");

    bool isValid = TrackIndexIsValid(trackIndex);
    CHECK_AND_RETURN_RET_LOG(isValid, AVCS_ERR_INVALID_VAL, "track index is invalid!");

    return sourceClient_->GetTrackFormat(format, trackIndex);
}

int32_t AVSourceImpl::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    AVCodecTrace trace("AVSource::SetTrackFormat");

    AVCODEC_LOGI("set source track format: trackIndex=%{public}d, format=%{public}s",
        trackIndex, format.Stringify().c_str());

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when set format!");

    bool isValid = TrackIndexIsValid(trackIndex);
    CHECK_AND_RETURN_RET_LOG(isValid, AVCS_ERR_INVALID_VAL, "track index is invalid!");

    auto &formatMap = format.GetFormatMap();
    bool allKeySupported = true;
    for (auto pair : formatMap) {
        auto index = std::find_if(setTrackFormatSupportedList.begin(), setTrackFormatSupportedList.end(),
                                  [pair](std::string_view support) { return pair.first == support; });
        if (index == setTrackFormatSupportedList.end()) {
            AVCODEC_LOGE("key %{punlic}s is not supported to set!", pair.first.c_str());
            allKeySupported = false;
            break;
        }
    }

    CHECK_AND_RETURN_RET_LOG(allKeySupported, AVCS_ERR_INVALID_VAL,
        "Set track format failed because input format is invalid!");

    return sourceClient_->SetTrackFormat(format, trackIndex);
}

bool AVSourceImpl::TrackIndexIsValid(uint32_t trackIndex)
{
    return (trackIndex >= 0 && trackIndex < trackCount_);
}

AVSourceTrackImpl::AVSourceTrackImpl(AVSourceImpl *source, uint32_t trackIndex)
{
    AVCODEC_LOGI("init source track: trackIndex=%{public}d", trackIndex);
    
    trackIndex_ = trackIndex;
    sourceImpl_ = source;
    
    AVCODEC_LOGD("AVSourceTrackImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVSourceTrackImpl::~AVSourceTrackImpl()
{
    AVCODEC_LOGI("uninit sourceTrackImpl");
    if (sourceImpl_ != nullptr) {
        sourceImpl_ = nullptr;
    }
    AVCODEC_LOGD("AVSourceTrackImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVSourceTrackImpl::SetTrackFormat(const Format &format)
{
    AVCodecTrace trace("AVSourceTrack::SetTrackFormat");

    return sourceImpl_->SetTrackFormat(format, trackIndex_);
}

int32_t AVSourceTrackImpl::GetTrackFormat(Format &format)
{
    AVCodecTrace trace("AVSourceTrack::GetTrackFormat");

    return sourceImpl_->GetTrackFormat(format, trackIndex_);
}
} // namespace Media
} // namespace OHOS