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

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "securec.h"
#include "native_avcodec_base.h"
#include "native_averrors.h"
#include "native_avformat.h"
#include "native_avmuxer.h"
#include "avmuxer_demo_common.h"


#define NORMAL 0
#define THREAD 1

struct WriteTrackSampleParam {
    OH_AVMuxer *muxer;
    int trackId;
    int fd;
};

struct MuxerParam {
    int outputFormat;
    char outputFormatType[20];
    int runMode;
    char runModeType[20];
    AudioTrackParam *audioParams;
    char audioType[20];
    VideoTrackParam *videoParams;
    char videoType[20];
    VideoTrackParam *coverParams;
    char coverType[20];
};

extern char *RUN_NORMAL;
extern char *RUN_MUL_THREAD;
extern AudioTrackParam g_audioMpegPar;
extern AudioTrackParam g_audioAacPar;
extern VideoTrackParam g_videoH264Par;
extern VideoTrackParam g_videoMpeg4Par;
extern VideoTrackParam g_jpegCoverPar;
extern VideoTrackParam g_pngCoverPar;
extern VideoTrackParam g_bmpCoverPar;

static struct MuxerParam g_muxerParam  = {
    .outputFormat = AV_OUTPUT_FORMAT_DEFAULT,
    .outputFormatType = "",
    .runMode = NORMAL,
    .runModeType = "",
    .audioParams = NULL,
    .audioType = "",
    .videoParams = NULL,
    .videoType = "",
    .coverParams = NULL,
    .coverType = "",
};

int AddTrackAudio(OH_AVMuxer *muxer, const AudioTrackParam *param, int fdInput)
{
    if (fdInput < 0) {
        printf("unselect audio, fd is %d\n", fdInput);
        return -1;
    }
    OH_AVFormat *formatAudio = OH_AVFormat_Create();
    if (formatAudio == NULL) {
        printf("audio format failed!\n");
        return AV_ERR_NO_MEMORY;
    }
    int extraSize = 0;
    unsigned char buffer[100] = {0};
    read(fdInput, (void*)&extraSize, sizeof(extraSize));
    if (extraSize <= 100 && extraSize > 0) {
        read(fdInput, buffer, extraSize);
        OH_AVFormat_SetBuffer(formatAudio, OH_MD_KEY_CODEC_CONFIG, buffer, extraSize);
    }
    printf("AddTrackAudio audio metadata size: %d\n", extraSize);
    OH_AVFormat_SetStringValue(formatAudio, OH_MD_KEY_CODEC_MIME, param->mimeType);
    OH_AVFormat_SetIntValue(formatAudio, OH_MD_KEY_AUD_SAMPLE_RATE, param->sampleRate);
    OH_AVFormat_SetIntValue(formatAudio, OH_MD_KEY_AUD_CHANNEL_COUNT, param->channels);
    int trackIndex = -1;
    int ret = OH_AVMuxer_AddTrack(muxer, &trackIndex, formatAudio);
    OH_AVFormat_Destroy(formatAudio);
    if (ret != AV_ERR_OK) {
        printf("AddTrackAudio failed! mime: %s\n", param->mimeType);
        return -1;
    }
    printf("AddTrackAudio success! trackIndex: %d\n", trackIndex);
    return trackIndex;
}

int AddTrackVideo(OH_AVMuxer *muxer, const VideoTrackParam *param, int fdInput)
{
    if (fdInput < 0) {
        printf("unselect video, fd is %d\n", fdInput);
        return -1;
    }
    OH_AVFormat *formatVideo = OH_AVFormat_Create();
    if (formatVideo == NULL) {
        printf("video format failed!\n");
        return AV_ERR_NO_MEMORY;
    }
    int extraSize = 0;
    unsigned char buffer[100] = {0};
    read(fdInput, (void*)&extraSize, sizeof(extraSize));
    if (extraSize <= 100 && extraSize > 0) {
        read(fdInput, buffer, extraSize);
        OH_AVFormat_SetBuffer(formatVideo, OH_MD_KEY_CODEC_CONFIG, buffer, extraSize);
    }
    printf("AddTrackVideo video metadata size: %d\n", extraSize);
    OH_AVFormat_SetStringValue(formatVideo, OH_MD_KEY_CODEC_MIME, param->mimeType);
    OH_AVFormat_SetIntValue(formatVideo, OH_MD_KEY_WIDTH, param->width);
    OH_AVFormat_SetIntValue(formatVideo, OH_MD_KEY_HEIGHT, param->height);
    int trackIndex = -1;
    int ret = OH_AVMuxer_AddTrack(muxer, &trackIndex, formatVideo);
    OH_AVFormat_Destroy(formatVideo);
    if (ret != AV_ERR_OK) {
        printf("AddTrackVideo failed! mime: %s\n", param->mimeType);
        return -1;
    }
    printf("AddTrackVideo success! trackIndex: %d\n", trackIndex);
    return trackIndex;
}

