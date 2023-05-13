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
#include <iostream>
#include <set>
#include <thread>
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "codec_utils.h"
#include "securec.h"
#include "utils.h"
#include "fcodec.h"
namespace OHOS {
namespace Media {
namespace Codec {
static const uint32_t INDEX_INPUT = 0;
static const uint32_t INDEX_OUTPUT = 1;
static const int32_t DEFAULT_IN_BUFFER_CNT = 8;
static const int32_t DEFAULT_OUT_BUFFER_CNT = 8;
static const int32_t DEFAULT_MIN_BUFFER_CNT = 1;
static const uint32_t VIDEO_PIX_DEPTH_YUV = 3;
static const uint32_t VIDEO_PIX_DEPTH_RGBA = 4;
static const int32_t VIDEO_ALIGN_SIZE = 16;  // 16字节对齐
static const int32_t VIDEO_MAX_SIZE = 15360; // 16K的宽
static const int32_t DEFAULT_VIDEO_WIDTH = 1920;
static const int32_t DEFAULT_VIDEO_HEIGHT = 1080;
static const uint32_t DEFAULT_TRY_DECODE_TIME = 10;
static const struct {
    const char *codecName;
    const char *mimeType;
    const char *ffmpegCodec;
    const bool isEncoder;
} SupportCodec[] = {
    {"video_decoder.avc", "video/avc", "h264", false},
};
static const size_t numSupportCodec = sizeof(SupportCodec) / sizeof(SupportCodec[0]);
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FCodec"};
}
FCodec::FCodec(const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_LOG(!name.empty(), "Create codec failed:  empty name");
    std::string fcodecName;
    for (size_t i = 0; i < numSupportCodec; ++i) {
        if (SupportCodec[i].codecName == name) {
            fcodecName = SupportCodec[i].ffmpegCodec;
            break;
        }
    }
    CHECK_AND_RETURN_LOG(!fcodecName.empty(), "Create codec failed: not support name: %{public}s", name.c_str());
    int32_t ret = Init(fcodecName);
    format_.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_NAME, name);
    CHECK_AND_RETURN_LOG(ret == AVCS_ERR_OK, "Create codec failed: init codec error");
}

FCodec::FCodec(bool isEncoder, const std::string &mime)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_LOG(!mime.empty(), "Create codec failed:  empty mime");
    std::string fcodecName;
    for (size_t i = 0; i < numSupportCodec; ++i) {
        if (SupportCodec[i].mimeType == mime && SupportCodec[i].isEncoder == isEncoder) {
            fcodecName = SupportCodec[i].ffmpegCodec;
            break;
        }
    }
    CHECK_AND_RETURN_LOG(!fcodecName.empty(), "Create codec failed: not support mime: %{public}s", mime.c_str());
    int32_t ret = Init(fcodecName);
    CHECK_AND_RETURN_LOG(ret == AVCS_ERR_OK, "Create codec failed: init codec error");
}

FCodec::~FCodec()
{
    callback_ = nullptr;
    CodecSurface_ = nullptr;
    Release();
    state_ = State::Uninitialized;
}

int32_t FCodec::Init(const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    avCodec_ =
        std::shared_ptr<AVCodec>(const_cast<AVCodec *>(avcodec_find_decoder_by_name(name.c_str())), [](void *ptr) {});
    CHECK_AND_RETURN_RET_LOG(avCodec_ != nullptr, AVCS_ERR_INVALID_VAL,
                             "Init codec failed:  cannot find codec with name %{public}s", name.c_str());
    codecName_ = name;
    sendTask_ = std::make_shared<TaskThread>("sendFrame");
    sendTask_->RegisterHandler([this] { SendFrame(); });
    receiveTask_ = std::make_shared<TaskThread>("ReceiveFrame");
    receiveTask_->RegisterHandler([this] { ReceiveFrame(); });
    state_ = State::Initialized;
    AVCODEC_LOGI("Init codec successful,  state: Uninitialized -> Initialized");
    return AVCS_ERR_OK;
}

int32_t FCodec::ConfigureDefault()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((state_ == State::Initialized), AVCS_ERR_INVALID_STATE,
                             "Configure codec failed:  not in Initialized state");
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_VIDEO_WIDTH);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_VIDEO_HEIGHT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_MAX_OUTPUT_BUFFER_COUNT, DEFAULT_OUT_BUFFER_CNT);
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_BUFFER_COUNT, DEFAULT_IN_BUFFER_CNT);
    avCodecContext_ = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(avCodec_.get()), [](AVCodecContext *p) {
        if (p != nullptr) {
            if (p->extradata) {
                av_free(p->extradata);
                p->extradata = nullptr;
            }
            avcodec_free_context(&p);
        }
    });
    CHECK_AND_RETURN_RET_LOG(avCodecContext_ != nullptr, AVCS_ERR_INVALID_OPERATION,
                             "Configure codec failed: Allocate context error");
    avCodecContext_->codec_type = AVMEDIA_TYPE_VIDEO;
    return AVCS_ERR_OK;
}

void FCodec::ConfigureBuffer(const Format &format, const std::string_view &formatKey, int32_t minVal, int32_t maxVal)
{
    if (format.GetValueType(formatKey) == FORMAT_TYPE_INT32) {
        int32_t val32 = 0;
        if (format.GetIntValue(formatKey, val32) && val32 > minVal && val32 < maxVal) {
            format_.PutIntValue(formatKey, val32);
        } else {
            AVCODEC_LOGW(
                "Set parameter failed: %{public}.*s, which minimum threshold=%{public}d, maximum threshold=%{public}d",
                formatKey.size(), formatKey.data(), minVal, maxVal);
        }
    } else if (format.GetValueType(formatKey) == FORMAT_TYPE_INT64) {
        int64_t val64 = 0;
        if (format.GetLongValue(formatKey, val64) && val64 > minVal) {
            format_.PutLongValue(formatKey, val64);
        } else {
            AVCODEC_LOGW("Set parameter failed: %{public}.*s", formatKey.size(), formatKey.data());
        }
    } else {
        AVCODEC_LOGW("Unsupport type for parameter: %{public}.*s", formatKey.size(), formatKey.data());
    }
}

