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

#include <avcodec_dfx.h>
#include <unistd.h>
#include "securec.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "hitrace_meter.h"
#include "dump_usage.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecDFX"};
constexpr uint32_t MAX_STRING_SIZE = 256;
constexpr char HISYSEVENT_DOMAIN_AVCODEC[] = "AV_CODEC";
} // namespace

namespace OHOS {
namespace Media {
bool AVCodecEvent::CreateMsg(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char msg[MAX_STRING_SIZE] = {0};
    if (vsnprintf_s(msg, sizeof(msg), sizeof(msg) - 1, format, args) < 0) {
        AVCODEC_LOGE("failed to call vsnprintf_s");
        va_end(args);
        return false;
    }
    va_end(args);
    msg_ = msg;
    return true;
}

void AVCodecEvent::BehaviorEventWrite(const std::string& eventName,
                                      OHOS::HiviewDFX::HiSysEvent::EventType type,
                                      const std::string& module)
{
    HiSysEventWrite(HISYSEVENT_DOMAIN_AVCODEC, eventName, type, "MODULE", module, "STATE", msg_);
}

void AVCodecEvent::FaultEventWrite(const std::string& eventName,
                                   OHOS::HiviewDFX::HiSysEvent::EventType type,
                                   FaultType faultType,
                                   const std::string& module)
{
    std::string faultName;
    switch (faultType) {
        case FaultType::FAULT_TYPE_FREEZE:
            faultName = "Freeze";
            break;
        case FaultType::FAULT_TYPE_CRASH:
            faultName = "Crash";
            break;
        case FaultType::FAULT_TYPE_INNER_ERROR:
            faultName = "Inner error";
            break;
        default:
            AVCODEC_LOGE("Invalid fault type:%{public}d", faultType);
    }
    HiSysEventWrite(HISYSEVENT_DOMAIN_AVCODEC, eventName, type, "MODULE", module, "FAULTTYPE", faultName, "MSG", msg_);
}

void BehaviorEventWrite(const std::string& status, const std::string& module)
{
    AVCodecEvent event;
    if (event.CreateMsg("%s, current state is: %s", "state change", status.c_str())) {
        event.BehaviorEventWrite("AV_CODEC_SUB_ABILITY_STATE", OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                                 module);
    } else {
        AVCODEC_LOGW("Failed to call CreateMsg");
    }
}

void FaultEventWrite(FaultType faultType, const std::string& msg, const std::string& module)
{
    AVCodecEvent event;
    if (event.CreateMsg("%s", msg.c_str())) {
        event.FaultEventWrite("AV_CODEC_FAULT", OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, faultType, module);
    } else {
        AVCODEC_LOGW("Failed to call CreateMsg");
    }
}

void StatisticTimeMemoryEventWrite(uint32_t useTime, const std::string& module)
{
    OHOS::HiviewDFX::DumpUsage dumpUse;
    uint64_t useMemory = dumpUse.GetPss(getpid());
    HiSysEventWrite(HISYSEVENT_DOMAIN_AVCODEC, "AV_CODEC_SERVER_STATISTICS",
                    OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "MODULE", module.c_str(), "TIME", useTime,
                    "MEMORY", useMemory);
}

void StatisticEventWrite(uint32_t codecCount,
                         uint32_t muxerCount,
                         uint32_t sourceCount,
                         uint32_t demuxerCount,
                         uint32_t codeclistCount,
                         const std::string& module)
{
    HiSysEventWrite(HISYSEVENT_DOMAIN_AVCODEC, "AV_CODEC_SUB_ABILITY_COUNT",
                    OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "MODULE", module.c_str(), "CODEC_COUNT",
                    codecCount, "MUXER_COUNT", muxerCount, "SOURCE_COUNT", sourceCount, "DEMUXER_COUNT", demuxerCount,
                    "CODECLIST_COUNT", codeclistCount);
}

AVCodecTrace::AVCodecTrace(const std::string& funcName)
{
    StartTrace(HITRACE_TAG_ZMEDIA, funcName);
    isSync_ = true;
}

void AVCodecTrace::TraceBegin(const std::string& funcName, int32_t taskId)
{
    StartAsyncTrace(HITRACE_TAG_ZMEDIA, funcName, taskId);
}

void AVCodecTrace::TraceEnd(const std::string& funcName, int32_t taskId)
{
    FinishAsyncTrace(HITRACE_TAG_ZMEDIA, funcName, taskId);
}

void AVCodecTrace::CounterTrace(const std::string& varName, int32_t val)
{
    CountTrace(HITRACE_TAG_ZMEDIA, varName, val);
}

AVCodecTrace::~AVCodecTrace()
{
    if (isSync_) {
        FinishTrace(HITRACE_TAG_ZMEDIA);
    }
}
} // namespace Media
} // namespace OHOS
