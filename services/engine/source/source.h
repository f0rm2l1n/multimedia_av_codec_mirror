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

#ifndef SOURCE_H
#define SOURCE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#ifdef __cplusplus
}
#endif

#include <thread>
#include "avcodec_errors.h"
#include "source_plugin.h"
#include "plugin_types.h"
#include "plugin_buffer.h"
#include "plugin_definition.h"
#include "sourcebase.h"

namespace OHOS {
namespace Media {
namespace Plugin {
struct FfmpegRegister : PackageRegister {
public:
    std::string name = "custom register";
    Status AddPlugin(const PluginDefBase& def) override;
    Status AddPackage(const PackageDef& def) override;
    std::shared_ptr<SourcePlugin> sourcePlugin {nullptr};
private:
    std::shared_ptr<PackageDef> packageDef;
};

class Source : public SourceBase {
public:
    Source();
    ~Source();

    int32_t Create(std::string& uri) override;
    int32_t GetTrackCount(uint32_t &trackCount) override;
    int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) override;
    int32_t GetSourceFormat(Format &format) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackIndex) override;
    uintptr_t GetSourceAddr() override;
    std::shared_ptr<SourcePlugin> GetSourcePlugin()
    {
        return sourcePlugin_;
    }

private:
    struct CustomIOContext {
        SourcePlugin* sourcePlugin = nullptr;
        size_t offset = 0;
        size_t position = 0;
        bool eof = false;
        size_t fileSize = 0;
        AVIOContext* avioContext = nullptr;
        std::shared_ptr<Buffer> bufMemory;
    };

    std::shared_ptr<AVFormatContext> formatContext_;
    std::map<uint32_t, AVDictionary*> trackParam_;
    std::shared_ptr<AVInputFormat> inputFormat_;
    std::shared_ptr<SourcePlugin> sourcePlugin_;
    std::shared_ptr<FfmpegRegister> register_;
    CustomIOContext customIOContext_;
    AVIOContext* avioContext_ = nullptr;
    int32_t LoadDemuxerList();
    int32_t LoadDynamicPlugin(const std::string& path);
    int32_t GuessInputFormat(const std::string& uri,  std::shared_ptr<AVInputFormat>& bestInputFormat);
    int32_t SniffInputFormat(const std::string& uri);
    static int AVReadPacket(void* opaque, uint8_t* buf, int bufSize);
    static int64_t AVSeek(void* opaque, int64_t offset, int whence);
    void InitAVIOContext(int flags);
    int32_t InitAVFormatContext();
    void GetStringFormatFromMetadata(std::string key, std::string_view formatName, Format &format);
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // AVSOURCE_H