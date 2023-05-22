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
#include "avcodec_dump_utils.h"
#include "avcodec_log.h"
#ifdef HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecXCollie"};
    constexpr uint32_t DUMP_XCOLLIE_INDEX = 0x01000000;
    constexpr uint8_t DUMP_OFFSET_16 = 16;
}

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
    std::string name = data != nullptr ? (char *)data : "";
    AVCODEC_LOGE("Task %{public}s timeout", name.c_str());
    static constexpr uint32_t threshold = 5; // >5 Restart service
    if (threadDeadlockCount_ >= threshold) {
        AVCODEC_LOGF("Process timeout, go exit.");
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
    AVCodecDumpControler dumpControler;
    uint32_t dumperIndex = 1;
    for (const auto &iter : dfxDumper_) {
        dumpControler.AddInfo(DUMP_XCOLLIE_INDEX + (dumperIndex << DUMP_OFFSET_16), "Timer_Name", iter.second.second);
        dumperIndex++;
    }
    dumpControler.GetDumpString(dumpString);
    if (fd != -1) {
        write(fd, dumpString.c_str(), dumpString.size());
        dumpString.clear();
    }
    return AVCS_ERR_OK;
}

uint64_t AVCodecXCollie::SetTimer(const std::string &name, bool recovery, uint32_t timeout)
{
#ifdef HICOLLIE_ENABLE
    auto func = [this](void *data) {
        this->TimerCallback(data);
    };

    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_NOOP;
    if (recovery) {
        flag |= HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    }
    uint64_t tempIndex = dumperIndex_++;
    dfxDumper_.emplace(tempIndex, std::pair<int32_t, std::string>(HiviewDFX::INVALID_ID, name));
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(
            name, timeout, func, (void *)dfxDumper_[tempIndex].second.c_str(), flag);
    if (id == HiviewDFX::INVALID_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dfxDumper_.find(id);
        if (it != dfxDumper_.end()) {
            dfxDumper_.erase(it);
        }
    }
    dfxDumper_[tempIndex].first = id;
    return tempIndex;
#else
    return -1;
#endif
}

uint64_t AVCodecXCollie::SetTimerByLog(const std::string &name, uint32_t timeout)
{
#ifdef HICOLLIE_ENABLE
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(name, timeout, nullptr, this, flag);
    uint64_t tempIndex = dumperIndex_;
    if (id != HiviewDFX::INVALID_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        dfxDumper_.emplace(dumperIndex_++, std::pair<int32_t, std::string>(id, name));
    }
    return tempIndex;
#else
    return -1;
#endif
}

void AVCodecXCollie::CancelTimer(uint64_t index)
{
#ifdef HICOLLIE_ENABLE
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = dfxDumper_.find(index);
    if (it == dfxDumper_.end()) {
        return;
    }
    int32_t id = it->second.first;
    dfxDumper_.erase(it);
    return HiviewDFX::XCollie::GetInstance().CancelTimer(id);
#else
    (void)id;
    return;
#endif
}
} // namespace Media
} // namespace OHOS