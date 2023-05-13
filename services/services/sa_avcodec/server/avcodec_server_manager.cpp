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

#include "avcodec_server_manager.h"
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_log_dump.h"
#include "avcodec_xcollie.h"
#ifdef SUPPORT_CODEC
#include "codec_service_stub.h"
#endif
#ifdef SUPPORT_CODECLIST
#include "codeclist_service_stub.h"
#endif
#ifdef SUPPORT_DEMUXER
#include "demuxer_service_stub.h"
#endif
#ifdef SUPPORT_MUXER
#include "muxer_service_stub.h"
#endif
#ifdef SUPPORT_SOURCE
#include "source_service_stub.h"
#endif

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServerManager"};
}


namespace OHOS {
namespace Media {
constexpr uint32_t SERVER_MAX_NUMBER = 16;
AVCodecServerManager &AVCodecServerManager::GetInstance()
{
    static AVCodecServerManager instance;
    return instance;
}

int32_t WriteInfo(int32_t fd, std::string &dumpString, std::vector<Dumper> dumpers, bool needDetail)
{
    int32_t i = 0;
    for (auto iter : dumpers) {
        dumpString += "-----Instance #" + std::to_string(i) + ": ";
        dumpString += "pid = ";
        dumpString += std::to_string(iter.pid_);
        dumpString += " uid = ";
        dumpString += std::to_string(iter.uid_);
        dumpString += "-----\n";
        if (fd != -1) {
            write(fd, dumpString.c_str(), dumpString.size());
            dumpString.clear();
        }
        i++;
        if (needDetail && iter.entry_(fd) != AVCS_ERR_OK) {
            return OHOS::INVALID_OPERATION;
        }
    }
    if (fd != -1) {
        write(fd, dumpString.c_str(), dumpString.size());
    } else {
        AVCODEC_LOGI("%{public}s", dumpString.c_str());
    }
    dumpString.clear();

    return OHOS::NO_ERROR;
}

int32_t AVCodecServerManager::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string dumpString;
    std::unordered_set<std::u16string> argSets;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argSets.insert(args[index]);
    }

#ifdef SUPPORT_CODEC
    dumpString += "------------------CodecServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::CODEC],
        argSets.find(u"codec") != argSets.end()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write CodecServer information");
        return OHOS::INVALID_OPERATION;
    }
#endif

#ifdef SUPPORT_MUXER
    dumpString += "------------------MuxerServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::MUXER],
        argSets.find(u"muxer") != argSets.end()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write MuxerServer information");
        return OHOS::INVALID_OPERATION;
    }
#endif

#ifdef SUPPORT_DEMUXER
    dumpString += "------------------AVDemuxerServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::DEMUXER],
        argSets.find(u"demuxer") != argSets.end()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write AVDemuxerServer information");
        return OHOS::INVALID_OPERATION;
    }
#endif

#ifdef SUPPORT_SOURCE
    dumpString += "------------------SourceServer------------------\n";
    if (WriteInfo(fd, dumpString, dumperTbl_[StubType::SOURCE],
        argSets.find(u"source") != argSets.end()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write SourceServer information");
        return OHOS::INVALID_OPERATION;
    }
#endif

    if (AVCodecXCollie::GetInstance().Dump(fd) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("Failed to write xcollie dump information");
        return OHOS::INVALID_OPERATION;
    }

    return OHOS::NO_ERROR;
}

AVCodecServerManager::AVCodecServerManager()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServerManager::~AVCodecServerManager()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

sptr<IRemoteObject> AVCodecServerManager::CreateStubObject(StubType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (type) {
#ifdef SUPPORT_CODECLIST
        case CODECLIST: {
            return CreateCodecListStubObject();
        }
#endif
#ifdef SUPPORT_CODEC
        case CODEC: {
            return CreateCodecStubObject();
        }
#endif
#ifdef SUPPORT_MUXER
        case MUXER: {
            return CreateMuxerStubObject();
        }
#endif
#ifdef SUPPORT_DEMUXER
        case DEMUXER: {
            return CreateDemuxerStubObject();
        }
#endif
#ifdef SUPPORT_SOURCE
        case SOURCE: {
            return CreateSourceStubObject();
        }
#endif
        default: {
            AVCODEC_LOGE("default case, av_codec server manager failed");
            return nullptr;
        }
    }
}

#ifdef SUPPORT_CODECLIST
sptr<IRemoteObject> AVCodecServerManager::CreateCodecListStubObject()
{
    if (codecListStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of codeclist services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", codecListStubMap_.size());
        return nullptr;
    }
    sptr<CodecListServiceStub> stub = CodecListServiceStub::Create();
    if (stub == nullptr) {
        AVCODEC_LOGE("failed to create AVCodecListServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = stub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        codecListStubMap_[object] = pid;
        AVCODEC_LOGD("The number of codeclist services(%{public}zu).", codecListStubMap_.size());
    }
    return object;
}
#endif
#ifdef SUPPORT_CODEC
sptr<IRemoteObject> AVCodecServerManager::CreateCodecStubObject()
{
    if (codecStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of codec services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", codecStubMap_.size());
        return nullptr;
    }
    sptr<CodecServiceStub> stub = CodecServiceStub::Create();
    if (stub == nullptr) {
        AVCODEC_LOGE("failed to create CodecServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = stub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        codecStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [stub](int32_t fd) -> int32_t {
            return stub->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::CODEC].emplace_back(dumper);
        AVCODEC_LOGD("The number of codec services(%{public}zu).", codecStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AVCODEC_LOGW("failed to call InstanceDump");
        }
    }
    return object;
}
#endif

#ifdef SUPPORT_DEMUXER
sptr<IRemoteObject> AVCodecServerManager::CreateDemuxerStubObject()
{
    if (demuxerStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of demuxer services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", demuxerStubMap_.size());
        return nullptr;
    }
    sptr<DemuxerServiceStub> stub = DemuxerServiceStub::Create();
    if (stub == nullptr) {
        AVCODEC_LOGE("failed to create DemuxerServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = stub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        demuxerStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [stub](int32_t fd) -> int32_t {
            return stub->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::DEMUXER].emplace_back(dumper);
        AVCODEC_LOGD("The number of demuxer services(%{public}zu).", demuxerStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AVCODEC_LOGW("failed to call InstanceDump");
        }
    }
    return object;
}
#endif

#ifdef SUPPORT_MUXER
sptr<IRemoteObject> AVCodecServerManager::CreateMuxerStubObject()
{
    if (muxerStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of muxer services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", muxerStubMap_.size());
        return nullptr;
    }
    sptr<MuxerServiceStub> muxerStub = MuxerServiceStub::Create();
    if (muxerStub == nullptr) {
        AVCODEC_LOGE("Create MuxerServiceStub failed");
        return nullptr;
    }
    sptr<IRemoteObject> object = muxerStub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        muxerStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [muxer = muxerStub](int32_t fd) -> int32_t {
            return muxer->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::MUXER].emplace_back(dumper);
        AVCODEC_LOGD("The number of muxer services(%{public}zu).", muxerStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AVCODEC_LOGW("Failed to call InstanceDump");
        }
    }
    return object;
}
#endif

#ifdef SUPPORT_SOURCE
sptr<IRemoteObject> AVCodecServerManager::CreateSourceStubObject()
{
    if (sourceStubMap_.size() >= SERVER_MAX_NUMBER) {
        AVCODEC_LOGE("The number of source services(%{public}zu) has reached the upper limit."
            "Please release the applied resources.", sourceStubMap_.size());
        return nullptr;
    }
    sptr<SourceServiceStub> stub = SourceServiceStub::Create();
    if (stub == nullptr) {
        AVCODEC_LOGE("failed to create SourceServiceStub");
        return nullptr;
    }
    sptr<IRemoteObject> object = stub->AsObject();
    if (object != nullptr) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        sourceStubMap_[object] = pid;

        Dumper dumper;
        dumper.entry_ = [stub](int32_t fd) -> int32_t {
            return stub->DumpInfo(fd);
        };
        dumper.pid_ = pid;
        dumper.uid_ = IPCSkeleton::GetCallingUid();
        dumper.remoteObject_ = object;
        dumperTbl_[StubType::SOURCE].emplace_back(dumper);
        AVCODEC_LOGD("The number of source services(%{public}zu).", sourceStubMap_.size());
        if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
            AVCODEC_LOGW("failed to call InstanceDump");
        }
    }
    return object;
}
#endif