void FCodec::ConfigureSufrace(const Format &format, const std::string_view &formatKey, uint32_t FORMAT_TYPE)
{
    uint8_t *addr = nullptr;
    size_t size = 0;
    int32_t val = 0;
    if (formatKey == MediaDescriptionKey::MD_KEY_CODEC_CONFIG && FORMAT_TYPE == FORMAT_TYPE_ADDR) {
        if (format.GetBuffer(formatKey, &addr, size) && size > 0) {
            auto allocSize = AlignUp(size + AV_INPUT_BUFFER_PADDING_SIZE, VIDEO_ALIGN_SIZE);
            avCodecContext_->extradata = static_cast<uint8_t *>(av_mallocz(allocSize));
            (void)memcpy_s(avCodecContext_->extradata, allocSize, addr, size);
            avCodecContext_->extradata_size = size;
            format_.PutBuffer(formatKey, avCodecContext_->extradata, avCodecContext_->extradata_size);
        }
    } else if (formatKey == MediaDescriptionKey::MD_KEY_PIXEL_FORMAT && FORMAT_TYPE == FORMAT_TYPE_INT32) {
        if (format.GetIntValue(formatKey, val)) {
            VideoPixelFormat vpf = static_cast<VideoPixelFormat>(val);
            if (vpf == VideoPixelFormat::RGBA || vpf == VideoPixelFormat::BGRA) {
                format_.PutIntValue(formatKey, val);
            }
        }
    } else if (formatKey == MediaDescriptionKey::MD_KEY_ROTATION_ANGLE && FORMAT_TYPE == FORMAT_TYPE_INT32) {
        if (format.GetIntValue(formatKey, val)) {
            GraphicTransformType sr = static_cast<GraphicTransformType>(val);
            if (sr == GraphicTransformType::GRAPHIC_ROTATE_NONE || sr == GraphicTransformType::GRAPHIC_ROTATE_90 ||
                sr == GraphicTransformType::GRAPHIC_ROTATE_180 || sr == GraphicTransformType::GRAPHIC_ROTATE_270) {
                format_.PutIntValue(formatKey, val);
            }
        }
    } else if (formatKey == MediaDescriptionKey::MD_KEY_SCALE_TYPE && FORMAT_TYPE == FORMAT_TYPE_INT32) {
        if (format.GetIntValue(formatKey, val)) {
            ScalingMode scaleMode = static_cast<ScalingMode>(val);
            if (scaleMode == ScalingMode::SCALING_MODE_SCALE_TO_WINDOW ||
                scaleMode == ScalingMode::SCALING_MODE_SCALE_CROP) {
                format_.PutIntValue(formatKey, val);
            }
        }
    } else {
        AVCODEC_LOGW("Set parameter failed: %{public}.*s, please check your value", formatKey.size(), formatKey.data());
    }
}

int32_t FCodec::Configure(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((state_ == State::Initialized), AVCS_ERR_INVALID_STATE,
                             "Configure codec failed:  not in Initialized state");
    CHECK_AND_RETURN_RET_LOG((ConfigureDefault() == AVCS_ERR_OK), AVCS_ERR_UNKNOWN,
                             "Configure codec failed:  not in Initialized state");
    for (auto &it : format.GetFormatMap()) {
        if (it.first == MediaDescriptionKey::MD_KEY_MAX_OUTPUT_BUFFER_COUNT ||
            it.first == MediaDescriptionKey::MD_KEY_MAX_INPUT_BUFFER_COUNT) {
            ConfigureBuffer(format, it.first, DEFAULT_MIN_BUFFER_CNT);
        } else if (it.first == MediaDescriptionKey::MD_KEY_WIDTH || it.first == MediaDescriptionKey::MD_KEY_HEIGHT) {
            ConfigureBuffer(format, it.first, 0, VIDEO_MAX_SIZE);
        } else if (it.first == MediaDescriptionKey::MD_KEY_BITRATE) {
            ConfigureBuffer(format, MediaDescriptionKey::MD_KEY_BITRATE);
        } else if (it.first == MediaDescriptionKey::MD_KEY_CODEC_CONFIG ||
                   it.first == MediaDescriptionKey::MD_KEY_PIXEL_FORMAT ||
                   it.first == MediaDescriptionKey::MD_KEY_ROTATION_ANGLE ||
                   it.first == MediaDescriptionKey::MD_KEY_SCALE_TYPE) {
            ConfigureSufrace(format, it.first, it.second.type);
        } else {
            AVCODEC_LOGW("Set parameter failed: %{public}.*s, unsupport name", it.first.size(), it.first.data());
        }
    }
    format.GetLongValue(MediaDescriptionKey::MD_KEY_BITRATE, avCodecContext_->bit_rate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width_);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height_);
    avCodecContext_->width = width_;
    avCodecContext_->height = height_;
    state_ = State::Configured;
    AVCODEC_LOGI("Configured codec successful: state: Initialized -> Configured");
    return AVCS_ERR_OK;
}

bool FCodec::IsActive() const
{
    return state_ == State::Running || state_ == State::Flushed;
}

void FCodec::ResetContext(bool isFlush)
{
    if (avCodecContext_ == nullptr) {
        return;
    }
    if (avCodecContext_->extradata) {
        av_free(avCodecContext_->extradata);
        avCodecContext_->extradata = nullptr;
    }
    avCodecContext_->coded_width = 0;
    avCodecContext_->coded_height = 0;
    avCodecContext_->extradata_size = 0;
    if (!isFlush) {
        avCodecContext_->width = 0;
        avCodecContext_->height = 0;
        avCodecContext_->get_buffer2 = nullptr;
    }
}

