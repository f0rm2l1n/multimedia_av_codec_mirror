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

#include "avsharedmemorybase.h"
#include <sys/mman.h>
#include <unistd.h>
#include "ashmem.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "scope_guard.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVSharedMemoryBase"};
}

namespace OHOS {
namespace Media {
struct AVSharedMemoryBaseImpl : public AVSharedMemoryBase {
public:
    AVSharedMemoryBaseImpl(int32_t fd, int32_t size, uint32_t flags, const std::string &name)
        : AVSharedMemoryBase(fd, size, flags, name) {}
};

std::shared_ptr<AVSharedMemory> AVSharedMemoryBase::CreateFromLocal(
    int32_t size, uint32_t flags, const std::string &name)
{
    std::shared_ptr<AVSharedMemoryBase> memory = std::make_shared<AVSharedMemoryBase>(size, flags, name);
    int32_t ret = memory->Init();
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("Create avsharedmemory failed, ret = %{public}d", ret);
        return nullptr;
    }

    return memory;
}

std::shared_ptr<AVSharedMemory> AVSharedMemoryBase::CreateFromRemote(
    int32_t fd, int32_t size, uint32_t flags, const std::string &name)
{
    std::shared_ptr<AVSharedMemoryBase> memory = std::make_shared<AVSharedMemoryBaseImpl>(fd, size, flags, name);
    int32_t ret = memory->Init();
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("Create avsharedmemory failed, ret = %{public}d", ret);
        return nullptr;
    }

    return memory;
}

AVSharedMemoryBase::AVSharedMemoryBase(int32_t size, uint32_t flags, const std::string &name)
    : base_(nullptr), size_(size), flags_(flags), name_(name), fd_(-1)
{
    AVCODEC_LOGD("enter ctor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryBase::AVSharedMemoryBase(int32_t fd, int32_t size, uint32_t flags, const std::string &name)
    : base_(nullptr), size_(size), flags_(flags), name_(name), fd_(dup(fd))
{
    AVCODEC_LOGD("enter ctor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryBase::~AVSharedMemoryBase()
{
    AVCODEC_LOGD("enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
    Close();
}

int32_t AVSharedMemoryBase::Init()
{
    ON_SCOPE_EXIT(0) {
        AVCODEC_LOGE("create avsharedmemory failed, name = %{public}s, size = %{public}d, "
                     "flags = 0x%{public}x, fd = %{public}d",
                     name_.c_str(), size_, flags_, fd_);
        Close();
    };

    CHECK_AND_RETURN_RET_LOG(size_ > 0, AVCS_ERR_INVALID_VAL, "size is invalid, size = %{public}d", size_);

    bool isRemote = false;
    if (fd_ > 0) {
        int size = AshmemGetSize(fd_);
        CHECK_AND_RETURN_RET_LOG(size == size_, AVCS_ERR_INVALID_VAL, "size not equal size_, size = %{public}d, "
                                "size_ = %{public}d", size, size_);
        isRemote = true;
    } else {
        fd_ = AshmemCreate(name_.c_str(), static_cast<size_t>(size_));
        CHECK_AND_RETURN_RET_LOG(fd_ > 0, AVCS_ERR_INVALID_VAL, "fd is invalid, fd = %{public}d", fd_);
    }

    int32_t ret = MapMemory(isRemote);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_VAL, "MapMemory failed, ret = %{plublic}d", ret);

    CANCEL_SCOPE_EXIT_GUARD(0);
    return AVCS_ERR_OK;
}

int32_t AVSharedMemoryBase::MapMemory(bool isRemote)
{
    unsigned int prot = PROT_READ | PROT_WRITE;
    if (isRemote && (flags_ & FLAGS_READ_ONLY)) {
        prot &= ~PROT_WRITE;
    }

    int result = AshmemSetProt(fd_, static_cast<int>(prot));
    CHECK_AND_RETURN_RET_LOG(result >= 0, AVCS_ERR_INVALID_OPERATION,
        "AshmemSetProt failed, result = %{public}d", result);

    void *addr = ::mmap(nullptr, static_cast<size_t>(size_), static_cast<int>(prot), MAP_SHARED, fd_, 0);
    CHECK_AND_RETURN_RET_LOG(addr != MAP_FAILED, AVCS_ERR_INVALID_OPERATION, "mmap failed, please check params");

    base_ = reinterpret_cast<uint8_t*>(addr);
    return AVCS_ERR_OK;
}

void AVSharedMemoryBase::Close() noexcept
{
    if (base_ != nullptr) {
        (void)::munmap(base_, static_cast<size_t>(size_));
        base_ = nullptr;
        size_ = 0;
        flags_ = 0;
    }

    if (fd_ > 0) {
        (void)::close(fd_);
        fd_ = -1;
    }
}
} // namespace Media
} // namespace OHOS