int AddTrackCover(OH_AVMuxer *muxer, const VideoTrackParam *param, int fdInput)
{
    if (fdInput < 0) {
        printf("unselect cover, fd is %d\n", fdInput);
        return -1;
    }

    OH_AVFormat *formatCover = OH_AVFormat_Create();
    if (formatCover == NULL) {
        printf("cover format failed!\n");
        return AV_ERR_NO_MEMORY;
    }
    OH_AVFormat_SetStringValue(formatCover, OH_MD_KEY_CODEC_MIME, param->mimeType);
    OH_AVFormat_SetIntValue(formatCover, OH_MD_KEY_WIDTH, param->width);
    OH_AVFormat_SetIntValue(formatCover, OH_MD_KEY_HEIGHT, param->height);
    int trackIndex = -1;
    int ret = OH_AVMuxer_AddTrack(muxer, &trackIndex, formatCover);
    OH_AVFormat_Destroy(formatCover);
    if (ret != AV_ERR_OK) {
        printf("AddTrackCover failed! mime: %s\n", param->mimeType);
        return -1;
    }
    printf("AddTrackCover success! trackIndex: %d\n", trackIndex);
    return trackIndex;
}

void WriteSingleTrackSample(OH_AVMuxer *muxer, int trackId, int fd)
{
    if (muxer == NULL || fd < 0 || trackId  < 0) {
        printf("WriteSingleTrackSample muxer is null or fd < 0, fd:%d\n", fd);
        return;
    }
    int ret = 0;
    int dataSize = 0;
    int flags = 0;
    unsigned char *avMuxerDemoBuffer = NULL;
    int avMuxerDemoBufferSize = 0;
    OH_AVCodecBufferAttr info;
    memset_s(&info, sizeof(info), 0, sizeof(info));
    while (1) {
        ret = read(fd, (void*)&info.pts, sizeof(info.pts));
        if (ret <= 0) {
            break;
        }

        ret = read(fd, (void*)&flags, sizeof(flags));
        if (ret <= 0) {
            break;
        }

        // read frame buffer
        ret = read(fd, (void*)&dataSize, sizeof(dataSize));
        if (ret <= 0 || dataSize < 0) {
            break;
        }

        if (avMuxerDemoBuffer != NULL && dataSize > avMuxerDemoBufferSize) {
            free(avMuxerDemoBuffer);
            avMuxerDemoBufferSize = 0;
            avMuxerDemoBuffer = NULL;
        }
        if (avMuxerDemoBuffer == NULL) {
            avMuxerDemoBuffer = malloc(dataSize);
            avMuxerDemoBufferSize = dataSize;
            if (avMuxerDemoBuffer == NULL) {
                printf("error malloc memory! %d\n", dataSize);
                break;
            }
        }
        ret = read(fd, (void*)avMuxerDemoBuffer, dataSize);
        if (ret <= 0) {
            break;
        }
        info.size = dataSize;

        info.flags = 0;
        if (flags != 0) {
            info.flags |= AVCODEC_BUFFER_FLAGS_SYNC_FRAME;
        }

        if (OH_AVMuxer_WriteSampleBuffer(muxer, trackId, avMuxerDemoBuffer, info) != AV_ERR_OK) {
            printf("OH_AVMuxer_WriteSampleBuffer error!\n");
            break;
        }
    }

    if (avMuxerDemoBuffer != NULL) {
        free(avMuxerDemoBuffer);
    }
}