int32_t FCodec::Start()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Start codec failed: callback is null");
    CHECK_AND_RETURN_RET_LOG((state_ == State::Configured || state_ == State::Flushed), AVCS_ERR_INVALID_STATE,
                             "Start codec failed: not in Configured or Flushed state");
    if (state_ == State::Flushed) {
        for (uint32_t i = 0; i < buffers_[INDEX_INPUT].size(); i++) {
            callback_->OnInputBufferAvailable(i);
        }
        state_ = State::Running;
        receiveTask_->Start();
        sendTask_->Start();
        AVCODEC_LOGI("Codec starts successful, state: Flushed -> Running");
        return AVCS_ERR_OK;
    }
    cachedFrame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame *p) { av_frame_free(&p); });
    avPacket_ = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket *p) { av_packet_free(&p); });
    CHECK_AND_RETURN_RET_LOG((cachedFrame_ != nullptr && avPacket_ != nullptr), AVCS_ERR_UNKNOWN,
                             "Start codec failed: cannot allocate frame or packet");
    for (int32_t i = 0; i < AV_NUM_DATA_POINTERS; i++) {
        scaleData_[i] = nullptr;
        scaleLineSize_[i] = 0;
    }
    isConverted_ = false;
    std::unique_lock<std::mutex> sLock(syncMutex_);
    CHECK_AND_RETURN_RET_LOG(avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr) == 0, AVCS_ERR_UNKNOWN,
                             "Start codec failed: cannot open avcodec");
    sLock.unlock();
    int32_t ret = AllocateBuffers();
    if (ret != AVCS_ERR_OK) {
        std::unique_lock<std::mutex> sLock(syncMutex_);
        avcodec_close(avCodecContext_.get());
        AVCODEC_LOGE("Start codec failed: cannot allocate buffers");
        return ret;
    }
    state_ = State::Running;
    receiveTask_->Start();
    sendTask_->Start();
    if (CodecSurface_) {
        renderTask_->Start();
    }
    AVCODEC_LOGI("Codec starts successful, state: Configured -> Running");
    return AVCS_ERR_OK;
}

int32_t FCodec::Stop()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(state_ != State::Configured, AVCS_ERR_OK, "Stop codec successful, state: Configured");
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), AVCS_ERR_INVALID_STATE,
                             "Stop codec failed: not in running or Eos state");
    AVCODEC_LOGI("Stopping codec starts");
    state_ = State::Stopping;
    if (sendTask_ != nullptr) {
        sendTask_->Stop();
    }
    if (receiveTask_ != nullptr) {
        receiveTask_->Stop();
    }
    if (CodecSurface_ && renderTask_ != nullptr) {
        renderTask_->Stop();
    }
    AVCODEC_LOGI("Stopp codec loops");
    std::unique_lock<std::mutex> sLock(syncMutex_);
    avcodec_close(avCodecContext_.get());
    if (avCodecContext_->extradata) {
        av_free(avCodecContext_->extradata);
        avCodecContext_->extradata = nullptr;
    }
    avCodecContext_->extradata_size = 0;
    sLock.unlock();
    AVCODEC_LOGI("Stopp ffmpeg codec");
    CHECK_AND_RETURN_RET_LOG(ReleaseBuffers() == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                             "Stop codec failed: cannot release buffer");
    state_ = State::Configured;
    AVCODEC_LOGI("Stop codec successful, state: Configured");
    return AVCS_ERR_OK;
}

int32_t FCodec::Flush()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), AVCS_ERR_INVALID_STATE,
                             "Flush codec failed: not in running or Eos state");
    state_ = State::Flushing;
    sendTask_->Pause();
    receiveTask_->Pause();
    if (CodecSurface_) {
        renderTask_->Pause();
    }
    std::unique_lock<std::mutex> sLock(syncMutex_);
    avcodec_flush_buffers(avCodecContext_.get());
    ResetContext(true);
    sLock.unlock();
    CHECK_AND_RETURN_RET_LOG(ReleaseBuffers(true) == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                             "Flush codec failed: cannot release buffer");
    std::unique_lock<std::mutex> oLock(outputMutex_);
    for (uint32_t i = 0; i < buffers_[INDEX_OUTPUT].size(); i++) {
        buffers_[INDEX_OUTPUT][i]->owner_ = AVBuffer::OWNED_BY_CODEC;
        codecAvailBuffers_.emplace_back(i);
    }
    oLock.unlock();
    std::unique_lock<std::shared_mutex> iLock(inputMutex_);
    for (uint32_t i = 0; i < buffers_[INDEX_INPUT].size(); i++) {
        buffers_[INDEX_INPUT][i]->owner_ = AVBuffer::OWNED_BY_USER;
    }
    iLock.unlock();
    state_ = State::Flushed;
    AVCODEC_LOGI("Flush codec successful, state: Flushed");
    return AVCS_ERR_OK;
}

int32_t FCodec::Reset()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(Release() == AVCS_ERR_OK, AVCS_ERR_UNKNOWN, "Reset codec failed: cannot release codec");
    int32_t ret = Init(codecName_);
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("Reset codec failed: cannot init codec");
    }
    AVCODEC_LOGI("Reset codec successful, state: Initialized");
    return ret;
}

int32_t FCodec::Release()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(state_ != State::Uninitialized, AVCS_ERR_OK, "Release codec successful");
    state_ = State::Releasing;
    if (sendTask_ != nullptr) {
        sendTask_->Stop();
    }
    if (receiveTask_ != nullptr) {
        receiveTask_->Stop();
    }
    if (CodecSurface_ && renderTask_ != nullptr) {
        renderTask_->Stop();
    }
    std::unique_lock<std::mutex> sLock(syncMutex_);
    avcodec_close(avCodecContext_.get());
    ResetContext();
    sLock.unlock();
    format_ = Format();
    if (CodecSurface_ != nullptr) {
        CodecSurface_ = nullptr;
    }
    CHECK_AND_RETURN_RET_LOG(ReleaseBuffers() == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                             "Release codec failed: cannot release buffers");

    state_ = State::Uninitialized;
    AVCODEC_LOGI("Release codec successful, state: Uninitialized");
    return AVCS_ERR_OK;
}

