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

#include <vector>
#include <queue>
#include <mutex>
#include <gtest/gtest.h>
#include <iostream>
#include <unistd.h>
#include <atomic>
#include <fstream>
#include <queue>
#include <string>
#include <thread>
#include "avcodec_codec_name.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include "native_avcodec_base.h"
#include "native_avformat.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "native_avcodec_audiodecoder.h"
#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif


using namespace std;
using namespace testing::ext;
using namespace OHOS::Media;

namespace {
const string CODEC_MP3_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_MP3_NAME_KEY);
const string CODEC_FLAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_FLAC_NAME_KEY);
const string CODEC_AAC_NAME = std::string(AVCodecCodecName::AUDIO_DECODER_AAC_NAME_KEY);
constexpr uint32_t MAX_CHANNEL_COUNT = 2;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;
constexpr uint32_t DEFAULT_BITRATE = 134000;
constexpr uint32_t DEFAULT_BITS_PER_CODED_RATE = 16;
constexpr uint32_t FRAME_DURATION_US = 33000;
constexpr string_view inputFilePath = "/data/test441_2_noid3.mp3";
constexpr string_view outputFilePath = "/data/audioOut.pcm";
} // namespace

namespace OHOS {
namespace Media {
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

class BufferCallback : public AVCodecCallback {
public:
    explicit BufferCallback(ADecSignal *userData) : userData_(userData) {}
    virtual ~BufferCallback() = default;
    ADecSignal *userData_;
    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
};

static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    (void)errorCode;
    (void)userData;
    cout << "Error received, errorCode:" << errorCode << endl;
}

static void OnOutputFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    (void)codec;
    (void)format;
    (void)userData;
    cout << "OnOutputFormatChanged received" << endl;
}

static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
{
    (void)codec;
    ADecSignal *signal = static_cast<ADecSignal *>(userData);
    unique_lock<mutex> lock(signal->inMutex_);
    signal->inQueue_.push(index);
    signal->inBufferQueue_.push(data);
    signal->inCond_.notify_all();
}


static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                                    void *userData)
{
    (void)codec;
    ADecSignal *signal = static_cast<ADecSignal *>(userData);
    unique_lock<mutex> lock(signal->outMutex_);
    signal->outQueue_.push(index);
    signal->outBufferQueue_.push(data);
    if (attr) {
        signal->attrQueue_.push(*attr);
    } else {
        cout << "OnOutputBufferAvailable error, attr is nullptr!" << endl;
    }
    signal->outCond_.notify_all();
}

class AudioCodeCapiDecoderUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceFunc();
    void InputFunc();
    void OutputFunc();
protected:
    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::ifstream> testFile_;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    int32_t index_;

    struct OH_AVCodecAsyncCallback cb_;
    ADecSignal *signal_;
    OH_AVCodec *audioDec_;
    OH_AVFormat *format;
    bool isFirstFrame_ = true;
    int64_t timeStamp_ = 0;
    uint32_t frameCount_ = 0;
    FILE *inFile_ {nullptr};
    FILE *dumpFd_ {nullptr};

    AVFormatContext *fmpt_ctx;
    AVFrame *frame;
    AVPacket pkt;
};

void AudioCodeCapiDecoderUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void AudioCodeCapiDecoderUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AudioCodeCapiDecoderUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void AudioCodeCapiDecoderUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

void AudioCodeCapiDecoderUnitTest::InputFunc()
{
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return (signal_->inQueue_.size() > 0 || !isRunning_.load()); });

        if (!isRunning_.load()) {
            break;
        }

        uint32_t index = signal_->inQueue_.front();
        auto buffer = signal_->inBufferQueue_.front();
        int32_t ret = av_read_frame(fmpt_ctx, &pkt);
        if (ret < 0) {
            OH_AVCodecBufferAttr info;
            info.size = 0;
            info.offset = 0;
            info.pts = 0;
            info.flags = AVCODEC_BUFFER_FLAGS_EOS;
            av_packet_unref(&pkt);
            OH_AudioDecoder_PushInputData(audioDec_, index, info);
            signal_->inBufferQueue_.pop();
            signal_->inQueue_.pop();
            std::cout << "end buffer\n";
            break;
        }
        OH_AVCodecBufferAttr info;
        info.size = pkt.size;
        info.offset = 0;
        info.pts = pkt.pts;
        memcpy_s(OH_AVMemory_GetAddr(buffer), pkt.size, pkt.data, pkt.size);

        ret = AVCS_ERR_OK;
        if (isFirstFrame_) {
            info.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
            isFirstFrame_ = false;
        } else {
            info.flags = AVCODEC_BUFFER_FLAGS_NONE;
            ret = OH_AudioDecoder_PushInputData(audioDec_, index, info);
        }

        timeStamp_ += FRAME_DURATION_US;
        signal_->inQueue_.pop();
        signal_->inBufferQueue_.pop();

        frameCount_++;

        if (ret != AVCS_ERR_OK) {
            cout << "Fatal error, exit" << endl;
            break;
        }
    }
}

