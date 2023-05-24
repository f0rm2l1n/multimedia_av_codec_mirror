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

#include "videodec_ndk_sample.h"
#include <sys/time.h>
#include <arpa/inet.h>
#include <utility>
#include "openssl/sha.h"
#include "openssl/crypto.h"
#define SECOND_TO_MIRCO 1000 * 1000

using namespace OHOS;
using namespace OHOS::Media;
using namespace std;
namespace {
const string MIME_TYPE = "video_decoder.avc";
const string CODEC_NAME = "avdec_h264";
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
SHA512_CTX c;
unsigned char md[SHA512_DIGEST_LENGTH];

void clearIntqueue(std::queue<uint32_t> &q)
{
    std::queue<uint32_t> empty;
    swap(empty, q);
}

void clearBufferqueue(std::queue<OH_AVCodecBufferAttr> &q)
{
    std::queue<OH_AVCodecBufferAttr> empty;
    swap(empty, q);
}
} // namespace

VDecNdkSample::~VDecNdkSample() {}

void VdecError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    cout << "Error errorCode=" << errorCode << endl;
    VDecSignal *signal = static_cast<VDecSignal *>(userData);
    if (errorCode != AV_ERR_OK)
        signal->errCount_++;
}

void VdecFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    cout << "Format Changed" << endl;
}

void VdecInputDataReady(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, void *userData)
{
    VDecSignal *signal = static_cast<VDecSignal *>(userData);
    unique_lock<mutex> lock(signal->inMutex_);
    signal->inIdxQueue_.push(index);
    signal->inBufferQueue_.push(data);
    signal->inCond_.notify_all();
}

void VdecOutputDataReady(OH_AVCodec *codec, uint32_t index, OH_AVMemory *data, OH_AVCodecBufferAttr *attr,
                         void *userData)
{
    VDecSignal *signal = static_cast<VDecSignal *>(userData);
    unique_lock<mutex> lock(signal->outMutex_);
    signal->outIdxQueue_.push(index);
    signal->attrQueue_.push(*attr);
    signal->outBufferQueue_.push(data);
    signal->outCond_.notify_all();
}

bool VDecNdkSample::MdCompare(unsigned char *buffer, int len, const char *source[])
{
    bool result = true;
    for (int i = 0; i < len; i++) {
        char std[SHA512_DIGEST_LENGTH] = {0};
        sprintf(std, "%02X", buffer[i]);
        int re = strcmp(source[i], std);
        if (re != 0) {
            result = false;
            break;
        }
    }
    return result;
}

int64_t VDecNdkSample::GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = (int64_t)now.tv_sec * NANOS_IN_SECOND + now.tv_nsec;

    return nanoTime / NANOS_IN_MICRO;
}

int32_t VDecNdkSample::ConfigureVideoDecoder()
{
    OH_AVFormat *format = OH_AVFormat_Create();
    if (format == nullptr) {
        cout << "Fatal: Failed to create format" << endl;
        return AV_ERR_UNKNOWN;
    }

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    int ret = OH_VideoDecoder_Configure(vdec_, format);
    OH_AVFormat_Destroy(format);
    return ret;
}

void VDecNdkSample::RunVideoDec(OHNativeWindow *window, string codeName)
{
    if (SURFACE_OUTPUT) {
        if (window == nullptr) {
            return;
        }
    }

    int err = CreateVideoDecoder(codeName);
    if (err != AV_ERR_OK) {
        cout << "Failed to create video decoder" << endl;
        return;
    }

    err = SetVideoDecoderCallback();
    if (err != AV_ERR_OK) {
        cout << "Failed to setCallback" << endl;
        Release();
        return;
    }

    err = ConfigureVideoDecoder();
    if (err != AV_ERR_OK) {
        cout << "Failed to configure video decoder" << endl;
        Release();
        return;
    }

    if (SURFACE_OUTPUT) {
        err = OH_VideoDecoder_SetSurface(vdec_, window);
        if (err != AV_ERR_OK) {
            cout << "Failed to set output surface" << endl;
            Release();
            return;
        }
    }

    err = StartVideoDecoder();
    if (err != AV_ERR_OK) {
        cout << "Failed to start video decoder" << endl;
        Release();
        return;
    }
}

int32_t VDecNdkSample::SetVideoDecoderCallback()
{
    signal_ = new VDecSignal();
    if (signal_ == nullptr) {
        cout << "Failed to new VDecSignal" << endl;
        return AV_ERR_UNKNOWN;
    }

    cb_.onError = VdecError;
    cb_.onStreamChanged = VdecFormatChanged;
    cb_.onNeedInputData = VdecInputDataReady;
    cb_.onNeedOutputData = VdecOutputDataReady;
    return OH_VideoDecoder_SetCallback(vdec_, cb_, static_cast<void *>(signal_));
}

int32_t VDecNdkSample::ConfigureFormat(uint32_t width, uint32_t height, uint32_t frameRate)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    if (format == nullptr) {
        cout << "Fatal: Failed to create format" << endl;
        return AV_ERR_UNKNOWN;
    }

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), width);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), height);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), frameRate);

    int ret = OH_VideoDecoder_Configure(vdec_, format);
    OH_AVFormat_Destroy(format);
    return ret;
}

void VDecNdkSample::ReleaseInFile()
{
    if (inFile_ != nullptr) {
        if (inFile_->is_open()) {
            inFile_->close();
        }
        inFile_.reset();
        inFile_ = nullptr;
    }
}

void VDecNdkSample::StopInloop()
{
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->inMutex_);
        clearIntqueue(signal_->inIdxQueue_);
        signal_->inCond_.notify_all();
        lock.unlock();

        inputLoop_->join();
        inputLoop_.reset();
    }
}

int32_t VDecNdkSample::StartVideoDecoder()
{
    int ret = OH_VideoDecoder_Start(vdec_);
    if (ret != AV_ERR_OK) {
        cout << "Failed to start codec" << endl;
        return ret;
    }

    isRunning_.store(true);

    inFile_ = make_unique<ifstream>();
    if (inFile_ == nullptr) {
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        return AV_ERR_UNKNOWN;
    }
    inFile_->open(INP_DIR, ios::in | ios::binary);
    if (!inFile_->is_open()) {
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        inFile_->close();
        inFile_.reset();
        inFile_ = nullptr;
        return AV_ERR_UNKNOWN;
    }

    inputLoop_ = make_unique<thread>(&VDecNdkSample::InputFunc, this);
    if (inputLoop_ == nullptr) {
        cout << "Failed to create input loop" << endl;
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        ReleaseInFile();
        return AV_ERR_UNKNOWN;
    }

    outputLoop_ = make_unique<thread>(&VDecNdkSample::OutputFunc, this);

    if (outputLoop_ == nullptr) {
        cout << "Failed to create output loop" << endl;
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        ReleaseInFile();
        StopInloop();
        Release();
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VDecNdkSample::CreateVideoDecoder(string codeName)
{
    if (!codeName.empty()) {
        vdec_ = OH_VideoDecoder_CreateByName(codeName.c_str());
    } else {
        vdec_ = OH_VideoDecoder_CreateByMime(MIME_TYPE.c_str());
    }
    return vdec_ == nullptr ? AV_ERR_UNKNOWN : AV_ERR_OK;
}

int32_t VDecNdkSample::StartVideoDecoderNdkTest()
{
    isRunning_.store(true);
    inFile_ = make_unique<ifstream>();
    if (inFile_ == nullptr) {
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        return AV_ERR_UNKNOWN;
    }
    inFile_->open(INP_DIR, ios::in | ios::binary);
    if (!inFile_->is_open()) {
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        inFile_->close();
        inFile_.reset();
        inFile_ = nullptr;
        return AV_ERR_UNKNOWN;
    }
    inputLoop_ = make_unique<thread>(&VDecNdkSample::InputFuncTest, this);
    if (inputLoop_ == nullptr) {
        cout << "Failed to create input loop" << endl;
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        ReleaseInFile();
        return AV_ERR_UNKNOWN;
    }
    outputLoop_ = make_unique<thread>(&VDecNdkSample::OutputFuncTest, this);
    if (outputLoop_ == nullptr) {
        cout << "Failed to create output loop" << endl;
        isRunning_.store(false);
        (void)OH_VideoDecoder_Stop(vdec_);
        ReleaseInFile();
        StopInloop();
        Release();
        return AV_ERR_UNKNOWN;
    }
    int ret = OH_VideoDecoder_Start(vdec_);
    if (ret != AV_ERR_OK) {
        cout << "Failed to start codec" << endl;
        return ret;
    }
    return AV_ERR_OK;
}

void VDecNdkSample::WaitForEOS()
{
    outputLoop_->join();
}

void VDecNdkSample::InputFunc()
{
    errCount = 0;
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        uint32_t index;
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return signal_->inIdxQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }
        index = signal_->inIdxQueue_.front();
        auto buffer = signal_->inBufferQueue_.front();
        OH_AVCodecBufferAttr attr;
        if (!inFile_->eof()) {
            char ch[4] = {};
            (void)inFile_->read(ch, 4);
            if (inFile_->eof()) {
                attr.pts = 0;
                attr.size = 0;
                attr.offset = 0;
                attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
                (void)OH_VideoDecoder_PushInputData(vdec_, index, attr);
                cout << "OH_VideoDecoder_PushInputData    EOS" << endl;
                break;
            }
            uint32_t bufferSize =
                (uint32_t)(((ch[3] & 0xFF)) | ((ch[2] & 0xFF) << 8) | ((ch[1] & 0xFF) << 16) | (ch[0] & 0xFF << 24));
            char *fileBuffer = new char[bufferSize + 4];
            if (fileBuffer == nullptr) {
                cout << "Fatal: no memory" << endl;
                continue;
            }
            fileBuffer[0] = 0x00;
            fileBuffer[1] = 0x00;
            fileBuffer[2] = 0x00;
            fileBuffer[3] = 0x01;
            (void)inFile_->read(fileBuffer + 4, bufferSize);
            attr.pts = GetSystemTimeUs();
            attr.size = bufferSize + 4;
            attr.offset = 0;
            if (0x07 == (fileBuffer[4] & 0x1f)) {
                attr.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            } else {
                attr.flags = AVCODEC_BUFFER_FLAGS_NONE;
            }
            int32_t size = OH_AVMemory_GetSize(buffer);
            if (size < bufferSize) {
                delete[] fileBuffer;
                cout << "bufferSize is " << endl;
                continue;
            }
            if (memcpy_s(OH_AVMemory_GetAddr(buffer), bufferSize + 4, fileBuffer, bufferSize + 4) != EOK) {
                delete[] fileBuffer;
                cout << "Fatal: memcpy fail" << endl;
                continue;
            }
            int32_t result = OH_VideoDecoder_PushInputData(vdec_, index, attr);
            cout << "OH_VideoDecoder_PushInputData, code = " << result << "  index=" << index
                 << "  flags=" << attr.flags << " bufferSize=" << bufferSize << endl;
            if (result != 0) {
                errCount = errCount + 1;
                break;
            }
            delete[] fileBuffer;
            frameCount_ = frameCount_ + 1;
        }
        signal_->inIdxQueue_.pop();
        signal_->inBufferQueue_.pop();
    }
}

void VDecNdkSample::OutputFunc()
{
    SHA512_Init(&c);
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        OH_AVCodecBufferAttr attr;
        uint32_t index;
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return signal_->outIdxQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }
        index = signal_->outIdxQueue_.front();
        attr = signal_->attrQueue_.front();
        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            signal_->outIdxQueue_.pop();
            signal_->attrQueue_.pop();
            signal_->outBufferQueue_.pop();
            SHA512_Final(md, &c);
            OPENSSL_cleanse(&c, sizeof(c));
            bool result = MdCompare(md, SHA512_DIGEST_LENGTH, fileSourcesha256);
            cout << "dec finish " << INP_DIR << " MdCompare result:" << result << endl;
            if (AFTER_EOS_DESTORY_CODEC) {
                (void)Stop();
                Release();
            }
            break;
        }
        int64_t decTs = GetSystemTimeUs() - attr.pts;
        cout << "dec " << INP_DIR << " time:" << decTs << "  attr.flags:" << attr.flags << endl;
        if (!SURFACE_OUTPUT) {
            int size = attr.size;
            OH_AVMemory *buffer = signal_->outBufferQueue_.front();
            FILE *outFile;
            outFile = fopen(OUT_DIR, "a");
            if (outFile == nullptr) {
                cout << "dump data fail" << endl;
            } else {
                fwrite(OH_AVMemory_GetAddr(buffer), 1, size, outFile);
            }
            fclose(outFile);
            SHA512_Update(&c, OH_AVMemory_GetAddr(buffer), size);
            if (OH_VideoDecoder_FreeOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: ReleaseOutputBuffer fail" << endl;
                errCount = errCount + 1;
                continue;
            }
        } else if (IsRender()) {
            if (OH_VideoDecoder_RenderOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: RenderOutputBuffer fail" << endl;
                errCount = errCount + 1;
                continue;
            }
        } else {
            if (OH_VideoDecoder_FreeOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: ReleaseOutputBuffer fail" << endl;
                errCount = errCount + 1;
                continue;
            }
        }
        if (errCount > 0) {
            break;
        }
        signal_->outIdxQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outBufferQueue_.pop();
    }
}

void VDecNdkSample::InputFuncTest()
{
    frameCount_ = 0;
    errCount = 0;
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        if (REPEAT_START_FLUSH_BEFORE_EOS > 0) {
            REPEAT_START_FLUSH_BEFORE_EOS--;
            OH_VideoDecoder_Flush(vdec_);
            OH_VideoDecoder_Start(vdec_);
        }
        if (REPEAT_START_STOP_BEFORE_EOS > 0) {
            REPEAT_START_STOP_BEFORE_EOS--;
            OH_VideoDecoder_Stop(vdec_);
            OH_VideoDecoder_Start(vdec_);
        }
        uint32_t index;
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return signal_->inIdxQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }
        index = signal_->inIdxQueue_.front();
        auto buffer = signal_->inBufferQueue_.front();
        OH_AVCodecBufferAttr attr;
        if (!inFile_->eof()) {
            if (BEFORE_EOS_INPUT && frameCount_ > 10) {
                attr.pts = 0;
                attr.size = 0;
                attr.offset = 0;
                attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
                (void)OH_VideoDecoder_PushInputData(vdec_, index, attr);
                break;
            }
            if (BEFORE_EOS_INPUT_INPUT && frameCount_ > 10) {
                attr.pts = 0;
                attr.size = 0;
                attr.offset = 0;
                attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
                BEFORE_EOS_INPUT_INPUT = false;
            }
            char ch[4] = {};
            (void)inFile_->read(ch, 4);
            if (inFile_->eof()) {
                attr.pts = 0;
                attr.size = 0;
                attr.offset = 0;
                attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
                (void)OH_VideoDecoder_PushInputData(vdec_, index, attr);
                cout << "OH_VideoDecoder_PushInputData    EOS" << endl;
                break;
            }
            uint32_t bufferSize =
                (uint32_t)(((ch[3] & 0xFF)) | ((ch[2] & 0xFF) << 8) | ((ch[1] & 0xFF) << 16) | (ch[0] & 0xFF << 24));
            char *fileBuffer = new char[bufferSize + 4];
            if (fileBuffer == nullptr) {
                cout << "Fatal: no memory" << endl;
                continue;
            }
            fileBuffer[0] = 0x00;
            fileBuffer[1] = 0x00;
            fileBuffer[2] = 0x00;
            fileBuffer[3] = 0x01;
            (void)inFile_->read(fileBuffer + 4, bufferSize);
            attr.pts = GetSystemTimeUs();
            attr.size = bufferSize + 4;
            attr.offset = 0;
            if (0x07 == (fileBuffer[4] & 0x1f)) {
                attr.flags = AVCODEC_BUFFER_FLAGS_CODEC_DATA;
            } else {
                attr.flags = AVCODEC_BUFFER_FLAGS_NONE;
            }
            int32_t size = OH_AVMemory_GetSize(buffer);
            if (size < bufferSize) {
                delete[] fileBuffer;
                cout << "bufferSize is " << endl;
                continue;
            }
            if (memcpy_s(OH_AVMemory_GetAddr(buffer), bufferSize + 4, fileBuffer, bufferSize + 4) != EOK) {
                delete[] fileBuffer;
                cout << "Fatal: memcpy fail" << endl;
                continue;
            }
            int32_t result = OH_VideoDecoder_PushInputData(vdec_, index, attr);
            cout << "OH_VideoDecoder_PushInputData, code = " << result << "  index=" << index
                 << "  flags=" << attr.flags << " bufferSize=" << bufferSize << endl;
            delete[] fileBuffer;
            frameCount_ = frameCount_ + 1;
        }
        signal_->inIdxQueue_.pop();
        signal_->inBufferQueue_.pop();
    }
}

void VDecNdkSample::OutputFuncTest()
{
    SHA512_Init(&c);
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        OH_AVCodecBufferAttr attr;
        uint32_t index;
        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return signal_->outIdxQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }
        index = signal_->outIdxQueue_.front();
        attr = signal_->attrQueue_.front();
        if (attr.flags == AVCODEC_BUFFER_FLAGS_EOS) {
            signal_->outIdxQueue_.pop();
            signal_->attrQueue_.pop();
            signal_->outBufferQueue_.pop();
            SHA512_Final(md, &c);
            OPENSSL_cleanse(&c, sizeof(c));
            bool result = MdCompare(md, SHA512_DIGEST_LENGTH, fileSourcesha256);
            cout << "dec finish " << INP_DIR << " MdCompare result:" << result << endl;
            if (AFTER_EOS_DESTORY_CODEC) {
                (void)Stop();
                Release();
            }
            break;
        }
        int64_t decTs = GetSystemTimeUs() - attr.pts;
        cout << "dec " << INP_DIR << " time:" << decTs << "  attr.flags:" << attr.flags << endl;
        if (!SURFACE_OUTPUT) {
            int size = attr.size;
            OH_AVMemory *buffer = signal_->outBufferQueue_.front();
            FILE *outFile;
            outFile = fopen(OUT_DIR, "a");
            if (outFile == nullptr) {
                cout << "dump data fail" << endl;
            } else {
                fwrite(OH_AVMemory_GetAddr(buffer), 1, size, outFile);
            }
            fclose(outFile);
            SHA512_Update(&c, OH_AVMemory_GetAddr(buffer), size);
            if (OH_VideoDecoder_FreeOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: ReleaseOutputBuffer fail" << endl;
                continue;
            }
        } else if (IsRender()) {
            if (OH_VideoDecoder_RenderOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: RenderOutputBuffer fail" << endl;
                continue;
            }
        } else {
            if (OH_VideoDecoder_FreeOutputData(vdec_, index) != AV_ERR_OK) {
                cout << "Fatal: ReleaseOutputBuffer fail" << endl;
                continue;
            }
        }
        signal_->outIdxQueue_.pop();
        signal_->attrQueue_.pop();
        signal_->outBufferQueue_.pop();
    }
}

bool VDecNdkSample::IsRender()
{
    if (lastRenderedTimeUs_ == 0) {
        return true;
    }
    int64_t curTimeUs = GetSystemTimeUs();
    if (curTimeUs - lastRenderedTimeUs_ > SECOND_TO_MIRCO / DEFAULT_FRAME_RATE / 2) {
        lastRenderedTimeUs_ = curTimeUs;
        return true;
    }
    return false;
}

int32_t VDecNdkSample::EOS()
{
    OH_AVCodecBufferAttr attr;
    int32_t index = 0;
    unique_lock<mutex> lock(signal_->outMutex_);
    signal_->outCond_.wait(lock, [this]() { return signal_->outIdxQueue_.size() > 0; });
    index = signal_->outIdxQueue_.front();
    attr = signal_->attrQueue_.front();
    attr.pts = 0;
    attr.size = 0;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAGS_EOS;
    return OH_VideoDecoder_PushInputData(vdec_, index, attr);
}

int32_t VDecNdkSample::Flush()
{
    unique_lock<mutex> inLock(signal_->inMutex_);
    clearIntqueue(signal_->inIdxQueue_);
    signal_->inCond_.notify_all();
    inLock.unlock();
    unique_lock<mutex> outLock(signal_->outMutex_);
    clearIntqueue(signal_->outIdxQueue_);
    clearBufferqueue(signal_->attrQueue_);
    signal_->outCond_.notify_all();
    outLock.unlock();

    return OH_VideoDecoder_Flush(vdec_);
}

int32_t VDecNdkSample::Reset()
{
    isRunning_.store(false);
    StopInloop();
    StopOutloop();
    inBufferMap_.clear();
    outBufferMap_.clear();
    ReleaseInFile();
    return OH_VideoDecoder_Reset(vdec_);
}

int32_t VDecNdkSample::Release()
{
    if (signal_ != nullptr) {
        delete signal_;
        signal_ = nullptr;
    }
    inBufferMap_.clear();
    outBufferMap_.clear();
    return OH_VideoDecoder_Destroy(vdec_);
}

int32_t VDecNdkSample::Stop()
{
    StopInloop();
    clearIntqueue(signal_->outIdxQueue_);
    clearBufferqueue(signal_->attrQueue_);
    ReleaseInFile();
    return OH_VideoDecoder_Stop(vdec_);
}

int32_t VDecNdkSample::Start()
{
    return OH_VideoDecoder_Start(vdec_);
}

void VDecNdkSample::StopOutloop()
{
    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        unique_lock<mutex> lock(signal_->outMutex_);
        clearIntqueue(signal_->outIdxQueue_);
        clearBufferqueue(signal_->attrQueue_);
        signal_->outCond_.notify_all();
        lock.unlock();
    }
}

int32_t VDecNdkSample::SetParameter(OH_AVFormat *format)
{
    return OH_VideoDecoder_SetParameter(vdec_, format);
}