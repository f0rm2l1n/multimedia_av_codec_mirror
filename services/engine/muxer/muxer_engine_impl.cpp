/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "muxer_engine_impl.h"
#include <set>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"
#include "avcodec_log.h"
#include "muxer_factory.h"
#include "avcodec_dfx.h"
#include "avcodec_info.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerEngineImpl"};
    constexpr int32_t MAX_LATITUDE = 90;
    constexpr int32_t MIN_LATITUDE = -90;
    constexpr int32_t MAX_LONGITUDE = 180;
    constexpr int32_t MIN_LONGITUDE = -180;
    constexpr int32_t ERR_TRACK_INDEX = -1;
}

namespace OHOS {
namespace Media {
const std::map<uint32_t, std::set<std::string_view>> MUX_FORMAT_INFO = {
    {OUTPUT_FORMAT_MPEG_4, {CodecMimeType::AUDIO_MPEG, CodecMimeType::AUDIO_AAC,
                                CodecMimeType::VIDEO_AVC, CodecMimeType::VIDEO_MPEG4,
                                CodecMimeType::IMAGE_JPG, CodecMimeType::IMAGE_PNG, CodecMimeType::IMAGE_BMP}},
    {OUTPUT_FORMAT_M4A, {CodecMimeType::AUDIO_AAC,
                            CodecMimeType::VIDEO_AVC, CodecMimeType::VIDEO_MPEG4,
                            CodecMimeType::IMAGE_JPG, CodecMimeType::IMAGE_PNG, CodecMimeType::IMAGE_BMP}},
};

const std::map<std::string_view, std::set<std::string_view>> MUX_MIME_INFO = {
    {CodecMimeType::AUDIO_MPEG, {MediaDescriptionKey::MD_KEY_SAMPLE_RATE, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT}},
    {CodecMimeType::AUDIO_AAC, {MediaDescriptionKey::MD_KEY_SAMPLE_RATE, MediaDescriptionKey::MD_KEY_CHANNEL_COUNT}},
    {CodecMimeType::VIDEO_AVC, {MediaDescriptionKey::MD_KEY_WIDTH, MediaDescriptionKey::MD_KEY_HEIGHT}},
    {CodecMimeType::VIDEO_MPEG4, {MediaDescriptionKey::MD_KEY_WIDTH, MediaDescriptionKey::MD_KEY_HEIGHT}},
    {CodecMimeType::IMAGE_JPG, {MediaDescriptionKey::MD_KEY_WIDTH, MediaDescriptionKey::MD_KEY_HEIGHT}},
    {CodecMimeType::IMAGE_PNG, {MediaDescriptionKey::MD_KEY_WIDTH, MediaDescriptionKey::MD_KEY_HEIGHT}},
    {CodecMimeType::IMAGE_BMP, {MediaDescriptionKey::MD_KEY_WIDTH, MediaDescriptionKey::MD_KEY_HEIGHT}},
};

std::shared_ptr<IMuxerEngine> IMuxerEngineFactory::CreateMuxerEngine(int32_t appUid, int32_t appPid, int32_t fd, OutputFormat format)
{
    AVCodecTrace trace("IMuxerEngineFactory::CreateMuxerEngine");
    CHECK_AND_RETURN_RET_LOG((fcntl(fd, F_GETFL, 0) & O_RDWR) == O_RDWR, nullptr, "no permission to read and write fd");
    CHECK_AND_RETURN_RET_LOG(lseek(fd, 0, SEEK_CUR) != -1, nullptr, "the fd is not seekable");
    std::shared_ptr<IMuxerEngine> muxerEngineImpl = std::make_shared<MuxerEngineImpl>(appUid, appPid, fd, format);
    return muxerEngineImpl;
}

MuxerEngineImpl::MuxerEngineImpl(int32_t appUid, int32_t appPid, int32_t fd, OutputFormat format)
    : appUid_(appUid), appPid_(appPid), fd_(fd), format_(format), que_("muxer_write_queue")
{
    format_ = (format_ == OUTPUT_FORMAT_DEFAULT) ? OUTPUT_FORMAT_MPEG_4 : format_;
    AVCodecTrace trace("MuxerEngine::Create");
    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    muxer_ = Plugin::MuxerFactory::Instance().CreatePlugin(fd_, format_);
    if (muxer_ != nullptr && fd_ >= 0) {
        state_ = INITIALIZED;
        AVCODEC_LOGI("state_ is INITIALIZED");
    } else {
        AVCODEC_LOGE("state_ is UNINITIALIZED");
    }
}

MuxerEngineImpl::~MuxerEngineImpl()
{
    AVCODEC_LOGD("Destroy");

    if (state_ == STARTED) {
        que_.SetActive(false);
        StopThread();
    }

    appUid_ = -1;
    appPid_ = -1;
    muxer_ = nullptr;
    tracks_.clear();
    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t MuxerEngineImpl::SetLocation(float latitude, float longitude)
{
    AVCodecTrace trace("MuxerEngine::SetLocation");
    AVCODEC_LOGI("SetLocation");
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION, 
        "The state is not INITIALIZED, the interface must be called after constructor and before Start(). "
        "The current state is %{public}s", ConvertStateToString(state_).c_str());
    if (latitude < MIN_LATITUDE || latitude > MAX_LATITUDE || 
        longitude < MIN_LONGITUDE || longitude > MAX_LONGITUDE) {
        AVCODEC_LOGW("Invalid GeoLocation, latitude must be greater than %{public}d and less than %{public}d,"
            "longitude must be greater than %{public}d and less than %{public}d",
            MIN_LATITUDE, MAX_LATITUDE, MIN_LONGITUDE, MAX_LONGITUDE);
        return AVCS_ERR_INVALID_VAL;
    }

    return TranslatePluginStatus(muxer_->SetLocation(latitude, longitude));
}

int32_t MuxerEngineImpl::SetRotation(int32_t rotation)
{
    AVCodecTrace trace("MuxerEngine::SetRotation");
    AVCODEC_LOGI("SetRotation");
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION,
        "The state is not INITIALIZED, the interface must be called after constructor and before Start(). "
        "The current state is %{public}s", ConvertStateToString(state_).c_str());
    if (rotation != VIDEO_ROTATION_0 && rotation != VIDEO_ROTATION_90 && 
        rotation != VIDEO_ROTATION_180 && rotation != VIDEO_ROTATION_270) {
        AVCODEC_LOGW("Invalid rotation: %{public}d, keep default 0", rotation);
        return AVCS_ERR_INVALID_VAL;
    }

    return TranslatePluginStatus(muxer_->SetRotation(rotation));
}

int32_t MuxerEngineImpl::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    AVCodecTrace trace("MuxerEngine::AddTrack");
    AVCODEC_LOGI("AddTrack");
    std::unique_lock<std::mutex> lock(mutex_);
    trackIndex = ERR_TRACK_INDEX;
    CHECK_AND_RETURN_RET_LOG(state_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION,
        "The state is not INITIALIZED, the interface must be called after constructor and before Start(). "
        "The current state is %{public}s", ConvertStateToString(state_).c_str());
    std::string mimeType = {};
    CHECK_AND_RETURN_RET_LOG(trackDesc.GetStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, mimeType),
        AVCS_ERR_INVALID_VAL, "track format does not contain mime");
    CHECK_AND_RETURN_RET_LOG(CanAddTrack(mimeType), AVCS_ERR_UNSUPPORT_CONTAINER_TYPE,
        "track mime is unsupported: %{public}s", mimeType.c_str());
    CHECK_AND_RETURN_RET_LOG(CheckKeys(mimeType, trackDesc), AVCS_ERR_INVALID_VAL,
        "track format keys not contained");

    int32_t trackId = -1;
    Plugin::Status ret = muxer_->AddTrack(trackId, trackDesc);
    CHECK_AND_RETURN_RET_LOG(ret == Plugin::Status::NO_ERROR, TranslatePluginStatus(ret), "AddTrack failed");
    CHECK_AND_RETURN_RET_LOG(trackId >= 0, AVCS_ERR_INVALID_OPERATION,
        "The track index is greater than or equal to 0, less than 99");
    trackIndex = trackId;
    tracks_[trackIndex] = mimeType;
    mediaDescMap_.emplace(trackIndex, MediaDescription(trackDesc));

    return AVCS_ERR_OK;
}

int32_t MuxerEngineImpl::Start()
{
    AVCodecTrace trace("MuxerEngine::Start");
    AVCODEC_LOGI("Start");
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == INITIALIZED, AVCS_ERR_INVALID_OPERATION,
        "The state is not INITIALIZED, the interface must be called after AddTrack() and before WriteSampleBuffer(). "
        "The current state is %{public}s", ConvertStateToString(state_).c_str());
    CHECK_AND_RETURN_RET_LOG(tracks_.size() > 0, AVCS_ERR_INVALID_OPERATION, 
        "The track count is error, count is %{public}d", tracks_.size());
    Plugin::Status ret = muxer_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == Plugin::Status::NO_ERROR, TranslatePluginStatus(ret), "Start failed");
    state_ = STARTED;
    StartThread("muxer_write_loop");

    return AVCS_ERR_OK;
}

int32_t MuxerEngineImpl::WriteSampleBuffer(std::shared_ptr<AVSharedMemory> sampleBuffer, const TrackSampleInfo &info)
{
    AVCodecTrace trace("MuxerEngine::WriteSampleBuffer");
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == STARTED, AVCS_ERR_INVALID_OPERATION, 
        "The state is not STARTED, the interface must be called after Start() and before Stop(). "
        "The current state is %{public}s", ConvertStateToString(state_).c_str());
    CHECK_AND_RETURN_RET_LOG(tracks_.find(info.trackIndex) != tracks_.end(), AVCS_ERR_INVALID_VAL,
        "The track index does not exist");
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr && info.timeUs >= 0, AVCS_ERR_INVALID_VAL, "Invalid memory");

    std::shared_ptr<BlockBuffer> blockBuffer = std::make_shared<BlockBuffer>();
    blockBuffer->buffer_ = sampleBuffer;
    blockBuffer->info_ = info;
    que_.Push(blockBuffer);

    return AVCS_ERR_OK;
}

int32_t MuxerEngineImpl::Stop()
{
    AVCodecTrace trace("MuxerEngine::Stop");
    AVCODEC_LOGI("Stop");
    std::unique_lock<std::mutex> lock(mutex_);
    if (state_ == STOPPED) {
        AVCODEC_LOGW("current state is STOPPED!");
        return AVCS_ERR_OK;
    }
    CHECK_AND_RETURN_RET_LOG(state_ == STARTED, AVCS_ERR_INVALID_OPERATION,
        "The state is not STARTED. The current state is %{public}s", ConvertStateToString(state_).c_str());
    state_ = STOPPED;
    que_.SetActive(false, false);
    cond_.wait(lock, [this] { return que_.Empty(); });
    StopThread();
    return TranslatePluginStatus(muxer_->Stop());
}

int32_t MuxerEngineImpl::DumpInfo(int32_t fd)
{
    AVCODEC_LOGI("DumpInfo fd:%{public}d", fd);
    std::string dumpString;
    dumpString += "In MuxerEngine::DumpInfo\n";
    dumpString += "Current MuxerEngine state is: " + ConvertStateToString(state_) + "\n";
    dumpString += "Current MuxerEngine output format is: " + std::to_string(format_) + "\n";
    dumpString += "\nCurrent MuxerEngine media description is:\n";
    if (fd < 0) {
        AVCODEC_LOGI("%{public}s", dumpString.c_str());
    } else {
        write(fd, dumpString.c_str(), dumpString.size());
    }

    for (auto it = mediaDescMap_.begin(); it != mediaDescMap_.end(); ++it) {
        dumpString = "Track id: " + std::to_string(it->first) + "\n";
        if (fd < 0) {
            AVCODEC_LOGI("%{public}s", dumpString.c_str());
        } else {
            write(fd, dumpString.c_str(), dumpString.size());
        }
        DumpMediaDescription(fd, it->second);
    }

    return AVCS_ERR_OK;
}

void MuxerEngineImpl::DumpMediaDescription(int32_t fd, const MediaDescription &trackDesc)
{
    std::string dumpString;
    auto dataMap = trackDesc.GetFormatMap();
    for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
        switch (it->second.type) {
            case FORMAT_TYPE_INT32:
                dumpString = it->first + ": " + std::to_string(it->second.val.int32Val) + "\n";
                break;
            case FORMAT_TYPE_INT64:
                dumpString = it->first + ": " + std::to_string(it->second.val.int64Val) + "\n";
                break;
            case FORMAT_TYPE_FLOAT:
                dumpString = it->first + ": " + std::to_string(it->second.val.floatVal) + "\n";
                break;
            case FORMAT_TYPE_DOUBLE:
                dumpString = it->first + ": " + std::to_string(it->second.val.doubleVal) + "\n";
                break;
            case FORMAT_TYPE_STRING:
                dumpString = it->first + ": " + it->second.stringVal + "\n";
                break;
            case FORMAT_TYPE_ADDR:
                dumpString = it->first + " size:" + std::to_string(it->second.size) + " buffer:\n";
                if (fd < 0) {
                    AVCODEC_LOGI("%{public}s", dumpString.c_str());
                } else {
                    write(fd, dumpString.c_str(), dumpString.size());
                    write(fd, it->second.addr, it->second.size);
                }
                dumpString = "\n";
                break;
            default:
                AVCODEC_LOGE("DumpMediaDescription fail to Marshalling Key: %{public}s", it->first.c_str());
                continue;
        }
        if (fd < 0) {
            AVCODEC_LOGI("%{public}s", dumpString.c_str());
        } else {
            write(fd, dumpString.c_str(), dumpString.size());
        }
    }
}

int32_t MuxerEngineImpl::StartThread(std::string name)
{
    threadName_ = name;
    if (thread_ != nullptr) {
        AVCODEC_LOGW("Started already! [%{public}s]", threadName_.c_str());
        return AVCS_ERR_OK;
    }
    isThreadExit_ = false;
    thread_ = std::make_unique<std::thread>(&MuxerEngineImpl::ThreadProcessor, this);
    AVCodecTrace::TraceBegin("muxer_write_thread", FAKE_POINTER(thread_.get()));
    AVCODEC_LOGD("thread started! [%{public}s]", threadName_.c_str());
    return AVCS_ERR_OK;
}

int32_t MuxerEngineImpl::StopThread() noexcept
{
    if (isThreadExit_) {
        AVCODEC_LOGD("Stopped already! [%{public}s]", threadName_.c_str());
        return AVCS_ERR_OK;
    }
    if (std::this_thread::get_id() == thread_->get_id()) {
        AVCODEC_LOGD("Stop at the task thread, reject");
        return AVCS_ERR_INVALID_OPERATION;
    }

    std::unique_ptr<std::thread> t;
    isThreadExit_ = true;
    cond_.notify_all();
    std::swap(thread_, t);
    if (t != nullptr && t->joinable()) {
        t->join();
    }
    thread_ = nullptr;
    return AVCS_ERR_OK;
}

void MuxerEngineImpl::ThreadProcessor()
{
    AVCODEC_LOGD("Enter ThreadProcessor [%{public}s]", threadName_.c_str());
    constexpr uint32_t nameSizeMax = 15;
    pthread_setname_np(pthread_self(), threadName_.substr(0, nameSizeMax).c_str());
    int32_t taskId = FAKE_POINTER(thread_.get());
    for (;;) {
        AVCodecTrace trace(threadName_);
        if (isThreadExit_) {
            AVCodecTrace::TraceEnd("muxer_write_thread", taskId);
            AVCODEC_LOGD("Exit ThreadProcessor [%{public}s]", threadName_.c_str());
            return;
        }
        auto buffer = que_.Pop();
        if (buffer != nullptr) {
            (void)muxer_->WriteSampleBuffer(buffer->buffer_->GetBase(), buffer->info_);
        }
        if (que_.Empty()) {
            cond_.notify_all();
        }
    }
}

bool MuxerEngineImpl::CanAddTrack(std::string &mimeType)
{
    auto it = MUX_FORMAT_INFO.find(format_);
    if (it == MUX_FORMAT_INFO.end()) {
        return false;
    }
    return it->second.find(mimeType) != it->second.end();
}

bool MuxerEngineImpl::CheckKeys(std::string &mimeType, const MediaDescription &trackDesc)
{
    bool ret = true;
    auto it = MUX_MIME_INFO.find(mimeType);
    if (it == MUX_MIME_INFO.end()) {
        return ret; // 不做检查
    }

    for (auto &key : it->second) {
        if(!trackDesc.ContainKey(key)) {
            ret = false;
            AVCODEC_LOGE("the MediaDescriptionKey %{public}s not contained", key.data());
        }
    }
    return ret;
}

std::string MuxerEngineImpl::ConvertStateToString(int32_t state)
{
    std::string stateInfo {};
    switch (state)
    {
    case UNINITIALIZED:
        stateInfo = "UNINITIALIZED";
        break;
    case INITIALIZED:
        stateInfo = "INITIALIZED";
        break;
    case STARTED:
        stateInfo = "STARTED";
        break;
    case STOPPED:
        stateInfo = "STOPPED";
        break;
    default:
        break;
    }
    return stateInfo;
}

int32_t MuxerEngineImpl::TranslatePluginStatus(Plugin::Status error)
{
    const static std::map<Plugin::Status, int32_t> g_transTable = {
        {Plugin::Status::END_OF_STREAM, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::OK, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::NO_ERROR, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::ERROR_UNKNOWN, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_PLUGIN_ALREADY_EXISTS, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_INCOMPATIBLE_VERSION, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NO_MEMORY, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY},
        {Plugin::Status::ERROR_WRONG_STATE, AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION},
        {Plugin::Status::ERROR_UNIMPLEMENTED, AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT},
        {Plugin::Status::ERROR_INVALID_PARAMETER, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_INVALID_DATA, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_MISMATCHED_TYPE, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_TIMED_OUT, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_UNSUPPORTED_FORMAT, AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_CONTAINER_TYPE},
        {Plugin::Status::ERROR_NOT_ENOUGH_DATA, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NOT_EXISTED, AVCodecServiceErrCode::AVCS_ERR_OPEN_FILE_FAILED},
        {Plugin::Status::ERROR_AGAIN, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_PERMISSION_DENIED, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NULL_POINTER, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_INVALID_OPERATION, AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION},
        {Plugin::Status::ERROR_CLIENT, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_SERVER, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_DELAY_READY, AVCodecServiceErrCode::AVCS_ERR_OK},
    };
    auto ite = g_transTable.find(error);
    if (ite == g_transTable.end()) {
        return AVCS_ERR_UNKNOWN;
    }
    return ite->second;
}
} // Media
} // OHOS
