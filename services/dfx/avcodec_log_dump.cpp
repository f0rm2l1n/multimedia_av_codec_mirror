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

#include "avcodec_log_dump.h"
#include <fstream>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include "securec.h"
#include "avcodec_log.h"

namespace {
constexpr int32_t FILE_MAX = 100;
constexpr int32_t FILE_LINE_MAX = 50000;
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecLogDump"};
}

namespace OHOS {
namespace Media {
AVCodecLogDump &AVCodecLogDump::GetInstance()
{
    static AVCodecLogDump avcodecLogDump;
    return avcodecLogDump;
}

AVCodecLogDump::AVCodecLogDump()
{
    CHECK_AND_RETURN_LOG(thread_ == nullptr, "TaskProcessor is existed");
    thread_ = std::make_unique<std::thread>(&AVCodecLogDump::TaskProcessor, this);
}

AVCodecLogDump::~AVCodecLogDump()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isExit_ = true;
        cond_.notify_all();
    }
    if (thread_ != nullptr && thread_->joinable()) {
        thread_->join();
    }
}

static void AddNewLog(std::string &logStr)
{
    struct timeval time = {};
    (void)gettimeofday(&time, nullptr);
    int64_t second = time.tv_sec % 60;
    int64_t allMinute = time.tv_sec / 60;
    int64_t minute = allMinute % 60;
    int64_t hour = allMinute / 60 % 24;
    int64_t mSecond = time.tv_usec / 1000;

    logStr += std::to_string(hour);
    logStr += ":";
    logStr += std::to_string(minute);
    logStr += ":";
    logStr += std::to_string(second);
    logStr += ":";
    logStr += std::to_string(mSecond);
    logStr += " ";
    logStr += " pid:";
    logStr += std::to_string(getpid());
    logStr += " tid:";
    logStr += std::to_string(gettid());
    logStr += " ";
}

void AVCodecLogDump::SaveLog(const char *fmt, ...)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!isEnable_) {
        return;
    }
    std::string temp = "";
    std::string fmtStr = fmt;
    int32_t srcPos = 0;
    auto dtsPos = fmtStr.find("{public}", srcPos);
    const int32_t pubLen = 8;
    while (dtsPos != std::string::npos) {
        temp += fmtStr.substr(srcPos, dtsPos - srcPos);
        srcPos = static_cast<int32_t>(dtsPos) + pubLen;
        dtsPos = fmtStr.find("{public}", srcPos);
    }
    temp += fmtStr.substr(srcPos);

    va_list ap;
    va_start(ap, fmt);
    constexpr uint8_t maxLogLen = 255;
    char logBuf[maxLogLen];
    auto ret = vsnprintf_s(logBuf, maxLogLen, maxLogLen - 1, temp.c_str(), ap);
    va_end(ap);

    AddNewLog(logString_);
    if (ret < 0) {
        logString_ += "dump log error";
    } else {
        logString_ += logBuf;
    }
    logString_ += "\n";
    lineCount_++;
    if (lineCount_ >= FILE_LINE_MAX) {
        cond_.notify_all();
    }
}

void AVCodecLogDump::UpdateCheckEnable()
{
    std::string file = "/data/media/log/check.config";
    std::ofstream ofStream(file);
    if (!ofStream.is_open()) {
        isEnable_ = false;
        return;
    }
    ofStream.close();
    isEnable_ = true;
}

void AVCodecLogDump::TaskProcessor()
{
    pthread_setname_np(pthread_self(), "AVCodecLogDumpTask");
    (void)mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE);
    (void)mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE);
    while (true) {
        std::string temp;
        int32_t lineCount = 0;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (isExit_) {
                return;
            }
            static constexpr int32_t timeout = 60; // every 1 minute have a log
            cond_.wait_for(lock, std::chrono::seconds(timeout),
                [this] {
                    UpdateCheckEnable();
                    return isExit_ || isDump_ || lineCount_ >= FILE_LINE_MAX || !logString_.empty();
            });
            isDump_ = false;
            lineCount = lineCount_;
            lineCount_ = lineCount_ >= FILE_LINE_MAX ? 0 : lineCount_;
            swap(logString_, temp);
        }

        std::string file = "/data/media/avcodec/log/";
        file += std::to_string(getpid());
        file += "dump_log_avcodec.log";
        file += std::to_string(fileCount_);
        std::ofstream ofStream;
        if (isNewFile_) {
            ofStream.open(file, std::ios::out | std::ios::trunc);
        } else {
            ofStream.open(file, std::ios::out | std::ios::app);
        }
        if (!ofStream.is_open()) {
            continue;
        }
        isNewFile_ = false;
        if (lineCount >= FILE_LINE_MAX) {
            isNewFile_ = true;
            fileCount_++;
            fileCount_ = fileCount_ > FILE_MAX ? 0 : fileCount_;
        }
        ofStream.write(temp.c_str(), temp.size());
        ofStream.close();
    }
}
} // namespace Media
} // namespace OHOS