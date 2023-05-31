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
#include<iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
#include <malloc.h>
#include <string>

#include "native_avcodec_base.h"
#include "native_avformat.h"
#include "native_avmagic.h"
#include "native_avmemory.h"
#include "media_description.h"
#include "avsharedmemorybase.h"
#include "avcodec_common.h"

#include "inner_demo/inner_source_demo.h"
#include "inner_demo/inner_demuxer_demo.h"
#include "capi_demo/avsource_demo.h"
#include "capi_demo/avdemuxer_demo.h"

#include "avdemuxer_demo_runner.h"

using namespace std;
using namespace OHOS::Media;

static int64_t g_seekTime = 1000;
static int64_t g_startTime = 0;

static void RunNativeDemuxer(const std::string filePath, const std::string fileMode)
{
    auto avSourceDemo = std::make_shared<AVSourceDemo>();
    if (fileMode == "0") {
        int32_t fd = open(filePath.c_str(), O_RDONLY);
        size_t filesize = avSourceDemo->GetFileSize(filePath);
        avSourceDemo->CreateWithFD(fd, 0, filesize);
    }
    if (fileMode == "1") {
        avSourceDemo->CreateWithURI((char*)(filePath.c_str()));
    }
    auto avDemuxerDemo = std::make_shared<AVDemuxerDemo>();
    OH_AVSource* av_source = avSourceDemo->GetAVSource();
    avDemuxerDemo->CreateWithSource(av_source);
    int32_t trackCount = 0;
    int64_t duration = 0;
    OH_AVFormat* oh_avformat = avSourceDemo->GetSourceFormat();
    // 北向获取sourceformat
    OH_AVFormat_GetIntValue(oh_avformat, OH_MD_KEY_TRACK_COUNT, &trackCount);
    OH_AVFormat_GetLongValue(oh_avformat, OH_MD_KEY_DURATION, &duration);
    printf("====>total tracks:%d\n", trackCount);
    printf("====>duration:%" PRId64 "\n", duration);
    // 添加轨道
    for (int32_t i = 0;i < trackCount; i++) {
        avDemuxerDemo->SelectTrackByID(i);
    }
    // 去掉轨道
    avDemuxerDemo->UnselectTrackByID(1);
    avDemuxerDemo->SelectTrackByID(1);
    // 创建memory
    uint32_t buffersize = 1024 * 1024;
    OH_AVMemory* sampleMem = OH_AVMemory_Create(buffersize);
    // demuxer run
    avDemuxerDemo->ReadAllSamples(sampleMem, trackCount);
    // 测试seek功能
    printf("seek to 1s,mode:SEEK_MODE_NEXT_SYNC\n");
    avDemuxerDemo->SeekToTime(g_seekTime, OH_AVSeekMode::SEEK_MODE_NEXT_SYNC);
    avDemuxerDemo->ReadAllSamples(sampleMem, trackCount);
    printf("seek to 1s,mode:SEEK_MODE_PREVIOUS_SYNC\n");
    avDemuxerDemo->SeekToTime(g_seekTime, OH_AVSeekMode::SEEK_MODE_PREVIOUS_SYNC);
    avDemuxerDemo->ReadAllSamples(sampleMem, trackCount);
    printf("seek to 1s,mode:SEEK_MODE_CLOSEST_SYNC\n");
    avDemuxerDemo->SeekToTime(g_seekTime, OH_AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
    avDemuxerDemo->ReadAllSamples(sampleMem, trackCount);
    printf("seek to 0s,mode:SEEK_MODE_CLOSEST_SYNC\n");
    avDemuxerDemo->SeekToTime(g_startTime, OH_AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
    avDemuxerDemo->ReadAllSamples(sampleMem, trackCount);
    OH_AVMemory_Destroy(sampleMem);
    avDemuxerDemo->Destroy();
    avSourceDemo->Destroy();
}

static void RunInnerSourceDemuxer(const std::string filePath, const std::string fileMode)
{
    auto innerSourceDemo = std::make_shared<InnerSourceDemo>();
    if (fileMode == "0") {
        int32_t fd = open(filePath.c_str(), O_RDONLY);
        size_t filesize = innerSourceDemo->GetFileSize(filePath);
        innerSourceDemo->CreateWithFD(fd, 0, filesize);
    }
    if (fileMode == "1") {
        innerSourceDemo->CreateWithURI(filePath);
    }
    auto innerDemuxerDemo = std::make_shared<InnerDemuxerDemo>();
    innerDemuxerDemo->CreateWithSource(*(innerSourceDemo->avsource_));
    int32_t trackCount = 0;
    int64_t duration = 0;
    Format source_format = innerSourceDemo->GetSourceFormat();
    source_format.GetIntValue(MediaDescriptionKey::MD_KEY_TRACK_COUNT, trackCount);
    source_format.GetLongValue(MediaDescriptionKey::MD_KEY_DURATION, duration);
    printf("====>duration:%" PRId64 "\n", duration);
    printf("====>total tracks:%d\n", trackCount);
    // 添加轨道
    for (int32_t i = 0;i < trackCount; i++) {
        innerDemuxerDemo->SelectTrackByID(i);
    }
    // 去掉轨道
    innerDemuxerDemo->UnselectTrackByID(0);
    innerDemuxerDemo->SelectTrackByID(0);
    uint32_t buffersize = 1024 * 1024;
    std::shared_ptr<AVSharedMemoryBase> sharedMemory = std::make_shared<AVSharedMemoryBase>(buffersize,
     AVSharedMemory::FLAGS_READ_WRITE, "userBuffer");
    sharedMemory->Init();
    // demuxer run
    innerDemuxerDemo->ReadAllSamples(sharedMemory, trackCount);
    // 测试seek功能
    printf("seek to 1s,mode:SEEK_MODE_NEXT_SYNC\n");
    innerDemuxerDemo->SeekToTime(g_seekTime, AVSeekMode::SEEK_MODE_NEXT_SYNC);
    innerDemuxerDemo->ReadAllSamples(sharedMemory, trackCount);
    printf("seek to 1s,mode:SEEK_MODE_PREVIOUS_SYNC\n");
    innerDemuxerDemo->SeekToTime(g_seekTime, AVSeekMode::SEEK_MODE_PREVIOUS_SYNC);
    innerDemuxerDemo->ReadAllSamples(sharedMemory, trackCount);
    printf("seek to 1s,mode:SEEK_MODE_CLOSEST_SYNC\n");
    innerDemuxerDemo->SeekToTime(g_seekTime, AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
    innerDemuxerDemo->ReadAllSamples(sharedMemory, trackCount);
    printf("seek to 0s,mode:SEEK_MODE_CLOSEST_SYNC\n");
    innerDemuxerDemo->SeekToTime(g_startTime, AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
    innerDemuxerDemo->ReadAllSamples(sharedMemory, trackCount);
    innerDemuxerDemo->Destroy();
}

void AVSourceDemuxerDemoCase(void)
{
    cout << "Please select a demuxer demo(default native demuxer demo): " << endl;
    cout << "0:native_demuxer" << endl;
    cout << "1:ffmpeg_demuxer" << endl;
    string mode;
    string fileMode;
    string filePath;
    (void)getline(cin, mode);
    cout << "Please select file path (0) or uri (1)" << endl;
    (void)getline(cin, fileMode);
     cout << "Please input file path  or uri:" << endl;
    (void)getline(cin, filePath);
    if (mode == "0" || mode == "") {
        RunNativeDemuxer(filePath, fileMode);
    } else if (mode =="1") {
        RunInnerSourceDemuxer(filePath, fileMode);
    } else {
        printf("select 0 or 1\n");
    }
}