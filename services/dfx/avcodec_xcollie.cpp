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

#include "avcodec_xcollie.h"
#include <unistd.h>
#include "avcodec_errors.h"
#include "param_wrapper.h"
#ifdef HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Media {
AVCodecXCollie &AVCodecXCollie::GetInstance()
{
    static AVCodecXCollie instance;
    return instance;
}

void AVCodecXCollie::TimerCallback(void *data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    threadDeadlockCount_++;
    static constexpr uint32_t threshold = 5; // >5 Restart service
    if (threadDeadlockCount_ >= threshold) {
        _exit(-1);
    }
}

int32_t AVCodecXCollie::Dump(int32_t fd)
{
    if (dfxDumper_.empty()) {
        return AVCS_ERR_OK;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::string dumpString = "[AVCodec_XCollie]\n";
    for (const auto &iter : dfxDumper_) {
        dumpString += "WaitTask-----";
        dumpString += iter.second;
        dumpString += "-----\n";
    }
    if (fd != -1) {
        write(fd, dumpString.c_str(), dumpString.size());
        dumpString.clear();
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

int32_t AVCodecXCollie::SetTimer(const std::string &name, bool recovery, uint32_t timeout)
{
#ifdef HICOLLIE_ENABLE
    auto func = [this](void *data) {
        this->TimerCallback(data);
    };

    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_NOOP;
    if (recovery) {
        flag |= HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    }
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(name, timeout, func, this, flag);
    if (id != HiviewDFX::INVALID_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        dfxDumper_.emplace(id, name);
    }
    return id;
#else
    return -1;
#endif
}

int32_t AVCodecXCollie::SetTimerByLog(const std::string &name, uint32_t timeout)
{
#ifdef HICOLLIE_ENABLE
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(name, timeout, nullptr, this, flag);
    if (id != HiviewDFX::INVALID_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        dfxDumper_.emplace(id, name);
    }
    return id;
#else
    return -1;
#endif
}

void AVCodecXCollie::CancelTimer(int32_t id)
{
#ifdef HICOLLIE_ENABLE
    if (id != HiviewDFX::INVALID_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dfxDumper_.find(id);
        if (it == dfxDumper_.end()) {
            return;
        }

        dfxDumper_.erase(it);
        return HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    }
#else
    (void)id;
    return;
#endif
}
} // namespace Media
} // namespace OHOS