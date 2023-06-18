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
namespace MediaAVCodec {
std::shared_ptr<AVSource> AVSourceFactory::CreateWithURI(const std::string &uri)
{
    AVCodecTrace trace("AVSourceFactory::CreateWithURI");
    
    AVCODEC_LOGI("create source with uri: uri=%{private}s", uri.c_str());

    std::shared_ptr<AVSourceImpl> sourceImpl = std::make_shared<AVSourceImpl>();
    CHECK_AND_RETURN_RET_LOG(sourceImpl != nullptr, nullptr, "New AVSourceImpl failed when create source with uri");

    int32_t ret = sourceImpl->InitWithURI(uri);

    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVSourceImpl failed when create source with uri");

    return sourceImpl;
}

std::shared_ptr<AVSource> AVSourceFactory::CreateWithFD(int32_t fd, int64_t offset, int64_t size)
{
    AVCodecTrace trace("AVSourceFactory::CreateWithFD");

    AVCODEC_LOGI("create source with fd: fd=%{private}d, offset=%{public}" PRId64 ", size=%{public}" PRId64,
        fd, offset, size);

    CHECK_AND_RETURN_RET_LOG(fd > STDERR_FILENO, nullptr,
        "Create source with uri failed because input fd is illegal, fd must be greater than 2!");
    CHECK_AND_RETURN_RET_LOG(offset >= 0, nullptr,
        "Create source with fd failed because input offset is negative");
    CHECK_AND_RETURN_RET_LOG(size > 0, nullptr,
        "Create source with fd failed because input size must be greater than zero");
    int32_t flag = fcntl(fd, F_GETFL, 0);
    CHECK_AND_RETURN_RET_LOG(flag >= 0, nullptr, "get fd status failed");
    CHECK_AND_RETURN_RET_LOG(
        (static_cast<uint32_t>(flag) & static_cast<uint32_t>(O_WRONLY)) != static_cast<uint32_t>(O_WRONLY),
        nullptr, "the fd not be permitted to read ");
    CHECK_AND_RETURN_RET_LOG(lseek(fd, 0, SEEK_CUR) != -1, nullptr, "The fd is not seekable");
    std::shared_ptr<AVSourceImpl> sourceImpl = std::make_shared<AVSourceImpl>();
    CHECK_AND_RETURN_RET_LOG(sourceImpl != nullptr, nullptr, "New AVSourceImpl failed when create source with fd");

    int32_t ret = sourceImpl->InitWithFD(fd, offset, size);

    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVSourceImpl failed when create source with fd");

    return sourceImpl;
}

int32_t AVSourceImpl::InitWithURI(const std::string &uri)
{
    AVCodecTrace trace("AVSource::InitWithURI");

    sourceClient_ = AVCodecServiceFactory::GetInstance().CreateSourceService();
    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Create source service failed when init sourceImpl with uri");

    int32_t ret = sourceClient_->InitWithURI(uri);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Call source service init failed when init sourceImpl with uri");

    ret = sourceClient_->GetTrackCount(trackCount_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Init track count failed when init sourceImpl with uri");

    return AVCS_ERR_OK;
}

int32_t AVSourceImpl::InitWithFD(int32_t fd, int64_t offset, int64_t size)
{
    AVCodecTrace trace("AVSource::InitWithFD");

    sourceClient_ = AVCodecServiceFactory::GetInstance().CreateSourceService();
    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Create source service failed when init sourceImpl with fd");

    int32_t ret = sourceClient_->InitWithFD(fd, offset, size);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Call source service init failed when init sourceImpl with fd");

    ret = sourceClient_->GetTrackCount(trackCount_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,  AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED,
        "Init track count failed when init sourceImpl with fd");

    return AVCS_ERR_OK;
}

AVSourceImpl::AVSourceImpl()
{
    AVCODEC_LOGI("init sourceImpl");
    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVSourceImpl::~AVSourceImpl()
{
    AVCODEC_LOGI("uninit sourceImpl for source %{private}s", sourceUri.c_str());
    if (sourceClient_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroySourceService(sourceClient_);
        sourceClient_ = nullptr;
    }

    AVCODEC_LOGD("AVSourceImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVSourceImpl::GetSourceAddr(uintptr_t &addr)
{
    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when get source addr!");

    return sourceClient_->GetSourceAddr(addr);
}

int32_t AVSourceImpl::GetSourceFormat(Format &format)
{
    AVCodecTrace trace("AVSource::GetSourceFormat");

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "source service died when get source format!");
    
    return sourceClient_->GetSourceFormat(format);
}

int32_t AVSourceImpl::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    AVCodecTrace trace("AVSource::GetTrackFormat");

    AVCODEC_LOGI("get track format: trackIndex=%{public}u", trackIndex);

    CHECK_AND_RETURN_RET_LOG(sourceClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
                             "source service died when get track format!");

    bool isValid = (trackIndex < trackCount_);
    CHECK_AND_RETURN_RET_LOG(isValid, AVCS_ERR_INVALID_VAL, "track index is invalid!");

    return sourceClient_->GetTrackFormat(format, trackIndex);
}
} // namespace MediaAVCodec
} // namespace OHOS