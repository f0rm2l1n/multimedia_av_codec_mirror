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

#include "avmuxer_demo_common.h"

enum ModeNum {
    MODE_ZERO = 0,
    MODE_ONE = 1,
    MODE_TWO = 2,
    MODE_THREE = 3
};

struct AudioTrackParam g_audioMpegPar = {
    .fileName = "mpeg_44100_2.bin",
    .mimeType = "audio/mpeg",
    .sampleRate = 44100,
    .channels = 2,
};

struct AudioTrackParam g_audioAacPar = {
    .fileName = "aac_44100_2.bin",
    .mimeType = "audio/mp4a-latm",
    .sampleRate = 44100,
    .channels = 2,
};

struct VideoTrackParam g_videoH264Par = {
    .fileName = "h264_640_360.bin",
    .mimeType = "video/avc",
    .width = 640,
    .height = 360,
};

struct VideoTrackParam g_videoMpeg4Par = {
    .fileName = "mpeg4_720_480.bin",
    .mimeType = "video/mp4v-es",
    .width = 720,
    .height = 480,
};

struct VideoTrackParam g_jpegCoverPar = {
    .fileName = "greatwall.jpg",
    .mimeType = "image/jpeg",
    .width = 352,
    .height = 288,
};

struct VideoTrackParam g_pngCoverPar = {
    .fileName = "greatwall.png",
    .mimeType = "image/png",
    .width = 352,
    .height = 288,
};

struct VideoTrackParam g_bmpCoverPar = {
    .fileName = "greatwall.bmp",
    .mimeType = "image/bmp",
    .width = 352,
    .height = 288,
};

const char *RUN_NORMAL = "normal";
const char *RUN_MUL_THREAD = "multhrd";