void FCodec::SetSurfaceParameter(const Format &format, const std::string_view &formatKey, uint32_t FORMAT_TYPE)
{
    int32_t val = 0;
    if (formatKey == MediaDescriptionKey::MD_KEY_PIXEL_FORMAT && format.GetIntValue(formatKey, val)) {
        VideoPixelFormat vpf = static_cast<VideoPixelFormat>(val);
        if (vpf == VideoPixelFormat::RGBA || vpf == VideoPixelFormat::BGRA) {
            format_.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, val);
            PixelFormat surfacePixelFmt = TranslateSurfaceFormat(vpf);
            std::lock_guard<std::mutex> oLock(outputMutex_);
            SurfaceMemory::SetConfig(width_, height_, surfacePixelFmt);
        }
    } else if (formatKey == MediaDescriptionKey::MD_KEY_ROTATION_ANGLE && format.GetIntValue(formatKey, val)) {
        GraphicTransformType sr = static_cast<GraphicTransformType>(val);
        if (sr == GraphicTransformType::GRAPHIC_ROTATE_NONE || sr == GraphicTransformType::GRAPHIC_ROTATE_90 ||
            sr == GraphicTransformType::GRAPHIC_ROTATE_180 || sr == GraphicTransformType::GRAPHIC_ROTATE_270) {
            format_.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, val);
            std::lock_guard<std::mutex> oLock(outputMutex_);
            CodecSurface_->SetTransform(sr);
        }
    } else if (formatKey == MediaDescriptionKey::MD_KEY_SCALE_TYPE && format.GetIntValue(formatKey, val)) {
        ScalingMode scaleMode = static_cast<ScalingMode>(val);
        if (scaleMode == ScalingMode::SCALING_MODE_SCALE_TO_WINDOW ||
            scaleMode == ScalingMode::SCALING_MODE_SCALE_CROP) {
            format_.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, val);
            std::lock_guard<std::mutex> oLock(outputMutex_);
            SurfaceMemory::SetScaleType(scaleMode);
        }
    } else {
        AVCODEC_LOGW("Set parameter failed: %{public}.*s", formatKey.size(), formatKey.data());
    }
}

int32_t FCodec::SetParameter(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(IsActive(), AVCS_ERR_INVALID_STATE,
                             "Set parameter failed: not in Running or Flushed state");
    for (auto &it : format.GetFormatMap()) {
        if (CodecSurface_ != nullptr) {
            if (it.second.type == FORMAT_TYPE_INT32) {
                if (it.first == MediaDescriptionKey::MD_KEY_PIXEL_FORMAT ||
                    it.first == MediaDescriptionKey::MD_KEY_ROTATION_ANGLE ||
                    it.first == MediaDescriptionKey::MD_KEY_SCALE_TYPE) {
                    SetSurfaceParameter(format, it.first, it.second.type);
                } else {
                    AVCODEC_LOGW("Current Version, %{public}s is not supported", it.first.c_str());
                }
            }
        } else {
            AVCODEC_LOGW("Current Version, SetParameter function only support Suface Mode");
        }
    }
    AVCODEC_LOGI("Set parameter successful");
    return AVCS_ERR_OK;
}

int32_t FCodec::GetOutputFormat(Format &format)
{
    AVCODEC_SYNC_TRACE;
    format = format_;
    AVCODEC_LOGI("Get outputFormat successful");
    return AVCS_ERR_OK;
}

std::tuple<int32_t, int32_t> FCodec::CalculateBufferSize()
{
    int32_t stride = AlignUp(width_, VIDEO_ALIGN_SIZE);
    int32_t inputBufferSize = static_cast<int32_t>((stride * height_) * (VIDEO_PIX_DEPTH_YUV >> 1));
    int32_t outputBufferSize = inputBufferSize;
    if (CodecSurface_ != nullptr) {
        outputBufferSize = static_cast<int32_t>(stride * height_ * VIDEO_PIX_DEPTH_RGBA);
    }
    AVCODEC_LOGI("Input buffer size = %{public}d, output buffer size=%{public}d", inputBufferSize, outputBufferSize);
    return std::make_tuple(inputBufferSize, outputBufferSize);
}

int32_t FCodec::AllocateInputBuffer(int32_t bufferCnt, int32_t inBufferSize)
{
    int32_t valBufferCnt = 0;
    for (uint32_t i = 0; i < bufferCnt; i++) {
        std::shared_ptr<AVBuffer> buf = std::make_shared<AVBuffer>();
        buf->memory_ = AVSharedMemoryBase::CreateFromLocal(inBufferSize, AVSharedMemory::FLAGS_READ_WRITE,
                                                           std::string("inBuffer") + std::to_string(i));
        if (buf->memory_ == nullptr || buf->memory_->GetBase() == nullptr) {
            AVCODEC_LOGE("Allocate input buffer failed, index=%{public}d", i);
            continue;
        }
        buf->owner_ = AVBuffer::OWNED_BY_USER;
        buffers_[INDEX_INPUT].emplace_back(buf);
        callback_->OnInputBufferAvailable(valBufferCnt);
        valBufferCnt++;
    }
    if (buffers_[INDEX_INPUT].size() < DEFAULT_MIN_BUFFER_CNT) {
        AVCODEC_LOGE("Allocate input buffer failed: only %{public}d buffer is allocated, no memory",
                     buffers_[INDEX_INPUT].size());
        buffers_[INDEX_INPUT].clear();
        return AVCS_ERR_NO_MEMORY;
    }
    return AVCS_ERR_OK;
}

