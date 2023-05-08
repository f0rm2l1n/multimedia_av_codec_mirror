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

#ifndef AVCODEC_SERVER_MANAGER_H
#define AVCODEC_SERVER_MANAGER_H

#include <memory>
#include <functional>
#include <map>
#include <list>
#include "iremote_object.h"
#include "ipc_skeleton.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
using DumperEntry = std::function<int32_t(int32_t)>;
struct Dumper {
    pid_t pid_;
    pid_t uid_;
    DumperEntry entry_;
    sptr<IRemoteObject> remoteObject_;
};

class AVCodecServerManager : public NoCopyable {
public:
    static AVCodecServerManager &GetInstance();
    ~AVCodecServerManager();

    enum StubType {
        CODECLIST,
        CODEC,
        MUXER,
        DEMUXER,
        SOURCE
    };
    sptr<IRemoteObject> CreateStubObject(StubType type);
    void DestroyStubObject(StubType type, sptr<IRemoteObject> object);
    void DestroyStubObjectForPid(pid_t pid);
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args);
    void DestroyDumper(StubType type, sptr<IRemoteObject> object);
    void DestroyDumperForPid(pid_t pid);

private:
    AVCodecServerManager();

#ifdef SUPPORT_MUXER
    sptr<IRemoteObject> CreateMuxerStubObject();
#endif

    class AsyncExecutor {
    public:
        AsyncExecutor() = default;
        virtual ~AsyncExecutor() = default;
        void Commit(sptr<IRemoteObject> obj);
        void Clear();
    private:
        void HandleAsyncExecution();
        std::list<sptr<IRemoteObject>> freeList_;
        std::mutex listMutex_;
    };

    std::map<sptr<IRemoteObject>, pid_t> demuxerStubMap_;
    std::map<sptr<IRemoteObject>, pid_t> muxerStubMap_;
    std::map<sptr<IRemoteObject>, pid_t> codecStubMap_;
    std::map<sptr<IRemoteObject>, pid_t> codecListStubMap_;
    std::map<sptr<IRemoteObject>, pid_t> sourceStubMap_;
    std::map<StubType, std::vector<Dumper>> dumperTbl_;
    AsyncExecutor executor_;

    std::mutex mutex_;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_SERVER_MANAGER_H