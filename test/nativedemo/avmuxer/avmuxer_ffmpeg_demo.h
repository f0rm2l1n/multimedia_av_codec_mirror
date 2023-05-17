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

#ifndef AVMUXER_FFMPEG_DEMO_H
#define AVMUXER_FFMPEG_DEMO_H

#include <vector>
#include "avmuxer.h"
#include "muxer_plugin.h"
#include "avmuxer_demo_base.h"
#include "plugin_definition.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class AVMuxerFFmpegDemo : public AVMuxerDemoBase {
public:
    AVMuxerFFmpegDemo();
    ~AVMuxerFFmpegDemo() = default;
private:
    struct FfmpegRegister : PackageRegister {
        Status AddPlugin(const PluginDefBase& def) override;
        Status AddPackage(const PackageDef& def) override
        {
            (void)def;
            return Status::NO_ERROR;
        }
        std::vector<MuxerPluginDef> plugins;
    };

    void DoRunMuxer() override;
    int GetFfmpegRegister();
    int DoWriteSample(std::shared_ptr<AVSharedMemory> sample, TrackSampleInfo &info) override;
    int DoAddTrack(int32_t &trackIndex, MediaDescription &trackDesc) override;
    void DoRunMultiThreadCase() override;

    std::shared_ptr<MuxerPlugin> ffmpegMuxer_ {nullptr};
    std::shared_ptr<FfmpegRegister> register_ {nullptr};
    void *dlHandle_ {nullptr};
    RegisterFunc registerFunc_ {nullptr};
    UnregisterFunc unregisterFunc_ {nullptr};
};
}  // Plugin
}  // namespace Media
}  // namespace OHOS
#endif  // AVMUXER_FFMPEG_DEMO_H