int32_t FCodec::AllocateOutputBuffer(int32_t bufferCnt, int32_t outBufferSize)
{
    int32_t valBufferCnt = 0;
    if (CodecSurface_) {
        CHECK_AND_RETURN_RET_LOG(CodecSurface_->SetQueueSize(bufferCnt) == OHOS::SurfaceError::SURFACE_ERROR_OK,
                                 AVCS_ERR_NO_MEMORY, "Surface set QueueSize=%{public}d failed", bufferCnt);
        int32_t val32 = 0;
        format_.GetIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, val32);
        PixelFormat surfacePixelFmt = TranslateSurfaceFormat(static_cast<VideoPixelFormat>(val32));
        CHECK_AND_RETURN_RET_LOG(surfacePixelFmt != PixelFormat::PIXEL_FMT_BUTT, AVCS_ERR_UNSUPPORT,
                                 "Failed to allocate output buffer: unsupported surface format");
        SurfaceMemory::SetSurface(CodecSurface_);
        SurfaceMemory::SetConfig(static_cast<int32_t>(width_), static_cast<int32_t>(height_), surfacePixelFmt);

        format_.GetIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, val32);
        SurfaceMemory::SetScaleType(static_cast<ScalingMode>(val32));
        format_.GetIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, val32);
        CodecSurface_->SetTransform(static_cast<GraphicTransformType>(val32));
    }
    for (int i = 0; i < bufferCnt; i++) {
        std::shared_ptr<AVBuffer> buf = std::make_shared<AVBuffer>();
        if (CodecSurface_ == nullptr) {
            buf->memory_ = AVSharedMemoryBase::CreateFromLocal(outBufferSize, AVSharedMemory::FLAGS_READ_WRITE,
                                                               std::string("outBuffer") + std::to_string(i));
        } else {
            buf->memory_ = SurfaceMemory::Create();
        }
        if (buf->memory_ == nullptr || buf->memory_->GetBase() == nullptr) {
            AVCODEC_LOGE("Allocate output buffer failed, index=%{public}d", i);
            continue;
        }
        buf->owner_ = AVBuffer::OWNED_BY_CODEC;
        buffers_[INDEX_OUTPUT].emplace_back(buf);
        codecAvailBuffers_.emplace_back(valBufferCnt);
        valBufferCnt++;
    }
    if (buffers_[INDEX_OUTPUT].size() < DEFAULT_MIN_BUFFER_CNT) {
        AVCODEC_LOGE("Allocate output buffer failed: only %{public}d buffer is allocated, no memory",
                     buffers_[INDEX_OUTPUT].size());
        buffers_[INDEX_INPUT].clear();
        buffers_[INDEX_OUTPUT].clear();
        return AVCS_ERR_NO_MEMORY;
    }
    return AVCS_ERR_OK;
}

int32_t FCodec::AllocateBuffers()
{
    int32_t inBufferSize = 0;
    int32_t outBufferSize = 0;
    std::tie(inBufferSize, outBufferSize) = CalculateBufferSize();
    CHECK_AND_RETURN_RET_LOG(inBufferSize > 0 && outBufferSize > 0, AVCS_ERR_INVALID_VAL,
                             "Allocate buffer with input size=%{public}d, output size=%{public}d failed", inBufferSize,
                             outBufferSize);
    int32_t InputBufferCnt = 0;
    int32_t OutputBufferCnt = 0;
    format_.GetIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_BUFFER_COUNT, InputBufferCnt);
    format_.GetIntValue(MediaDescriptionKey::MD_KEY_MAX_OUTPUT_BUFFER_COUNT, OutputBufferCnt);
    if (AllocateInputBuffer(InputBufferCnt, inBufferSize) == AVCS_ERR_NO_MEMORY ||
        AllocateOutputBuffer(OutputBufferCnt, outBufferSize) == AVCS_ERR_NO_MEMORY) {
        return AVCS_ERR_NO_MEMORY;
    }
    AVCODEC_LOGI("Allocate buffers successful");
    return AVCS_ERR_OK;
}

int32_t FCodec::ReleaseBuffers(bool isFlush)
{
    std::unique_lock<std::shared_mutex> iLock(inputMutex_);
    inBufQue_.clear();
    if (!isFlush) {
        buffers_[INDEX_INPUT].clear();
    }
    iLock.unlock();
    std::unique_lock<std::mutex> oLock(outputMutex_);
    codecAvailBuffers_.clear();
    if (!isFlush) {
        buffers_[INDEX_OUTPUT].clear();
    }
    oLock.unlock();
    if (scaleData_[0] != nullptr) {
        if (isConverted_) {
            av_free(scaleData_[0]);
            isConverted_ = false;
            scale_.reset();
        }
        for (int32_t i = 0; i < AV_NUM_DATA_POINTERS; i++) {
            scaleData_[i] = nullptr;
            scaleLineSize_[i] = 0;
        }
    }
    if (CodecSurface_) {
        CodecSurface_->CleanCache();
    }
    return AVCS_ERR_OK;
}

std::shared_ptr<AVSharedMemoryBase> FCodec::GetInputBuffer(size_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(IsActive(), nullptr, "Get input buffer failed: not in Running or Flushed state");
    std::vector<std::shared_ptr<AVBuffer>> &avBuffers = buffers_[INDEX_INPUT];
    CHECK_AND_RETURN_RET_LOG(index < avBuffers.size(), nullptr,
                             "Get buffer failed with bad index, index=%{public}zu", index);
    CHECK_AND_RETURN_RET_LOG(avBuffers[index]->owner_ == AVBuffer::OWNED_BY_USER, nullptr,
                             "Get buffer failed with index=%{public}zu, buffer is not available", index);
    std::shared_lock<std::shared_mutex> iLock(inputMutex_);
    return std::static_pointer_cast<AVSharedMemoryBase>(avBuffers[index]->memory_);
}

std::shared_ptr<AVSharedMemoryBase> FCodec::GetOutputBuffer(size_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), nullptr,
                             "Get output buffer failed: not in Running/Flushed/EOS state");
    CHECK_AND_RETURN_RET_LOG(CodecSurface_ == nullptr, nullptr, "Get output buffer failed: surface output");
    std::vector<std::shared_ptr<AVBuffer>> &avBuffers = buffers_[INDEX_OUTPUT];
    CHECK_AND_RETURN_RET_LOG(index < avBuffers.size(), nullptr,
                             "Get buffer failed with bad index, index=%{public}zu", index);
    CHECK_AND_RETURN_RET_LOG(avBuffers[index]->owner_ == AVBuffer::OWNED_BY_USER, nullptr,
                             "Get buffer failed with index=%{public}zu, buffer is not available", index);

    std::unique_lock<std::mutex> oLock(outputMutex_);
    return std::static_pointer_cast<AVSharedMemoryBase>(avBuffers[index]->memory_);
}

int32_t FCodec::QueueInputBuffer(size_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag &flag)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(IsActive(), AVCS_ERR_INVALID_STATE,
                             "Queue input buffer failed: not in Running or Flushed state");
    std::vector<std::shared_ptr<AVBuffer>> &inBuffers = buffers_[INDEX_INPUT];
    CHECK_AND_RETURN_RET_LOG(index < inBuffers.size(), AVCS_ERR_INVALID_VAL,
                             "Queue input buffer failed with bad index, index=%{public}zu, buffer_size=%{public}zu",
                             index, inBuffers.size());
    CHECK_AND_RETURN_RET_LOG(inBuffers[index]->owner_ == AVBuffer::OWNED_BY_USER, AVCS_ERR_INVALID_OPERATION,
                             "Queue input buffer failed: buffer with index=%{public}zu is not available", index);
    std::unique_lock<std::shared_mutex> iLock(inputMutex_);
    inBufQue_.emplace_back(index);
    inBuffers[index]->owner_ = AVBuffer::OWNED_BY_CODEC;
    inBuffers[index]->bufferInfo_ = info;
    inBuffers[index]->bufferFlag_ = flag;
    return AVCS_ERR_OK;
}

void FCodec::SendFrame()
{
    AVCODEC_SYNC_TRACE;
    if (!IsActive()) {
        AVCODEC_LOGD("Cannot send frame to codec: not in Running or Flushed state");
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    std::unique_lock<std::shared_mutex> iLock(inputMutex_);
    if (inBufQue_.empty()) {
        AVCODEC_LOGD("Cannot send frame to codec: empty input buffer");
        iLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    size_t index = inBufQue_.front();
    std::shared_ptr<AVBuffer> &inputBuffer = buffers_[INDEX_INPUT][index];
    iLock.unlock();
    if (inputBuffer->bufferFlag_ != AVCODEC_BUFFER_FLAG_EOS) {
        avPacket_->data = inputBuffer->memory_->GetBase();
        avPacket_->size = static_cast<int32_t>(inputBuffer->bufferInfo_.size);
        avPacket_->pts = inputBuffer->bufferInfo_.presentationTimeUs;
    } else {
        avPacket_->data = nullptr;
        avPacket_->size = 0;
        std::unique_lock<std::mutex> sendLock(sendMutex_);
        isSendEos_ = true;
        sendCv_.wait(sendLock);
    }
    int ret;
    std::unique_lock<std::mutex> sLock(syncMutex_);
    ret = avcodec_send_packet(avCodecContext_.get(), avPacket_.get());
    av_packet_unref(avPacket_.get());
    sLock.unlock();
    if (ret == 0) {
        std::unique_lock<std::shared_mutex> iLock(inputMutex_);
        inBufQue_.pop_front();
        iLock.unlock();
        inputBuffer->owner_ = AVBuffer::OWNED_BY_USER;
        callback_->OnInputBufferAvailable(index);
    } else {
        AVCODEC_LOGD("Cannot send frame to codec: ffmpeg ret = %{public}s", AVStrError(ret).c_str());
        std::unique_lock<std::mutex> sendLock(sendMutex_);
        isSendWait_ = true;
        sendCv_.wait(sendLock); // 接收帧后唤醒
    }
}

int32_t FCodec::FillFrameBufferImpl(const std::shared_ptr<AVBuffer> &frameBuffer, AVPixelFormat ffmpegFormat,
                                    VideoPixelFormat outputPixelFmt)
{
    int32_t ret;
    std::shared_ptr<AVSharedMemory> frameMomory = frameBuffer->memory_;
    if (IsYuvFormat(ffmpegFormat)) {
        std::shared_ptr<AVSharedMemoryBase> buffer = std::static_pointer_cast<AVSharedMemoryBase>(frameMomory);
        CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AVCS_ERR_INVALID_VAL,
                                 "Failed to dynamic cast AVSharedMemory to ShareMemory");
        buffer->ClearUsedSize();
        ret = WriteYuvData(buffer, scaleData_, scaleLineSize_, format_);
        frameBuffer->bufferInfo_.size = buffer->GetUsedSize();
    } else if (IsRgbFormat(ffmpegFormat)) {
        std::shared_ptr<SurfaceMemory> buffer = std::static_pointer_cast<SurfaceMemory>(frameMomory);
        CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AVCS_ERR_INVALID_VAL,
                                 "Failed to dynamic cast AVSharedMemory to SurfaceMemory");
        buffer->ClearUsedSize();
        ret = WriteRgbData(buffer, scaleData_, scaleLineSize_, format_);
        frameBuffer->bufferInfo_.size = buffer->GetUsedSize();
    } else {
        AVCODEC_LOGE("Fill frame buffer failed : unsupported pixel format: %{public}d", outputPixelFmt);
        return AVCS_ERR_UNSUPPORT;
    }

    frameBuffer->bufferInfo_.presentationTimeUs = cachedFrame_->pts;
    AVCODEC_LOGD("Fill frame buffer successful");
    return ret;
}

