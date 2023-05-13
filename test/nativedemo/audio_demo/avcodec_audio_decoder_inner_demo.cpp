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

#include "avcodec_audio_decoder_inner_demo.h"
#include "avcodec_errors.h"
#include "avcodec_common.h"
#include "demo_log.h"
#include "media_description.h"
#include "securec.h"
#include <iostream>
#include <unistd.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
}

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::InnerAudioDemo;
using namespace std;
namespace {
constexpr uint32_t CHANNEL_COUNT = 2;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t BITS_RATE = 169000; // for mp3
constexpr uint32_t BITS_PER_CODED_RATE = 4;
constexpr string_view inputFilePath = "/data/audioIn.mp3";
constexpr string_view outputFilePath = "/data/audioOut.pcm";
} // namespace

void ADecInnerDemo::RunCase()
{
    DEMO_CHECK_AND_RETURN_LOG(CreateDec() == AVCS_ERR_OK, "Fatal: CreateDec fail");

    Format format;
    format.PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, CHANNEL_COUNT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, SAMPLE_RATE);
    format.PutLongValue(MediaDescriptionKey::MD_KEY_BITRATE, BITS_RATE);
    format.PutIntValue("bits_per_coded_sample", BITS_PER_CODED_RATE);
    DEMO_CHECK_AND_RETURN_LOG(Configure(format) == AVCS_ERR_OK, "Fatal: Configure fail");

    DEMO_CHECK_AND_RETURN_LOG(Start() == AVCS_ERR_OK, "Fatal: Start fail");
    while (isRunning_.load()) {
        sleep(1); // start run 1s
    }
    DEMO_CHECK_AND_RETURN_LOG(Stop() == AVCS_ERR_OK, "Fatal: Stop fail");
    DEMO_CHECK_AND_RETURN_LOG(Release() == AVCS_ERR_OK, "Fatal: Release fail");
}

int32_t ADecInnerDemo::CreateDec()
{
    audioDec_ = AudioDecoderFactory::CreateByName("OH.Media.Codec.MP3.FFMPEGMp3");
    DEMO_CHECK_AND_RETURN_RET_LOG(audioDec_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: CreateByName fail");

    signal_ = make_shared<ADecSignal>();

    cb_ = make_unique<ADecDemoCallback>(signal_);
    DEMO_CHECK_AND_RETURN_RET_LOG(cb_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");
    DEMO_CHECK_AND_RETURN_RET_LOG(audioDec_->SetCallback(cb_) == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                                  "Fatal: SetCallback fail");

    return AVCS_ERR_OK;
}

int32_t ADecInnerDemo::Configure(const Format &format)
{
    return audioDec_->Configure(format);
}

int32_t ADecInnerDemo::Start()
{
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&ADecInnerDemo::InputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(inputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    outputLoop_ = make_unique<thread>(&ADecInnerDemo::OutputFunc, this);
    DEMO_CHECK_AND_RETURN_RET_LOG(outputLoop_ != nullptr, AVCS_ERR_UNKNOWN, "Fatal: No memory");

    return audioDec_->Start();
}

int32_t ADecInnerDemo::Stop()
{
    isRunning_.store(false);

    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inQueue_.push(0);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
        inputLoop_.reset();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outQueue_.push(0);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
        outputLoop_.reset();
    }

    return audioDec_->Stop();
}

int32_t ADecInnerDemo::Flush()
{
    return audioDec_->Flush();
}

int32_t ADecInnerDemo::Reset()
{
    return audioDec_->Reset();
}

int32_t ADecInnerDemo::Release()
{
    return audioDec_->Release();
}

void ADecInnerDemo::InputFunc()
{
    AVFormatContext *fmpt_ctx;
    AVPacket pkt;
    if (avformat_open_input(&fmpt_ctx, inputFilePath.data(), NULL, NULL) < 0) {
        std::cout << "open file failed"
                  << "\n";
        return;
    }
    if (avformat_find_stream_info(fmpt_ctx, NULL) < 0) {
        std::cout << "get file stream failed"
                  << "\n";
        return;
    }
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        std::unique_lock<std::mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return signal_->inQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->inQueue_.front();
        std::shared_ptr<AVSharedMemory> buffer = audioDec_->GetInputBuffer(index);
        if (buffer == nullptr) {
            isRunning_.store(false);
            std::cout << "buffer is null:" << index << "\n";
            break;
        }
        int ret = av_read_frame(fmpt_ctx, &pkt);
        if (ret < 0) {
            AVCodecBufferInfo attr;
            AVCodecBufferFlag flag;
            attr.presentationTimeUs = 0;
            attr.size = 0;
            attr.offset = 0;
            flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS;
            av_packet_unref(&pkt);
            (void)audioDec_->QueueInputBuffer(index, attr, flag);
            signal_->inQueue_.pop();
            std::cout << "end buffer\n";
            break;
        }
        if (memcpy_s(buffer->GetBase(), buffer->GetSize(), pkt.data, pkt.size) != EOK) {
            cout << "Fatal: memcpy fail" << endl;
            break;
        }
        AVCodecBufferInfo attr;
        AVCodecBufferFlag flag;
        attr.presentationTimeUs = pkt.pts;
        attr.size = pkt.size;
        attr.offset = 0;
        flag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
        av_packet_unref(&pkt);
        frameCount_++;
        auto result = audioDec_->QueueInputBuffer(index, attr, flag);
        signal_->inQueue_.pop();
        if (result != AVCS_ERR_OK) {
            std::cout << "QueueInputBuffer error:\n";
            isRunning_ = false;
            break;
        }
    }
}

void ADecInnerDemo::OutputFunc()
{
    std::ofstream outputFile(outputFilePath.data(), std::ios::binary);
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return signal_->outQueue_.size() > 0; });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->outQueue_.front();
        auto buffer = audioDec_->GetOutputBuffer(index);
        if (buffer == nullptr) {
            cout << "get output buffer failed" << endl;
            isRunning_.store(false);
            break;
        }
        auto attr = signal_->infoQueue_.front();
        auto flag = signal_->flagQueue_.front();
        if (flag == AVCODEC_BUFFER_FLAG_EOS) {
            cout << "decode eos" << endl;
            isRunning_.store(false);
        }
        outputFile.write((char*)buffer->GetBase(), attr.size);
        if (audioDec_->ReleaseOutputBuffer(index) != AVCS_ERR_OK) {
            cout << "Fatal: ReleaseOutputBuffer fail" << endl;
            break;
        }

        signal_->outQueue_.pop();
        signal_->infoQueue_.pop();
        signal_->flagQueue_.pop();
    }
}

ADecDemoCallback::ADecDemoCallback(shared_ptr<ADecSignal> signal) : signal_(signal) {}

void ADecDemoCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    cout << "Error received, errorType:" << errorType << " errorCode:" << errorCode << endl;
}

void ADecDemoCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    cout << "OnOutputFormatChanged received" << endl;
}

void ADecDemoCallback::OnInputBufferAvailable(uint32_t index)
{
    cout << "OnInputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->inMutex_);
    signal_->inQueue_.push(index);
    signal_->inCond_.notify_all();
}

void ADecDemoCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    (void)info;
    (void)flag;
    cout << "OnOutputBufferAvailable received, index:" << index << endl;
    unique_lock<mutex> lock(signal_->outMutex_);
    signal_->outQueue_.push(index);
    signal_->infoQueue_.push(info);
    signal_->flagQueue_.push(flag);
    signal_->outCond_.notify_all();
}