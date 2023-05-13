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

#include <climits>
#include <iostream>
#include <thread>
#include <vector>
#include "avmuxer_demo.h"
#include "avmuxer_ffmpeg_demo.h"
#include "avmuxer_engine_demo.h"
#include "avcodec_audio_decoder_inner_demo.h"
#include "avcodec_audio_encoder_inner_demo.h"
#include "avcodec_audio_decoder_demo.h"
#include "avcodec_audio_encoder_demo.h"
#include "videodec_ffmpeg_demo.h"
#include "codeclist_demo.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::AudioDemo;
using namespace OHOS::Media::InnerAudioDemo;
using namespace OHOS::Media::Codec;
using namespace std;

constexpr int RUN_TIME = 600;

extern "C" {
    extern int NativeSelectMode();
    extern int RunNativeMuxer(const char *out);
}

static int RunLoopNativeMuxer(string out)
{
    time_t startTime = time(NULL);
    time_t curTime = time(NULL);
    while (difftime(curTime, startTime) < RUN_TIME) {
        RunNativeMuxer(out.c_str());
        time(&curTime);
    }
    return 0;
}

static int RunAVMuxer()
{
    auto avmuxer = std::make_unique<AVMuxerDemo>();
    if (avmuxer == nullptr) {
        cout << "avmuxer is null" << endl;
        return 0;
    }
    avmuxer->RunCase();
    cout << "demo avmuxer end" << endl;
    return 0;
}

static int RunAVMuxerWithMultithread()
{
    auto avmuxer = std::make_unique<AVMuxerDemo>();
    if (avmuxer == nullptr) {
        cout << "avmuxer is null" << endl;
        return 0;
    }
    avmuxer->RunMultiThreadCase();
    cout << "demo multi thread avmuxer end" << endl;
    return 0;
}

static int RunFfmpegMuxer()
{
    std::unique_ptr<AVMuxerDemoBase> ffmpegMuxer = std::make_unique<AVMuxerFFmpegDemo>();
    if (ffmpegMuxer == nullptr) {
        cout << "ffmpegMuxer is null" << endl;
        return 0;
    }
    ffmpegMuxer->RunCase();
    cout << "demo ffmpegMuxer end" << endl;
    return 0;
}

static int RunEngineMuxer()
{
    std::unique_ptr<AVMuxerDemoBase> muxer = std::make_unique<AVMuxerEngineDemo>();
    if (muxer == nullptr) {
        cout << "AVMuxerEngineDemo is null" << endl;
        return 0;
    }
    muxer->RunCase();
    cout << "demo engine demo end" << endl;
    return 0;
}


static int RunLoopEngineMuxer()
{
    time_t startTime = time(NULL);
    time_t curTime = time(NULL);
    while (difftime(curTime, startTime) < RUN_TIME) {
        RunEngineMuxer();
        time(&curTime);
    }
    return 0;
}

