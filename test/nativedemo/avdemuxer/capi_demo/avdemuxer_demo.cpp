
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

#include "avdemuxer_demo.h"

namespace OHOS {
namespace Media {
AVDemuxerDemo::AVDemuxerDemo()
{
    printf("AVDemuxerDemo constructor \n");
}

AVDemuxerDemo::~AVDemuxerDemo()
{
    printf("AVDemuxerDemo deconstructor \n");
}

int32_t AVDemuxerDemo::Destroy()
{
    int32_t ret = static_cast<int32_t>(OH_AVDemuxer_Destroy(avdemxuer_));
    if (ret != 0) {
        printf("OH_AVDemuxer_Destroy is failed\n");
    }
    return -1;
}

int32_t AVDemuxerDemo::CreateWithSource(OH_AVSource *avsource)
{
    this->avsource_ = avsource;
    if (!this->avsource_) {
        printf("this avsource is nullptr\n");
        return -1;
    }
    avdemxuer_ = OH_AVDemuxer_CreateWithSource(avsource);
    if (!avdemxuer_) {
        printf("OH_AVDemuxer_CreateWithSource is failed\n");
        return -1;
    }
    return 0;
}

int32_t AVDemuxerDemo::SelectTrackByID(uint32_t trackIndex)
{
    int32_t ret = static_cast<int32_t>(OH_AVDemuxer_SelectTrackByID(this->avdemxuer_, trackIndex));
    if (ret != 0) {
            printf("OH_AVDemuxer_SelectTrackByID is faild \n");
    }
    return ret;
}

int32_t AVDemuxerDemo::UnselectTrackByID(uint32_t trackIndex)
{
    int32_t ret = OH_AVDemuxer_UnselectTrackByID(this->avdemxuer_, trackIndex);
    if (ret != 0) {
        printf("OH_AVDemuxer_UnselectTrackByID is faild \n");
    }
    return ret;
}

int32_t AVDemuxerDemo::PrintInfo(int32_t tracks)
{
    for (int32_t i = 0; i < tracks; i++) {
        printf("streams[%d]==>total frames=%lld,KeyFrames=%lld\n", i,
            frames_[i], key_frames_[i]);
    }
    return 0;
}

int32_t AVDemuxerDemo::ReadSample(uint32_t trackIndex, OH_AVMemory *sample, OH_AVCodecBufferAttr *bufferAttr)
{
    int32_t ret = OH_AVDemuxer_ReadSample(this->avdemxuer_, trackIndex, sample, bufferAttr);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int32_t AVDemuxerDemo::ReadAllSamples(OH_AVMemory *sample, int32_t tracks)
{
    int32_t ret = -1;
    bool isEnd = false;
    for (int i = 0;i < tracks; i++) {
        frames_[i] = 0;
        key_frames_[i] = 0;
    }
    while (!isEnd) {
        for (int32_t i = 0; i < tracks; i++) {
            ret = ReadSample(i, sample, &bufferInfo);
            if (ret != 0) {
                printf("read finished\n");
                PrintInfo(tracks);
                isEnd = true;
            }
            if (bufferInfo.flags == AVCODEC_BUFFER_FLAGS_EOS) {
                frames_[i]++;
                printf("streams[%d]==>bufferInfo:pts=%lld,size=%d,offset=%d\n",
                    i, bufferInfo.pts, bufferInfo.size, bufferInfo.offset);
            }
            if (bufferInfo.flags == AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                frames_[i]++;
                key_frames_[i]++;
            }
        }
    }
    return ret;
}

int32_t AVDemuxerDemo::SeekToTime(int64_t mSeconds, OH_AVSeekMode mode)
{
    int32_t ret = OH_AVDemuxer_SeekToTime(this->avdemxuer_, mSeconds, mode);
    if (ret != 0) {
        printf("OH_AVDemuxer_CopyNextSample is faild \n");
    }
    return ret;
}
}  // namespace Media
}  // namespace OHOS