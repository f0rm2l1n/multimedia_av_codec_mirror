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

#ifndef AVCODEC_AUDIO_DECODER_DEMO_H
#define AVCODEC_AUDIO_DECODER_DEMO_H

#include <atomic>
#include <fstream>
#include <queue>
#include <string>
#include <thread>

#include "native_avcodec_audiodecoder.h"
#include "nocopyable.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace Media {
namespace AudioDemo {
class ADecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<uint32_t> inQueue_;
    std::queue<uint32_t> outQueue_;
    std::queue<OH_AVMemory *> inBufferQueue_;
    std::queue<OH_AVMemory *> outBufferQueue_;
    std::queue<OH_AVCodecBufferAttr> attrQueue_;
};

class ADecDemo : public NoCopyable {
public:
    ADecDemo();
    virtual ~ADecDemo();
    void RunCase();

private:
    int32_t CreateDec();
    int32_t Configure(OH_AVFormat *format);
    int32_t Start();
    int32_t Stop();
    int32_t Flush();
    int32_t Reset();
    int32_t Release();
    void InputFunc();
    void OutputFunc();

    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::ifstream> testFile_;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    OH_AVCodec *audioDec_;
    ADecSignal *signal_;
    struct OH_AVCodecAsyncCallback cb_;
    bool isFirstFrame_ = true;
    int64_t timeStamp_ = 0;
    uint32_t frameCount_ = 0;

    AVFormatContext *fmpt_ctx;
    AVFrame *frame;
    AVPacket pkt;
};
} // namespace AudioDemo
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_AUDIO_DECODER_DEMO_H
