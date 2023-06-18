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

#ifndef FILE_SERVER_MOCK_H
#define FILE_SERVER_MOCK_H

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace OHOS {
namespace MediaAVCodec {
class FileServerMock {
public:
    explicit FileServerMock();
    virtual ~FileServerMock();
    void StartServer();
    void StopServer();

private:
    static void FileLoopFunc(int32_t connFd);
    void ServerLoopFunc();
    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::thread> serverLoop_ = nullptr;
    std::unique_ptr<std::thread> fileLoop_ = nullptr;
    int32_t listenFd_ = 0;
};
} // namespace MediaAVCodec
} // namespace OHOS

#endif