/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "source_server.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "ipc_skeleton.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServer"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<ISourceService> SourceServer::Create()
{
    std::shared_ptr<SourceServer> server = std::make_shared<SourceServer>();
    CHECK_AND_RETURN_RET_LOG(server != nullptr, nullptr, "Source Service does not exist");
    return server;
}

SourceServer::SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    appUid_ = IPCSkeleton::GetCallingUid();
    appPid_ = IPCSkeleton::GetCallingPid();
}

SourceServer::~SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    sourceEngine_ = nullptr;
}

int32_t SourceServer::Init(const std::string &uri)
{
    sourceEngine_ = ISourceEngineFactory::CreateSourceEngine(appUid_, appPid_, uri);
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackCount(uint32_t &trackCount)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = sourceEngine_->GetTrackCount(trackCount);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

int32_t SourceServer::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = sourceEngine_->SetTrackFormat(format, trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = sourceEngine_->GetTrackFormat(format, trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetSourceFormat(Format &format)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = sourceEngine_->GetSourceFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

uint64_t SourceServer::GetSourceAddr()
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = sourceEngine_->GetSourceAddr();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS