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

#ifndef AVCODEC_DFX_H
#define AVCODEC_DFX_H

#include <string>
#include <refbase.h>
#include "nocopyable.h"
#include "hisysevent.h"

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecEvent : public NoCopyable {
public:
    AVCodecEvent() = default;
    ~AVCodecEvent() = default;
    bool CreateMsg(const char *format, ...) __attribute__((__format__(printf, 2, 3)));
    void StatisticEventWrite(std::string eventName, OHOS::HiviewDFX::HiSysEvent::EventType type,
        std::string module);
    void BehaviorEventWrite(std::string eventName, OHOS::HiviewDFX::HiSysEvent::EventType type,
        std::string module);
    void FaultEventWrite(std::string eventName, int32_t errorCode, OHOS::HiviewDFX::HiSysEvent::EventType type,
        std::string module);
private:
    std::string msg_;
};

__attribute__((visibility("default"))) void BehaviorEventWrite(std::string status, std::string moudle);
__attribute__((visibility("default"))) void FaultEventWrite(int32_t errorCode, std::string msg, std::string moudle);
__attribute__((visibility("default"))) void StatisticEventWrite(std::string msg, std::string moudle);

#define AVCODEC_SYNC_TRACE AVCodecTrace trace(std::string(__FUNCTION__));

class __attribute__((visibility("default"))) AVCodecTrace : public NoCopyable {
public:
    explicit AVCodecTrace(const std::string &funcName);
    static void TraceBegin(const std::string &funcName, int32_t taskId);
    static void TraceEnd(const std::string &funcName, int32_t taskId);
    static void CounterTrace(const std::string &varName, int32_t val);
    ~AVCodecTrace();
private:
    bool isSync_ = false;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_DFX_H