void *ThreadWriteTrackSample(void *param)
{
    struct WriteTrackSampleParam *wrTrackParam = (struct WriteTrackSampleParam *)param;
    WriteSingleTrackSample(wrTrackParam->muxer, wrTrackParam->trackId, wrTrackParam->fd);
    return NULL;
}

void WriteTrackSample(OH_AVMuxer *muxer, int audioTrackIndex, int videoTrackIndex, FdListStr *fdStr)
{
    if (fdStr == NULL || fdStr->inAudioFd < 0 || fdStr->inVideoFd < 0) {
        printf("WriteTrackSample start failed!\n");
        return;
    }
    printf("WriteTrackSample\n");
    int dataSize = 0;
    int ret = 0;
    int trackId = 0;
    int curFd = 0;
    int flags = 0;
    int64_t audioPts = 0;
    int64_t videoPts = 0;
    OH_AVCodecBufferAttr info;
    memset_s(&info, sizeof(info), 0, sizeof(info));
    unsigned char *avMuxerDemoBuffer = NULL;
    int avMuxerDemoBufferSize = 0;

    ret = read(fdStr->inAudioFd, (void*)&audioPts, sizeof(audioPts));
    if (ret <= 0) {
        return;
    }
    ret = read(fdStr->inVideoFd, (void*)&videoPts, sizeof(videoPts));
    if (ret <= 0) {
        return;
    }
    while (1) {
        if (audioPts > videoPts) { // write video
            info.pts = videoPts;
            trackId =  videoTrackIndex;
            curFd = fdStr->inVideoFd;
        } else { // write audio
            info.pts = audioPts;
            trackId = audioTrackIndex;
            curFd = fdStr->inAudioFd;
        }

        // read flags
        ret = read(curFd, (void*)&flags, sizeof(flags));
        if (ret <= 0) {
            break;
        }
        info.flags = 0;
        if (flags != 0) {
            info.flags |= AVCODEC_BUFFER_FLAGS_SYNC_FRAME;
        }

        // read frame buffer
        ret = read(curFd, (void*)&dataSize, sizeof(dataSize));
        if (ret <= 0 || dataSize < 0) {
            break;
        }

        if (avMuxerDemoBuffer != NULL && dataSize > avMuxerDemoBufferSize) {
            free(avMuxerDemoBuffer);
            avMuxerDemoBufferSize = 0;
            avMuxerDemoBuffer = NULL;
        }
        if (avMuxerDemoBuffer == NULL) {
            avMuxerDemoBuffer = malloc(dataSize);
            avMuxerDemoBufferSize = dataSize;
            if (avMuxerDemoBuffer == NULL) {
                printf("error malloc memory! %d\n", dataSize);
                break;
            }
        }
        ret = read(curFd, (void*)avMuxerDemoBuffer, dataSize);
        if (ret <= 0) {
            break;
        }
        info.size = dataSize;

        if (OH_AVMuxer_WriteSampleBuffer(muxer, trackId, avMuxerDemoBuffer, info) != AV_ERR_OK) {
            printf("OH_AVMuxer_WriteSampleBuffer error!\n");
            break;
        }

        if (curFd == fdStr->inVideoFd) {
            ret = read(fdStr->inVideoFd, (void*)&videoPts, sizeof(videoPts));
            if (ret <= 0) {
                break;
            }
        } else {
            ret = read(fdStr->inAudioFd, (void*)&audioPts, sizeof(audioPts));
            if (ret <= 0) {
                break;
            }
        }
    }

    if (avMuxerDemoBuffer != NULL) {
        free(avMuxerDemoBuffer);
    }
}