int32_t FCodec::FillFrameBuffer(const std::shared_ptr<AVBuffer> &frameBuffer)
{
    CHECK_AND_RETURN_RET_LOG((cachedFrame_->flags & AV_FRAME_FLAG_CORRUPT) == 0, AVCS_ERR_INVALID_VAL,
                             "Recevie frame from codec failed: decoded frame is corrupt");
    AVPixelFormat ffmpegFormat = static_cast<AVPixelFormat>(cachedFrame_->format);
    VideoPixelFormat outputPixelFmt;
    if (CodecSurface_ == nullptr) {
        outputPixelFmt = ConvertPixelFormatFromFFmpeg(cachedFrame_->format);
        CHECK_AND_RETURN_RET_LOG(outputPixelFmt != VideoPixelFormat::UNKNOWN, AVCS_ERR_UNSUPPORT,
                                 "Recevie frame from codec failed: unsupported pixel fmt");
    } else {
        int32_t val32;
        format_.GetIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, val32);
        outputPixelFmt = static_cast<VideoPixelFormat>(val32);
        ffmpegFormat = ConvertPixelFormatToFFmpeg(outputPixelFmt);
        CHECK_AND_RETURN_RET_LOG(ffmpegFormat != AVPixelFormat::AV_PIX_FMT_NONE, AVCS_ERR_UNSUPPORT,
                                 "Recevie frame from codec failed: unsupported pixel fmt");
    }
    if (CodecSurface_ == nullptr || ffmpegFormat == static_cast<AVPixelFormat>(cachedFrame_->format)) {
        for (int32_t i = 0; cachedFrame_->linesize[i] > 0; i++) {
            scaleData_[i] = cachedFrame_->data[i];
            scaleLineSize_[i] = cachedFrame_->linesize[i];
        }
        format_.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(outputPixelFmt));
    } else {
        int32_t ret = ConvertVideoFrame(scale_, cachedFrame_, scaleData_, scaleLineSize_, ffmpegFormat);
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Scale video frame failed: %{public}d", ret);
        isConverted_ = true;
    }
    return FillFrameBufferImpl(frameBuffer, ffmpegFormat, outputPixelFmt);
}

void FCodec::FramePostProcess(std::shared_ptr<AVBuffer> frameBuffer, int32_t status, int ret)
{
    size_t index = codecAvailBuffers_.front();
    if (status == AVCS_ERR_OK) {
        std::unique_lock<std::mutex> oLock(outputMutex_);
        codecAvailBuffers_.pop_front();
        oLock.unlock();
        frameBuffer->owner_ = AVBuffer::OWNED_BY_USER;
        if (ret == AVERROR_EOF) {
            callback_->OnOutputBufferAvailable(index, frameBuffer->bufferInfo_, AVCODEC_BUFFER_FLAG_EOS);
        } else {
            if (isSendWait_) {
                std::unique_lock<std::mutex> sLock(sendMutex_);
                isSendWait_ = false;
                sendCv_.notify_one();
            }
            callback_->OnOutputBufferAvailable(index, frameBuffer->bufferInfo_, AVCODEC_BUFFER_FLAG_NONE);
        }
    } else if (status == AVCS_ERR_UNSUPPORT) {
        AVCODEC_LOGE("Recevie frame from codec failed: OnError");
        callback_->OnError(AVCODEC_ERROR_EXTEND_START, status);
        state_ = State::Error;
    } else {
        AVCODEC_LOGI("Recevie frame from codec failed");
    }
}

void FCodec::ReceiveFrame()
{
    AVCODEC_SYNC_TRACE;
    if (!IsActive()) {
        AVCODEC_LOGD("Cannot recevie frame from codec: not in Running or Flushed state");
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    std::unique_lock<std::mutex> oLock(outputMutex_);
    outputCv_.wait(oLock, [this]() { return codecAvailBuffers_.size() > 0; });
    if (!IsActive()) {
        oLock.unlock();
        AVCODEC_LOGD("Cannot recevie frame from codec: not in Running or Flushed state");
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    size_t index = codecAvailBuffers_.front();
    std::shared_ptr<AVBuffer> frameBuffer = buffers_[INDEX_OUTPUT][index];
    oLock.unlock();
    int ret;
    std::unique_lock<std::mutex> sLock(syncMutex_);
    av_frame_unref(cachedFrame_.get());
    ret = avcodec_receive_frame(avCodecContext_.get(), cachedFrame_.get());
    sLock.unlock();
    int32_t status = AVCS_ERR_OK;
    if (ret >= 0) {
        status = FillFrameBuffer(frameBuffer);
    } else if (ret == AVERROR_EOF) {
        AVCODEC_LOGI("Receive EOS frame");
        frameBuffer->bufferFlag_ = AVCODEC_BUFFER_FLAG_EOS;
        state_ = State::EOS;
        std::unique_lock<std::mutex> sLock(syncMutex_);
        avcodec_flush_buffers(avCodecContext_.get());
    } else if (ret == AVERROR(EAGAIN)) {
        if (isSendWait_ || isSendEos_) {
            std::lock_guard<std::mutex> sLock(sendMutex_);
            sendCv_.notify_one();
        }
        return;
    } else {
        AVCODEC_LOGE("Failed to decoder: unknow error");
        return;
    }
    FramePostProcess(frameBuffer, status, ret);
}

void FCodec::RenderFrame()
{
    AVCODEC_SYNC_TRACE;
    if (!IsActive()) {
        AVCODEC_LOGD("Failed to render frame to codec: not in Running or Flushed state");
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    std::unique_lock<std::mutex> oLock(outputMutex_);
    if (renderBuffers_.empty()) {
        AVCODEC_LOGD("Failed to render frame to codec: empty render buffer");
        oLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        return;
    }
    size_t index = renderBuffers_.front();
    std::shared_ptr<AVBuffer> outputBuffer = buffers_[INDEX_OUTPUT][index];
    oLock.unlock();
    std::shared_ptr<SurfaceMemory> surfaceMemory = std::static_pointer_cast<SurfaceMemory>(outputBuffer->memory_);
    while (true) {
        if (surfaceMemory->GetSurfaceBuffer() == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TRY_DECODE_TIME));
        } else {
            AVCODEC_LOGD("render frame success, index=%{public}zu", index);
            std::lock_guard<std::mutex> oLock(outputMutex_);
            codecAvailBuffers_.emplace_back(index);
            renderBuffers_.pop_front();
            outputBuffer->owner_ = AVBuffer::OWNED_BY_CODEC;
            if (codecAvailBuffers_.size() == 1) {
                outputCv_.notify_one();
            }
            break;
        }
    }
}

int32_t FCodec::ReleaseOutputBuffer(size_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), AVCS_ERR_INVALID_STATE,
                             "Release output buffer failed: not in Running/Flushed/EOS");
    std::unique_lock<std::mutex> oLock(outputMutex_);
    if (std::find(codecAvailBuffers_.begin(), codecAvailBuffers_.end(), index) == codecAvailBuffers_.end() &&
        buffers_[INDEX_OUTPUT][index]->owner_ == AVBuffer::OWNED_BY_USER) {
        buffers_[INDEX_OUTPUT][index]->owner_ = AVBuffer::OWNED_BY_CODEC;
        codecAvailBuffers_.emplace_back(index);
        if (codecAvailBuffers_.size() == 1) {
            outputCv_.notify_one();
        }
        return AVCS_ERR_OK;
    } else {
        AVCODEC_LOGE("Release output buffer failed: check your index=%{public}zu", index);
        return AVCS_ERR_INVALID_VAL;
    }
}