static int RunAudioDecoder()
{
    auto audioEnc = std::make_unique<ADecDemo>();
    if (audioEnc == nullptr) {
        cout << "audio decoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioEncoder()
{
    auto audioEnc = std::make_unique<AEncDemo>();
    if (audioEnc == nullptr) {
        cout << "audio encoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio encoder end" << endl;
    return 0;
}

static int RunAudioInnerDecoder()
{
    auto audioEnc = std::make_unique<ADecInnerDemo>();
    if (audioEnc == nullptr) {
        cout << "audio decoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioInnerEncoder()
{
    auto audioEnc = std::make_unique<AEnInnerDemo>();
    if (audioEnc == nullptr) {
        cout << "audio encoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio encoder end" << endl;
    return 0;
}

static int RunVideoInnerDecoder()
{      
    cout << "please input: filePath width height" << endl;
    string args;
    (void)getline(cin, args);

    vector<string> res;
    string pattern = " ";
    
    args += pattern;
    size_t pos = args.find(pattern);
    while(pos != args.npos)
    {
        string temp = args.substr(0, pos);
        res.push_back(temp);
        args = args.substr(pos+1, args.size());
        pos = args.find(pattern);
    }

    if(res.size() != 3){
        cout << "input args invalid" << endl;
    }
    
    string inputFile = res[0].c_str();
    pos = res[0].find(".");
    string outputFile = res[0].substr(0, pos) + ".yuv";
    int32_t width = stoi(res[1]);
    int32_t height = stoi(res[2]);

    cout << "inputFile:"<<inputFile<<",width:"<<width<<",height:"<<height<<endl;

    auto vdec = std::make_unique<VDecFfmpegSample>();
    if (vdec == nullptr) {
        cout << "vdec is null" << endl;
    }

    FILE *inFp = fopen(inputFile.c_str(), "rb");
    if (inFp == nullptr) {
        cout << "failed to open input" << endl;
        return 0;
    }

    FILE *outFp = fopen(outputFile.c_str(), "wb");
    if (outFp == nullptr) {
        cout << "failed to open output" << endl;
        return 0;
    }

    vdec->RunVideoDec(inFp, outFp, width, height);
    cout << "video decoder sample end" << endl;
    return 0;
}

static int RunCodecList()
{
    auto codecList = std::make_unique<CodecListDemo>();
    if (codecList == nullptr) {
        cout << "codec list is null" << endl;
        return 0;
    }
    codecList->RunCase();
    cout << "codec list end" << endl;
    return 0;
}

int main(int argc, char *argv[])
{
    constexpr int minRequiredArgCount = 2;
    string path;
    if (argc >= minRequiredArgCount && argv[1] != nullptr) {
        path = argv[1];
    }
    cout << "Please select a demo scenario number(default Audio Decoder): " << endl;
    cout << "0:Audio Decoder" << endl;
    cout << "1:Audio Encoder" << endl;
    cout << "2:Audio Inner Decoder" << endl;
    cout << "3:Audio Inner Encoder" << endl;
    cout << "4:native_muxer" << endl;
    cout << "5:native_muxer loop" << endl;
    cout << "6:native_muxer multithread" << endl;
    cout << "7:inner_muxer" << endl;
    cout << "8:inner_muxer with multithread write" << endl;
    cout << "9:ffmpeg_muxer" << endl;
    cout << "10:engine_muxer" << endl;
    cout << "11:engine_muxer loop" << endl;
    cout << "12:Video Inner Decoder" << endl;
    cout << "13:codeclist" << endl;

    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        (void)RunAudioDecoder();
    } else if (mode == "1") {
        (void)RunAudioEncoder();
    } else if (mode == "2") {
        (void)RunAudioInnerDecoder();
    } else if (mode == "3") {
        (void)RunAudioInnerEncoder();
    } else if (mode == "4") {
        NativeSelectMode();
        RunNativeMuxer("native_mux");
    } else if (mode == "5") {
        NativeSelectMode();
        RunLoopNativeMuxer("loop_native_mux");
    } else if (mode == "6") {
        NativeSelectMode();
        vector<thread> vecThread;
        for (int i = 0; i < 10; ++i) {
            string out = to_string(i + 1);
            out += "_native_mux";
            vecThread.push_back(thread(RunLoopNativeMuxer, out));
        }
        for (int i = 0; i < 10; ++i) {
            vecThread[i].join();
        }
    } else if (mode == "7") {
        RunAVMuxer();
    } else if (mode == "8") {
        RunAVMuxerWithMultithread();
    } else if (mode == "9") {
        RunFfmpegMuxer();
    } else if (mode == "10") {
        RunEngineMuxer();
    } else if (mode == "11") {
        RunLoopEngineMuxer();
    } else if (mode == "12") {
        RunVideoInnerDecoder();
    } else if (mode == "13") {
        RunCodecList();
    }  else {
        cout << "no that selection" << endl;
    }
    return 0;
}