void AudioCodeCapiDecoderUnitTest::OutputFunc()
{
    std::ofstream pcmFile;
    pcmFile.open(outputFilePath.data(), std::ios::out | std::ios::binary);
    if (!pcmFile.is_open()) {
        std::cout << "open " << outputFilePath << " failed!" << std::endl;
    }

    while (true) {
        if (!isRunning_.load()) {
            cout << "stop, exit" << endl;
            break;
        }

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return (signal_->outQueue_.size() > 0 || !isRunning_.load()); });

        if (!isRunning_.load()) {
            cout << "wait to stop, exit" << endl;
            break;
        }

        uint32_t index = signal_->outQueue_.front();
        OH_AVCodecBufferAttr attr = signal_->attrQueue_.front();
        OH_AVMemory *data = signal_->outBufferQueue_.front();
        if (data != nullptr) {
            pcmFile.write(reinterpret_cast<char *>(OH_AVMemory_GetAddr(data)), attr.size);
        }

        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            cout << "decode eos" << endl;
            isRunning_.store(false);
        }
        signal_->outBufferQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outQueue_.pop();
        EXPECT_EQ(AV_ERR_OK, OH_AudioDecoder_FreeOutputData(audioDec_, index));
    }
    pcmFile.close();
}

int32_t AudioCodeCapiDecoderUnitTest::ProceFunc(void)
{
    int32_t ret = 0;
    ret = avformat_open_input(&fmpt_ctx, inputFilePath.data(), NULL, NULL);
    if (ret < 0) {
        std::cout << "open file failed" << ret << "\n";
        exit(1);
    }
    if (avformat_find_stream_info(fmpt_ctx, NULL) < 0) {
        std::cout << "get file stream failed"
                  << "\n";
        exit(1);
    }
    frame = av_frame_alloc();
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    audioDec_ = OH_AudioDecoder_CreateByName("avdec_mp3");
    signal_ = new ADecSignal();

    cb_ = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetCallback(audioDec_, cb_, signal_));

    format = OH_AVFormat_Create();
    return AVCS_ERR_OK;
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_CreateByName_01, TestSize.Level1)
{
    audioDec_ = OH_AudioDecoder_CreateByName("avdec_mp3");
}


HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_SetParameter_01, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);

    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
        while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }

    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }
    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_SetParameter_02, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_SetParameter(audioDec_, format));
}
HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_Configure_01, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_normalcase_01, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);

    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }
    
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }
    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_normalcase_02, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);
    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);
    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }
    
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Stop(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_normalcase_03, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);

    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }
    
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}


HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_normalcase_04, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);

    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_normalcase_05, TestSize.Level1)
{
    ProceFunc();
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    isRunning_.store(true);

    inputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::InputFunc, this);
    EXPECT_NE(nullptr, inputLoop_);
    outputLoop_ = make_unique<thread>(&AudioCodeCapiDecoderUnitTest::OutputFunc, this);
    EXPECT_NE(nullptr, outputLoop_);

    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
    while (isRunning_.load()) {
        sleep(1); // sleep 1s
    }
    
    isRunning_.store(false);
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.notify_all();
        lock.unlock();
        inputLoop_->join();
    }

    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.notify_all();
        lock.unlock();
        outputLoop_->join();
    }
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Flush(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
    EXPECT_EQ(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Destroy(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_abnormalcase_01, TestSize.Level1)
{
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(), MAX_CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(), DEFAULT_SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE.data(),
                                DEFAULT_BITS_PER_CODED_RATE);
    OH_AVFormat_SetLongValue(format, MediaDescriptionKey::MD_KEY_BITRATE.data(), DEFAULT_BITRATE);

    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_abnormalcase_02, TestSize.Level1)
{
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Start(audioDec_));
}

HWTEST_F(AudioCodeCapiDecoderUnitTest, audioDecoder_abnormalcase_03, TestSize.Level1)
{
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Configure(audioDec_, format));
    EXPECT_NE(OH_AVErrCode::AV_ERR_OK, OH_AudioDecoder_Reset(audioDec_));
}
}  // namespace Media
}  // namespace OHOS