int32_t FCodec::UpdateSurfaceMemory(std::shared_ptr<SurfaceMemory> &surfaceMemory, int64_t pts)
{
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), AVCS_ERR_INVALID_STATE,
                             "Failed to update surface memory: Invalid state");

    sptr<SurfaceBuffer> surfaceBuffer = surfaceMemory->GetSurfaceBuffer();
    CHECK_AND_RETURN_RET_LOG(surfaceBuffer != nullptr, AVCS_ERR_INVALID_VAL,
                             "Failed to update surface memory: surface buffer is NULL");
    OHOS::BufferFlushConfig flushConfig = {{0, 0, surfaceBuffer->GetWidth(), surfaceBuffer->GetHeight()}, pts};
    surfaceMemory->SetNeedRender(true);
    surfaceMemory->UpdateSurfaceBufferScaleMode();
    auto res = CodecSurface_->FlushBuffer(surfaceBuffer, surfaceMemory->GetFlushFence(), flushConfig);
    if (res != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        AVCODEC_LOGW("Failed to update surface memory: %{public}d", res);
        surfaceMemory->SetNeedRender(false);
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}

int32_t FCodec::RenderOutputBuffer(size_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((IsActive() || state_ == State::EOS), AVCS_ERR_INVALID_STATE,
                             "Failed to render output buffer: invalid state");

    if (std::find(codecAvailBuffers_.begin(), codecAvailBuffers_.end(), index) == codecAvailBuffers_.end() &&
        buffers_[INDEX_OUTPUT][index]->owner_ == AVBuffer::OWNED_BY_USER) {
        // render
        std::shared_ptr<AVBuffer> outputBuffer = buffers_[INDEX_OUTPUT][index];
        std::shared_ptr<SurfaceMemory> surfaceMemory = std::static_pointer_cast<SurfaceMemory>(outputBuffer->memory_);

        int32_t ret = UpdateSurfaceMemory(surfaceMemory, outputBuffer->bufferInfo_.presentationTimeUs);
        if (ret != AVCS_ERR_OK) {
            AVCODEC_LOGW("Update surface memory failed: %{public}d", static_cast<int32_t>(ret));
        } else {
            AVCODEC_LOGD("Update surface memory successful");
        }
        surfaceMemory->ReleaseSurfaceBuffer();
        std::lock_guard<std::mutex> oLock(outputMutex_);
        outputBuffer->owner_ = AVBuffer::OWNED_BY_SURFACE;
        renderBuffers_.emplace_back(index);
        return AVCS_ERR_OK;
    } else {
        AVCODEC_LOGE("Failed to render output buffer with bad index, index=%{public}zu", index);
        return AVCS_ERR_INVALID_VAL;
    }
}

int32_t FCodec::SetOutputSurface(sptr<Surface> CodecSurface)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((!IsActive()), AVCS_ERR_INVALID_STATE,
                             "Set output surface failed: not in Running/Flushed state");
    CHECK_AND_RETURN_RET_LOG(CodecSurface != nullptr, AVCS_ERR_INVALID_VAL,
                             "Set output surface failed: surface is NULL");
    CodecSurface_ = CodecSurface;
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::RGBA));
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                        static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                        static_cast<int32_t>(GraphicTransformType::GRAPHIC_ROTATE_NONE));
    if (renderTask_ == nullptr) {
        renderTask_ = std::make_shared<TaskThread>("RenderFrame");
        renderTask_->RegisterHandler([this] { (void)RenderFrame(); });
    }
    return AVCS_ERR_OK;
}

int32_t FCodec::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG((!IsActive()), AVCS_ERR_INVALID_STATE,
                             "Set callback failed: not in Running/Flushed state");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AVCS_ERR_INVALID_VAL, "Set callback failed: callback is NULL");
    callback_ = callback;
    return AVCS_ERR_OK;
}

int32_t FCodec::Pause()
{
    return AVCS_ERR_OK;
}

int32_t FCodec::Resume()
{
    return AVCS_ERR_OK;
}
} // namespace Codec
} // namespace Media
} // namespace OHOS