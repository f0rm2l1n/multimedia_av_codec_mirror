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
#include "avcodec_client.h"
#include "avcodec_xcollie.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecClient"};
}

namespace OHOS {
namespace Media {
static AVCodecClient avCodecClientInstance;
IAVCodecService &AVCodecServiceFactory::GetInstance()
{
    return avCodecClientInstance;
}

AVCodecClient::AVCodecClient() noexcept
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecClient::~AVCodecClient()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool AVCodecClient::IsAlived()
{
    if (avCodecProxy_ == nullptr) {
        avCodecProxy_ = GetAVCodecProxy();
    }

    return avCodecProxy_ != nullptr;
}

#ifdef SUPPORT_MUXER
std::shared_ptr<IMuxerService> AVCodecClient::CreateMuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("avcodec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_MUXER, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "muxer proxy object is nullptr.");

    sptr<IStandardMuxerService> muxerProxy = iface_cast<IStandardMuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(muxerProxy != nullptr, nullptr, "muxer proxy is nullptr.");

    std::shared_ptr<MuxerClient> muxer = MuxerClient::Create(muxerProxy);
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, nullptr, "failed to create muxer client.");

    muxerClientList_.push_back(muxer);
    return muxer;
}

int32_t AVCodecClient::DestroyMuxerService(std::shared_ptr<IMuxerService> muxer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AVCS_ERR_NO_MEMORY, "input muxer is nullptr.");
    muxerClientList_.remove(muxer);
    return AVCS_ERR_OK;
}
#endif

sptr<IStandardAVCodecService> AVCodecClient::GetAVCodecProxy()
{
    AVCODEC_LOGD("enter");
    sptr<ISystemAbilityManager> samgr = nullptr;
    COLLIE_LISTEN(samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager(),
                  "AVCodecClient::GetAVCodecProxy");
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "system ability manager is nullptr.");

    sptr<IRemoteObject> object = nullptr;
    COLLIE_LISTEN(object = samgr->GetSystemAbility(OHOS::AV_CODEC_SERVICE_ID), "AVCodecClient::GetAVCodecProxy");
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "avcodec object is nullptr.");

    avCodecProxy_ = iface_cast<IStandardAVCodecService>(object);

    CHECK_AND_RETURN_RET_LOG(avCodecProxy_ != nullptr, nullptr, "avcodec proxy is nullptr.");

    pid_t pid = 0;
    deathRecipient_ = new (std::nothrow) AVCodecDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient_ != nullptr, nullptr, "failed to new AVCodecDeathRecipient.");

    deathRecipient_->SetNotifyCb(std::bind(&AVCodecClient::AVCodecServerDied, std::placeholders::_1));
    bool result = object->AddDeathRecipient(deathRecipient_);
    if (!result) {
        AVCODEC_LOGE("failed to add deathRecipient");
        return nullptr;
    }

    listenerStub_ = new (std::nothrow) AVCodecListenerStub();
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, nullptr, "failed to new AVCodecListenerStub");
    return avCodecProxy_;
}

void AVCodecClient::AVCodecServerDied(pid_t pid)
{
    AVCODEC_LOGE("av_codec server is died, pid:%{public}d!", pid);
    avCodecClientInstance.DoAVCodecServerDied();
}

void AVCodecClient::DoAVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (avCodecProxy_ != nullptr) {
        (void)avCodecProxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        avCodecProxy_ = nullptr;
    }
    listenerStub_ = nullptr;
    deathRecipient_ = nullptr;

#ifdef SUPPORT_MUXER
    for (auto &it : muxerClientList_) {
        auto muxer = std::static_pointer_cast<MuxerClient>(it);
        if (muxer != nullptr) {
            muxer->AVCodecServerDied();
        }
    }
#endif
}
} // namespace Media
} // namespace OHOS