void WriteTrackCover(OH_AVMuxer *muxer, int coverTrackIndex, int fdInput)
{
    printf("WriteTrackCover\n");
    OH_AVCodecBufferAttr info;
    memset_s(&info, sizeof(info), 0, sizeof(info));
    struct stat fileStat;
    fstat(fdInput, &fileStat);
    info.size = fileStat.st_size;
    unsigned char *avMuxerDemoBuffer = malloc(info.size);
    if (avMuxerDemoBuffer == NULL) {
        printf("malloc memory error! size: %d \n", info.size);
        return;
    }

    int ret = read(fdInput, avMuxerDemoBuffer, info.size);
    if (ret <= 0) {
        free(avMuxerDemoBuffer);
        return;
    }

    if (OH_AVMuxer_WriteSampleBuffer(muxer, coverTrackIndex, avMuxerDemoBuffer, info) != AV_ERR_OK) {
        free(avMuxerDemoBuffer);
        printf("OH_AVMuxer_WriteSampleBuffer error!\n");
        return;
    }
    free(avMuxerDemoBuffer);
}

int GetInputNum(int defaultNum)
{
    int num = defaultNum;
    num = getchar();
    if (num == '\n') { // default
        num = defaultNum;
    } else {
        ungetc(num, stdin);
        scanf("%d", &num);
        fflush(stdin);
    }
    return num;
}


void NativeSelectMuxerType()
{
    int num;

    printf("\nplese select muxer type : 0.mp4 1.m4a\n");
    num = GetInputNum(0);
    switch (num)
    {
    case 0:
        g_muxerParam.outputFormat = AV_OUTPUT_FORMAT_MPEG_4;
        snprintf(g_muxerParam.outputFormatType, sizeof(g_muxerParam.outputFormatType), "%s", "mp4");
        break;
    case 1:
        g_muxerParam.outputFormat = AV_OUTPUT_FORMAT_M4A;
        snprintf(g_muxerParam.outputFormatType, sizeof(g_muxerParam.outputFormatType), "%s", "m4a");
        break;
    default:
        g_muxerParam.outputFormat = AV_OUTPUT_FORMAT_MPEG_4;
        snprintf(g_muxerParam.outputFormatType, sizeof(g_muxerParam.outputFormatType), "%s", "mp4");
        break;
    }
    printf("select mode:%d\n", num);
}

void NativeSelectRunMode()
{
    int num;

    printf("\nplese select audio vide wrtie mode:\n");
    printf("0. audio video write in sample thread\n");
    printf("1. audio video write in different thread\n");
    num = GetInputNum(0);
    switch (num)
    {
    case 0:
        g_muxerParam.runMode = NORMAL;
        snprintf(g_muxerParam.runModeType, sizeof(g_muxerParam.runModeType), "%s", RUN_NORMAL);
        break;
    case 1:
        g_muxerParam.runMode = THREAD;
        snprintf(g_muxerParam.runModeType, sizeof(g_muxerParam.runModeType), "%s", RUN_MUL_THREAD);
        break;
    default:
        g_muxerParam.runMode = NORMAL;
        snprintf(g_muxerParam.runModeType, sizeof(g_muxerParam.runModeType), "%s", RUN_NORMAL);
        break;
    }
    printf("select mode:%d\n", num);
}

void NativeSelectAudio()
{
    int num;

    printf("\nplese select audio mode: 0.noAudio 1.aac 2.mpeg\n");
    num = GetInputNum(1);
    switch (num)
    {
    case 1:
        g_muxerParam.audioParams = &g_audioAacPar;
        snprintf(g_muxerParam.audioType, sizeof(g_muxerParam.audioType), "%s", "aac");
        break;
    case 2:
        g_muxerParam.audioParams = &g_audioMpegPar;
        snprintf(g_muxerParam.audioType, sizeof(g_muxerParam.audioType), "%s", "mpeg");
        break;
    default:
        g_muxerParam.audioParams = NULL;
        snprintf(g_muxerParam.audioType, sizeof(g_muxerParam.audioType), "%s", "noAudio");
        break;
    }
    printf("select mode:%d\n", num);
}

void NativeSelectVideo()
{
    int num;

    printf("\nplese select video mode: 0.noVideo 1.h264 2.mpeg4\n");
    num = GetInputNum(1);
    switch (num)
    {
    case 1:
        g_muxerParam.videoParams = &g_videoH264Par;
        snprintf(g_muxerParam.videoType, sizeof(g_muxerParam.videoType), "%s", "h264");
        break;
    case 2:
        g_muxerParam.videoParams = &g_videoMpeg4Par;
        snprintf(g_muxerParam.videoType, sizeof(g_muxerParam.videoType), "%s", "mpeg4");
        break;
    default:
        g_muxerParam.videoParams = NULL;
        snprintf(g_muxerParam.videoType, sizeof(g_muxerParam.videoType), "%s", "noVideo");
        break;
    }
    printf("select mode:%d\n", num);
}

