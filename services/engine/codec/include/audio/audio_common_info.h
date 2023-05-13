/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef AV_CODEC_AUDIO_COMMON_INFO_H
#define AV_CODEC_AUDIO_COMMON_INFO_H

namespace OHOS {
namespace Media {

enum class BufferStatus {
    IDEL,
    OWNE_BY_CLIENT,
};

enum class CodecState {
    RELEASED,
    INITLIZED,
    FLUSHED,
    RUNNING,

    INITLIZING, // RELEASED -> INITLIZED
    STARTING,   // INITLIZED -> RUNNING
    STOPPING,   // RUNNING -> INITLIZED
    FLUSHING,   // RUNNING -> FLUSHED
    RESUMING,   // FLUSHED -> RUNNING
    RRELEASING, // {ANY EXCEPT RELEASED} -> RELEASED
};

} // namespace Media
} // namespace OHOS

#endif