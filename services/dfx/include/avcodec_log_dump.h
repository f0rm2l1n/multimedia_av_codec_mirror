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

#ifndef AVCODEC_LOG_DUMP_H
#define AVCODEC_LOG_DUMP_H

#include <string>
#include <thread>
#include <mutex>

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecLogDump {
public:
    static AVCodecLogDump &GetInstance();
    void SaveLog(const char *fmt, ...);

private:
    AVCodecLogDump();
    ~AVCodecLogDump();
    void UpdateCheckEnable();
    int32_t fileCount_ = 0;
    int32_t lineCount_ = 0;
    std::unique_ptr<std::thread> thread_;
    void TaskProcessor();
    std::mutex mutex_;
    std::condition_variable cond_;
    std::string logString_;
    bool isDump_ = false;
    bool isExit_ = false;
    bool isEnable_ = false;
    bool isNewFile_ = true;
};

// AVCodec log dump macro interface
#ifdef  OHOS_MEDIA_AVCODEC_LOG_DUMP
#define AVCODEC_DUMP_LOG(fmt, args...)                                            \
    do {                                                                            \
        (void)OHOS::Media::AVCodecLogDump::GetInstance().SaveLog(                   \
            "{%s():%d} " fmt, __FUNCTION__, __LINE__, ##args);                      \
    } while (0);
#else
#define AVCODEC_DUMP_LOG(fmt, args...) ;
#endif
} // namespace Media
} // namespace OHOS

#endif // AVCODEC_LOG_DUMP_H