void AVCodecServerManager::DestroyStubObject(StubType type, sptr<IRemoteObject> object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    DestroyDumper(type, object);
    
    auto compare_func = [object](std::pair<sptr<IRemoteObject>, pid_t> objectPair) ->
        bool { return objectPair.first == object; };
    switch (type) {
        case CODEC: {
            auto it = find_if(codecStubMap_.begin(), codecStubMap_.end(), compare_func);
            if (it != codecStubMap_.end()) {
                AVCODEC_LOGD("destroy codec stub services(%{public}zu) pid(%{public}d).", codecStubMap_.size(), pid);
                (void)codecStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find codec object failed, pid(%{public}d).", pid);
            break;
        }
        case CODECLIST: {
            auto it = find_if(codecListStubMap_.begin(), codecListStubMap_.end(), compare_func);
            if (it != codecListStubMap_.end()) {
                AVCODEC_LOGD("destroy codeclist stub services(%{public}zu) pid(%{public}d).",
                    codecListStubMap_.size(), pid);
                (void)codecListStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find codeclist object failed, pid(%{public}d).", pid);
            break;
        }
        case MUXER: {
            auto it = find_if(muxerStubMap_.begin(), muxerStubMap_.end(), compare_func);
            if (it != muxerStubMap_.end()) {
                AVCODEC_LOGD("destroy muxer stub services(%{public}zu) pid(%{public}d).",
                    muxerStubMap_.size(), pid);
                (void)muxerStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find muxer object failed, pid(%{public}d).", pid);
            break;
        }
        case DEMUXER: {
            auto it = find_if(demuxerStubMap_.begin(), demuxerStubMap_.end(), compare_func);
            if (it != demuxerStubMap_.end()) {
                AVCODEC_LOGD("destroy demuxer stub services(%{public}zu) pid(%{public}d).",
                    demuxerStubMap_.size(), pid);
                (void)demuxerStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find demuxer object failed, pid(%{public}d).", pid);
            break;
        }
        case SOURCE: {
            auto it = find_if(sourceStubMap_.begin(), sourceStubMap_.end(), compare_func);
            if (it != sourceStubMap_.end()) {
                AVCODEC_LOGD("destroy source stub services(%{public}zu) pid(%{public}d).", sourceStubMap_.size(), pid);
                (void)sourceStubMap_.erase(it);
                return;
            }
            AVCODEC_LOGE("find demuxer object failed, pid(%{public}d).", pid);
            break;
        }
        default: {
            AVCODEC_LOGE("default case, av_codec server manager failed, pid(%{public}d).", pid);
            break;
        }
    }
}

void AVCodecServerManager::DestroyStubObjectForPid(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AVCODEC_LOGD("codec stub services(%{public}zu) pid(%{public}d).", codecStubMap_.size(), pid);
    for (auto it = codecStubMap_.begin(); it != codecStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = codecStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("codec stub services(%{public}zu).", codecStubMap_.size());

    AVCODEC_LOGD("codeclist stub services(%{public}zu) pid(%{public}d).", codecListStubMap_.size(), pid);
    for (auto it = codecListStubMap_.begin(); it != codecListStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = codecListStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("codeclist stub services(%{public}zu).", codecListStubMap_.size());

    AVCODEC_LOGD("muxer stub services(%{public}zu) pid(%{public}d).", muxerStubMap_.size(), pid);
    for (auto it = muxerStubMap_.begin(); it != muxerStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = muxerStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("muxer stub services(%{public}zu).", muxerStubMap_.size());

    AVCODEC_LOGD("demuxer stub services(%{public}zu) pid(%{public}d).", demuxerStubMap_.size(), pid);
    for (auto it = demuxerStubMap_.begin(); it != demuxerStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = demuxerStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("demuxer stub services(%{public}zu).", demuxerStubMap_.size());

    AVCODEC_LOGD("source stub services(%{public}zu) pid(%{public}d).", sourceStubMap_.size(), pid);
    for (auto it = sourceStubMap_.begin(); it != sourceStubMap_.end();) {
        if (it->second == pid) {
            executor_.Commit(it->first);
            it = sourceStubMap_.erase(it);
        } else {
            it++;
        }
    }
    AVCODEC_LOGD("source stub services(%{public}zu).", sourceStubMap_.size());

    executor_.Clear();
}

void AVCodecServerManager::DestroyDumper(StubType type, sptr<IRemoteObject> object)
{
    for (auto it = dumperTbl_[type].begin(); it != dumperTbl_[type].end(); it++) {
        if (it->remoteObject_ == object) {
            (void)dumperTbl_[type].erase(it);
            AVCODEC_LOGD("AVCodecServerManager::DestroyDumper");
            if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
                AVCODEC_LOGW("failed to call InstanceDump");
            }
            return;
        }
    }
}

void AVCodecServerManager::DestroyDumperForPid(pid_t pid)
{
    for (auto &dumpers : dumperTbl_) {
        for (auto it = dumpers.second.begin(); it != dumpers.second.end();) {
            if (it->pid_ == pid) {
                it = dumpers.second.erase(it);
                AVCODEC_LOGD("AVCodecServerManager::DestroyDumperForPid");
            } else {
                it++;
            }
        }
    }
    if (Dump(-1, std::vector<std::u16string>()) != OHOS::NO_ERROR) {
        AVCODEC_LOGW("failed to call InstanceDump");
    }
}

void AVCodecServerManager::AsyncExecutor::Commit(sptr<IRemoteObject> obj)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    freeList_.push_back(obj);
}

void AVCodecServerManager::AsyncExecutor::Clear()
{
    std::thread(&AVCodecServerManager::AsyncExecutor::HandleAsyncExecution, this).detach();
}

void AVCodecServerManager::AsyncExecutor::HandleAsyncExecution()
{
    std::list<sptr<IRemoteObject>> tempList;
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        freeList_.swap(tempList);
    }
    tempList.clear();
}
} // namespace Media
} // namespace OHOS