void NativeSelectCover()
{
    int num;

    printf("\nplese select cover mode: 0.noCover 1.jpg 2.png 3.bmp\n");
    num = GetInputNum(1);
    switch (num)
    {
    case 1:
        g_muxerParam.coverParams = &g_jpegCoverPar;
        snprintf(g_muxerParam.coverType, sizeof(g_muxerParam.coverType), "%s", "jpg");
        break;
    case 2:
        g_muxerParam.coverParams = &g_pngCoverPar;
        snprintf(g_muxerParam.coverType, sizeof(g_muxerParam.coverType), "%s", "png");
        break;
    case 3:
        g_muxerParam.coverParams = &g_bmpCoverPar;
        snprintf(g_muxerParam.coverType, sizeof(g_muxerParam.coverType), "%s", "bmp");
        break;
    default:
        g_muxerParam.coverParams = NULL;
        snprintf(g_muxerParam.coverType, sizeof(g_muxerParam.coverType), "%s", "noCover");
        break;
    }
    printf("select mode:%d\n", num);
}

void NativeSelectMode()
{
    if (g_muxerParam.outputFormat != AV_OUTPUT_FORMAT_DEFAULT) {
        return;
    }
    
    NativeSelectMuxerType();
    NativeSelectRunMode();
    NativeSelectAudio();
    NativeSelectVideo();
    NativeSelectCover();
}

int OpenAllInputFile(FdListStr *fdStr)
{
    if (!fdStr) {
        printf("fdStr is null!\n");
        return -1;
    }

    if (g_muxerParam.audioParams) {
        fdStr->inAudioFd = open(g_muxerParam.audioParams->fileName, O_RDONLY);
        if (fdStr->inAudioFd < 0) {
            printf("open %s failed!!\n", g_muxerParam.audioParams->fileName);
        } else {
            printf("open file %s success, -fd:%d, -flags %x\n", g_muxerParam.audioParams->fileName,
                fdStr->inAudioFd, fcntl(fdStr->inAudioFd, F_GETFL, 0));
        }
    }

    if (g_muxerParam.videoParams) {
        fdStr->inVideoFd = open(g_muxerParam.videoParams->fileName, O_RDONLY);
        if (fdStr->inVideoFd < 0) {
            printf("open %s failed!!\n", g_muxerParam.videoParams->fileName);
        } else {
            printf("open file %s success, -fd:%d, -flags %x\n", g_muxerParam.videoParams->fileName,
                fdStr->inVideoFd, fcntl(fdStr->inVideoFd, F_GETFL, 0));
        }
    }

    if (g_muxerParam.coverParams) {
        fdStr->inCoverFd = open(g_muxerParam.coverParams->fileName, O_RDONLY);
        if (fdStr->inCoverFd < 0) {
            printf("open %s failed!!\n", g_muxerParam.coverParams->fileName);
        } else {
            printf("open file %s success, -fd:%d, -flags %x\n", g_muxerParam.coverParams->fileName,
                fdStr->inCoverFd, fcntl(fdStr->inCoverFd, F_GETFL, 0));
        }
    }
    return 0;
}

