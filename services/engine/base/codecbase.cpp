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

#include "avcodec_log.h"
#include "codecbase.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecBase"};
}

namespace OHOS {
namespace Media {

int32_t CodecBase::NotifyEos()
{
    AVCODEC_LOGW("NotifyEos is not supported");
    return 0;
}

sptr<Surface> CodecBase::CreateInputSurface()
{
    AVCODEC_LOGW("CreateInputSurface is not supported");
    return nullptr;
}

int32_t CodecBase::SetOutputSurface(sptr<Surface> surface)
{
    (void)surface;
    AVCODEC_LOGW("SetOutputSurface is not supported");
    return 0;
}

int32_t CodecBase::RenderOutputBuffer(size_t index)
{
    (void)index;
    AVCODEC_LOGW("RenderOutputBuffer is not supported");
    return 0;
}

int32_t CodecBase::SignalRequestIDRFrame()
{
    AVCODEC_LOGW("SignalRequestIDRFrame is not supported");
    return 0;
}

} // namespace Media
} // namespace OHOS
