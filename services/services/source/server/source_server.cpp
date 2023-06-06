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

#include "source_server.h"
#include <unistd.h>
#include <string>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_dump_utils.h"
#include "avcodec_dfx.h"
#include "ipc_skeleton.h"
#include "avcodec_common.h"
#include "media_description.h"
#include "native_avcodec_base.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServer"};
    constexpr uint32_t DUMP_SOURCE_INFO_INDEX = 0x01010000;
    constexpr uint32_t DUMP_TRACK_INFO_INDEX = 0x01020000;
    constexpr uint32_t DUMP_OFFSET_8 = 8;

    const std::vector<std::pair<std::string_view, const std::string>> SOURCE_DUMP_TABLE = {
        { OHOS::Media::AVSourceFormat::SOURCE_TITLE, "Title" },
        { OHOS::Media::AVSourceFormat::SOURCE_ARTIST, "Artist" },
        { OHOS::Media::AVSourceFormat::SOURCE_ALBUM, "Album" },
        { OHOS::Media::AVSourceFormat::SOURCE_ALBUM_ARTIST, "Album_Artist" },
        { OHOS::Media::AVSourceFormat::SOURCE_DATE, "Date" },
        { OHOS::Media::AVSourceFormat::SOURCE_COMMENT, "Comment" },
        { OHOS::Media::AVSourceFormat::SOURCE_GENRE, "Genre" },
        { OHOS::Media::AVSourceFormat::SOURCE_COPYRIGHT, "Copyright" },
        { OHOS::Media::AVSourceFormat::SOURCE_LANGUAGE, "Language" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_DURATION, "Duration" },
        { OHOS::Media::AVSourceFormat::SOURCE_DESCRIPTION, "Description" },
        { OHOS::Media::AVSourceFormat::SOURCE_LYRICS, "Lyrics" },
    };

    const std::vector<std::pair<std::string_view, const std::string>> AUDIO_TRACK_DUMP_TABLE = {
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_MIME, "Codec_Mime" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_SAMPLE_RATE, "Sample_Rate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, "Channel_Count" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_BITRATE, "Bitrate" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, "AAC_Is_ADTS" },
    };

    const std::vector<std::pair<std::string_view, const std::string>> VIDEO_TRACK_DUMP_TABLE = {
        { OHOS::Media::MediaDescriptionKey::MD_KEY_CODEC_MIME, "Codec_Mime" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_WIDTH, "Width" },
        { OHOS::Media::MediaDescriptionKey::MD_KEY_HEIGHT, "Height" },
    };
}

namespace OHOS {
namespace Media {
std::shared_ptr<ISourceService> SourceServer::Create()
{
    std::shared_ptr<SourceServer> server = std::make_shared<SourceServer>();
    CHECK_AND_RETURN_RET_LOG(server != nullptr, nullptr, "Source Service does not exist");
    return server;
}

SourceServer::SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    appUid_ = IPCSkeleton::GetCallingUid();
    appPid_ = IPCSkeleton::GetCallingPid();
}

SourceServer::~SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    sourceEngine_ = nullptr;
    if (fd_ >= 0) {
        (void)::close(fd_);
        fd_ = -1;
    }
}

int32_t SourceServer::InitWithURI(const std::string &uri)
{
    sourceEngine_ = ISourceEngineFactory::CreateSourceEngine(appUid_, appPid_, uri);
    uri_ = uri;
    int32_t ret = sourceEngine_->Create();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call InitWithURI");
    return AVCS_ERR_OK;
}

int32_t SourceServer::InitWithFD(int32_t fd, int64_t offset, int64_t size)
{
    std::string uri = "fd://" + std::to_string(fd) + "?offset=" + \
        std::to_string(offset) + "&size=" + std::to_string(size);

    sourceEngine_ = ISourceEngineFactory::CreateSourceEngine(appUid_, appPid_, uri);
    uri_ = uri;
    fd_ = fd;
    int32_t ret = sourceEngine_->Create();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call InitWithFD");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackCount(uint32_t &trackCount)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Source engine does not exist");
    int32_t ret = sourceEngine_->GetTrackCount(trackCount);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call GetTrackCount");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Source engine does not exist");
    int32_t ret = sourceEngine_->GetTrackFormat(format, trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call GetTrackFormat");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetSourceFormat(Format &format)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Source engine does not exist");
    int32_t ret = sourceEngine_->GetSourceFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call GetSourceFormat");
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetSourceAddr(uintptr_t &addr)
{
    CHECK_AND_RETURN_RET_LOG(sourceEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Source engine does not exist");
    addr = sourceEngine_->GetSourceAddr();
    CHECK_AND_RETURN_RET_LOG(addr != 0, AVCS_ERR_INVALID_VAL, "Failed to call GetSourceAddr");
    return AVCS_ERR_OK;
}

int32_t SourceServer::DumpInfo(int32_t fd)
{
    CHECK_AND_RETURN_RET_LOG(fd != -1, AVCS_ERR_INVALID_VAL, "Attempt to write to a invalid fd: %{public}d", fd);
    std::string dumpInfo;
    GetDumpInfo(dumpInfo);

    write(fd, dumpInfo.c_str(), dumpInfo.size());
    return AVCS_ERR_OK;
}

int32_t SourceServer::GetDumpInfo(std::string &dumpInfo)
{
    Format sourceFormat, trackFormat;
    uint32_t trackCount = 0;
    int32_t ret = this->GetTrackCount(trackCount);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,
        AVCS_ERR_INVALID_OPERATION, "Get track count failed!");
    ret = GetSourceFormat(sourceFormat);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,
        AVCS_ERR_INVALID_OPERATION, "Get track format failed!");
    AVCodecDumpControler dumpControler;

    int32_t sourceDumpIndex = 1;
    dumpControler.AddInfo(DUMP_SOURCE_INFO_INDEX, "Source_Info");
    for (auto iter : SOURCE_DUMP_TABLE) {
        dumpControler.AddInfoFromFormat(DUMP_SOURCE_INFO_INDEX + (sourceDumpIndex << DUMP_OFFSET_8),
            sourceFormat, iter.first, iter.second);
        sourceDumpIndex++;
    }

    dumpControler.AddInfo(DUMP_TRACK_INFO_INDEX, "Track_Info");
    for (uint32_t idx = 0; idx < trackCount; idx++) {
        ret = GetTrackFormat(trackFormat, idx);
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION, "Get track format failed!");
        int32_t trackDumpIndex = 1;
        int32_t trackListIndex = (idx + 1) << DUMP_OFFSET_8;
        int32_t trackType;
        trackFormat.GetIntValue("track_type", trackType);
        std::string indexString =
            std::string("Index_") + std::to_string(idx) + std::string("_") + (trackType ? "Video" : "Audio");
        dumpControler.AddInfo(DUMP_TRACK_INFO_INDEX + trackListIndex, indexString);
        const auto &dumpTable = trackType == MEDIA_TYPE_AUD ? AUDIO_TRACK_DUMP_TABLE : VIDEO_TRACK_DUMP_TABLE;
        for (auto iter : dumpTable) {
            dumpControler.AddInfoFromFormat(DUMP_TRACK_INFO_INDEX + trackListIndex + trackDumpIndex,
                trackFormat, iter.first, iter.second);
            trackDumpIndex++;
        }
    }
    
    ret = dumpControler.GetDumpString(dumpInfo);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,
        AVCS_ERR_INVALID_OPERATION, "Get dump string failed!");
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS