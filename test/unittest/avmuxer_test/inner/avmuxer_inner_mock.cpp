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

#include "avmuxer_inner_mock.h"

namespace OHOS {
namespace Media {

int32_t MuxerInnerMock::Destroy()
{
    if (muxer_ != nullptr) {
        muxer_ = nullptr;
        return AV_ERR_OK;
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerInnerMock::Start()
{
    if (muxer_ != nullptr) {
        return muxer_->Start();
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerInnerMock::Stop()
{
    if (muxer_ != nullptr) {
        return muxer_->Stop();
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerInnerMock::AddTrack(int32_t &trackIndex, std::shared_ptr<FormatMock> &trackFormat)
{
    if (muxer_ != nullptr) {
        auto formatMock = std::static_pointer_cast<AVFormatInnerMock>(trackFormat);
        return muxer_->AddTrack(trackIndex, static_cast<MediaDescription>(formatMock->GetFormat()));
    }
    return AV_ERR_UNKNOWN;
}
                         
int32_t MuxerInnerMock::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, const AVCodecBufferAttrMock &info)
{
    if (muxer_ != nullptr) {
        TrackSampleInfo sampleInfo;
        sampleInfo.trackIndex = trackIndex;
        sampleInfo.timeUs = info.pts;
        sampleInfo.size = info.size;
        sampleInfo.flags = info.flags;
        return muxer_->WriteSampleBuffer(sampleBuffer, sampleInfo);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerInnerMock::SetLocation(float latitude, float longitude)
{
    if (muxer_ != nullptr) {
        return muxer_->SetLocation(latitude, longitude);
    }
    return AV_ERR_UNKNOWN;
}

int32_t MuxerInnerMock::SetRotation(int32_t rotation)
{
    if (muxer_ != nullptr) {
        return muxer_->SetRotation(rotation);
    }
    return AV_ERR_UNKNOWN;
}
} // namespace Media
} // namespace OHOS