int DoRunMuxer(FdListStr *fdStr, OH_AVMuxer *muxer)
{
    if (fdStr == NULL || muxer == NULL) {
        printf("fdStr or  muxer is null!\n");
        return -1;
    }

    if (OH_AVMuxer_SetLocation(muxer, 10, 10) != AV_ERR_OK 
        || OH_AVMuxer_SetRotation(muxer, 0) != AV_ERR_OK ) {
        printf("set failed!\n");
        return -1;
    }
    int audioTrackIndex = AddTrackAudio(muxer, g_muxerParam.audioParams, fdStr->inAudioFd);
    int videoTrackIndex = AddTrackVideo(muxer, g_muxerParam.videoParams, fdStr->inVideoFd);
    int coverTrackIndex =  AddTrackCover(muxer, g_muxerParam.coverParams, fdStr->inCoverFd);
    
    if (OH_AVMuxer_Start(muxer) != AV_ERR_OK) {
        printf("start muxer failed!\n");
        return -1;
    }

    if (coverTrackIndex >= 0) {
        WriteTrackCover(muxer, coverTrackIndex, fdStr->inCoverFd);
    }

    if (g_muxerParam.runMode == NORMAL) {
        printf("== write audio video sample in same thread\n");
        if (audioTrackIndex >= 0 && videoTrackIndex >= 0) {
            WriteTrackSample(muxer, audioTrackIndex, videoTrackIndex, fdStr);
        } else if (audioTrackIndex >= 0) {
            WriteSingleTrackSample(muxer, audioTrackIndex, fdStr->inAudioFd);
        } else if (videoTrackIndex >= 0) {
            WriteSingleTrackSample(muxer, videoTrackIndex, fdStr->inVideoFd);
        }
    } else if (g_muxerParam.runMode == THREAD) {
        printf("== write audio video sample in different thread\n");
        pthread_t auThread;
        pthread_t viThread;

        struct WriteTrackSampleParam audioThParam = {muxer, audioTrackIndex, fdStr->inAudioFd};
        struct WriteTrackSampleParam videoThparam = {muxer, videoTrackIndex, fdStr->inVideoFd};
        pthread_create(&auThread, NULL, ThreadWriteTrackSample, &audioThParam);
        pthread_create(&viThread, NULL, ThreadWriteTrackSample, &videoThparam);

        pthread_join(viThread, NULL);
        pthread_join(auThread, NULL);
    }

    if (OH_AVMuxer_Stop(muxer) != AV_ERR_OK) {
        printf("stop muxer failed!\n");
        return -1;
    }
    printf("native avmuxer finish! fd:out:%d, audio:%d, video:%d, cover:%d\n",
        fdStr->outputFd, fdStr->inAudioFd, fdStr->inVideoFd, fdStr->inCoverFd);
    return 0;
}

void CloseAllFd(FdListStr *fdStr)
{
    printf("close fd : [");
    int fdTotalCount = sizeof(*fdStr) /sizeof(fdStr->start[0]);
    for (int i = 0; i < fdTotalCount; i++) {
        printf("%d, ", fdStr->start[i]);
        if (fdStr->start[i] > 0) {
            close(fdStr->start[i]);
            fdStr->start[i] = -1;
        }
    }
    printf("\b\b]\n");
}

int RunNativeMuxer(const char *out)
{
    FdListStr fdStr;
    int fdTotalCount = sizeof(fdStr) /sizeof(fdStr.start[0]);
    printf("fd list total size is %d\n", fdTotalCount);
    for (int i = 0; i < fdTotalCount; i++) {
        fdStr.start[i] = -1;
    }

    if (OpenAllInputFile(&fdStr) < 0) {
        CloseAllFd(&fdStr);
        return -1;
    }

    char outFileName[100] = {0};
    snprintf(outFileName, sizeof(outFileName), "%s_%s_%s_%s_%s.%s", 
        out, g_muxerParam.runModeType, g_muxerParam.audioType, g_muxerParam.videoType,
        g_muxerParam.coverType, g_muxerParam.outputFormatType);

    fdStr.outputFd = open(outFileName, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdStr.outputFd < 0) {
        printf("open file failed! filePath is: %s %d\n", outFileName, fdStr.outputFd);
        CloseAllFd(&fdStr);
        return -1;
    }
    printf("open file %s success, -fd:%d, -flags %x\n", outFileName, fdStr.outputFd, fcntl(fdStr.outputFd, F_GETFL, 0));

    OH_AVMuxer *muxer = OH_AVMuxer_Create(fdStr.outputFd, g_muxerParam.outputFormat);
    DoRunMuxer(&fdStr, muxer);

    if (muxer != NULL) {
        OH_AVMuxer_Destroy(muxer);
        muxer = NULL;
    }

    CloseAllFd(&fdStr);
    
    return 0;
}
