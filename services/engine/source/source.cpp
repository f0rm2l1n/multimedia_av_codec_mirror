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
#include "source.h"

#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <sys/stat.h>

#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "avcodec_common.h"
#include "media_source.h"
#include "format.h"

static std::string g_libFileHead = "libhistreamer_plugin_";
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
static std::string g_fileSeparator = "\\";
#else
static std::string g_fileSeparator = "/";
#endif
static std::string g_libFileTail = ".z.so";

using namespace OHOS::Media::Plugin;

Status FfmpegRegister::AddPlugin(const PluginDefBase& def)
{
    auto& tmpDef = (SourcePluginDef&) def;
    sourcePlugin = (tmpDef.creator)(def.name);
    return Status::OK;
}

Status FfmpegRegister::AddPackage(const PackageDef& def)
{
    packageDef = std::make_shared<PackageDef>(def);
    return Status::OK;
}

namespace OHOS {
namespace Media {
constexpr size_t DEFAULT_READ_SIZE = 4096;


namespace {
   constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "Source"};
}

namespace {

#ifdef WIN32
#include <windows.h>
#include <io.h>
char *realpath(const char *__restrict name, char *__restrict resolved)
{
    return _fullpath(resolved, name, PATH_MAX);
}

int access(const char *name, int type)
{
    return _access(name, type);
}
#else
#include <unistd.h>
#endif

inline bool FileIsExists (const char* name) {
  struct stat buffer;   
  return (stat (name, &buffer) == 0); 
}


bool ConvertFullPath(const std::string& partialPath, std::string& fullPath)
{
   if (partialPath.empty() || partialPath.length() >= PATH_MAX) {
       return false;
   }
   char tmpPath[PATH_MAX] = {0};
   if (realpath(partialPath.c_str(), tmpPath) == nullptr) {
       return false;
   }
   if (access(tmpPath, F_OK) || access(tmpPath, R_OK)) {
       return false;
   }
   fullPath = tmpPath;
   AVCODEC_LOGD("[Source::Create] convert partialPaht to fullPath: %{public}s -> %{public}s.", partialPath.c_str(), fullPath.c_str());
   return true;
}

static std::map<std::string, std::string> pluginMap = {
    {"http", "libhistreamer_plugin_HttpSource.z.so"},
    {"https", "libhistreamer_plugin_HttpSource.z.so"},
    {"fd", "libhistreamer_plugin_FileFdSource.z.so"},
    {"file", "libhistreamer_plugin_FileSource.z.so"}
};

std::map<std::string, std::shared_ptr<AVInputFormat>> g_pluginInputFormat;

std::string GetUriSuffix(const std::string& uri)
{
    AVCODEC_LOGD("GetUriSuffix, input: uri=%{public}s", uri.c_str());
    std::string suffix;
    auto const pos = uri.find_last_of('.');
    if (pos != std::string::npos) {
        suffix = uri.substr(pos + 1);
    }
    AVCODEC_LOGD("suffix: %{public}s", suffix.c_str());
    return suffix;
}

int32_t ParseProtocol(const std::string& uri, std::string& protocol)
{
    AVCODEC_LOGD("ParseProtocol, input: uri=%{public}s, protocol=%{public}s", uri.c_str(), protocol.c_str());
    int32_t ret;
    auto const pos = uri.find("://");
    if (pos != std::string::npos) {
        auto prefix = uri.substr(0, pos);
        protocol.append(prefix);
        ret = AVCS_ERR_OK;
    } else {
        protocol.append("file");
        std::string fullPath;
        ret = ConvertFullPath(uri, fullPath);
    }
    
    if (protocol.empty()) {
        AVCODEC_LOGE("ERROR:Invalid protocol: %{public}s", protocol.c_str());
        ret = AVCS_ERR_INVALID_OPERATION;
    }
    return ret;
}

RegisterFunc OpenFilePlugin(const std::string& path, const std::string& name)
{
    AVCODEC_LOGD("OpenFilePlugin, input: path=%{public}s, name=%{public}s", path.c_str(), name.c_str());
    void *handler = nullptr;

    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        handler =  ::LoadLibraryA(path.c_str());
    #else
        auto pathStr = path.c_str();
        std::string tmpStr = "/mnt/data/z30028037/workspace/av_codec_0425/av_codec/services/services/source/hst_releated/libhistreamer_plugin_FileSource.z.so";
        pathStr = tmpStr.c_str();
        if (FileIsExists(pathStr)) {
            handler = ::dlopen(pathStr, RTLD_NOW);
            if (handler == nullptr) {
                AVCODEC_LOGE("dlopen failed due to %{public}s", ::dlerror());
            }
        }
    #endif
    if (handler) {
        std::string registerFuncName = "register_" + name;
        RegisterFunc registerFunc = nullptr;

        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
            registerFunc = (RegisterFunc)(::GetProcAddress((HMODULE)handler, registerFuncName.c_str()));
        #else
            registerFunc = (RegisterFunc)(::dlsym(handler, registerFuncName.c_str()));
        #endif

        if (registerFunc) {
            return registerFunc;
        } else {
            AVCODEC_LOGE("register is not found in %{public}s", registerFuncName.c_str());
        }
    } else {
        AVCODEC_LOGE("dlopen failed: %{public}s", pathStr);
    }
    return {};
}

bool IsInputFormatSupported(const char* name)
{
    if (!strcmp(name, "audio_device") || !strncmp(name, "image", 5) ||                 
        !strcmp(name, "mjpeg") || !strcmp(name, "redir") || !strncmp(name, "u8", 2) || 
        !strncmp(name, "u16", 3) || !strncmp(name, "u24", 3) ||                        
        !strncmp(name, "u32", 3) ||                                                    
        !strncmp(name, "s8", 2) || !strncmp(name, "s16", 3) ||                         
        !strncmp(name, "s24", 3) ||                                                    
        !strncmp(name, "s32", 3) || !strncmp(name, "f32", 3) ||                        
        !strncmp(name, "f64", 3) ||                                                    
        !strcmp(name, "mulaw") || !strcmp(name, "alaw")) {
        return false;
    }

    /* no network demuxers */
    if (!strcmp(name, "sdp") || !strcmp(name, "rtsp") || !strcmp(name, "applehttp")) {
        return false;
    }
    return true;
}


void ReplaceDelimiter(const std::string& delmiters, char newDelimiter, std::string& str)
{
   for (auto it = str.begin(); it != str.end(); ++it) {
       if (delmiters.find(newDelimiter) != std::string::npos) {
           *it = newDelimiter;
       }
   }
};

}


Source::Source()
    :formatContext_(nullptr),inputFormat_(nullptr) 
{ 
    AVCODEC_LOGI("Source::Source is on call");
    register_ = std::make_shared<FfmpegRegister>();
}


Source::~Source()
{
    formatContext_ = nullptr;
    inputFormat_ = nullptr;
    sourcePlugin_ = nullptr;
    register_ = nullptr;
    avioContext_ = nullptr;
    AVCODEC_LOGI("Source::~Source is on call");
}

int32_t Source::GetTrackCount(uint32_t &trackCount)
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION, "call getTrackcount failed, because create source failed!");
    trackCount = static_cast<uint32_t>(formatContext_->nb_streams);
    return AVCS_ERR_OK;
}

int32_t Source::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    AVCODEC_LOGI("Source::SetTrackFormat is on call");
    AVCODEC_LOGI("set track: %{public}d, format: %{public}s", trackIndex, format.Stringify().c_str());


    if ( trackIndex < 0 || trackIndex >= static_cast<uint32_t>(formatContext_->nb_streams)) {
        AVCODEC_LOGE("trackIndex is invalid!");
        return AVCS_ERR_INVALID_VAL;
    }

    AVStream *stream = formatContext_->streams[trackIndex];

    CHECK_AND_RETURN_RET_LOG(stream != nullptr, AVCS_ERR_INVALID_OPERATION, "streams %{public}d is nullptr!", trackIndex);

    Format::FormatDataMap formatMap = format.GetFormatMap();
    AVDictionary *streamMetadata = stream->metadata;
    for( auto iter = formatMap.rbegin(); iter != formatMap.rend(); iter++) {
        if ( iter->first == AVSourceTrackFormat::VIDEO_BIT_STREAM_FORMAT ) {
            if ( stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
                AVCODEC_LOGE("track is not video, set VIDEO_BIT_STREAM_FORMAT failed!");
                return AVCS_ERR_INVALID_OPERATION;
            }
            int32_t streamFormat = iter->second.val.int32Val;
            AVCODEC_LOGD("SetTrackFormat format: streamFormat: %{public}d, codec_id: %{public}d", streamFormat, stream->codecpar->codec_id);
            if ( (streamFormat == VideoBitStreamFormat::AVCC && stream->codecpar->codec_id != AV_CODEC_ID_H264) || 
                 (streamFormat == VideoBitStreamFormat::HVCC && stream->codecpar->codec_id != AV_CODEC_ID_HEVC )) {
                return AVCS_ERR_INVALID_OPERATION;
            };
            av_dict_set(&streamMetadata, iter->first.c_str(), (std::to_string(streamFormat)).c_str(), 0);
        }
    }
    av_dict_copy(&stream->metadata, streamMetadata, 0);
    
    return AVCS_ERR_OK;
}

int32_t Source::GetSourceFormat(Format &format)
{
    AVCODEC_LOGI("Source::GetFormat is on call");
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION, "formatContext_ is nullptr!");

    Format::FormatDataMap formatMap = format.GetFormatMap();
    int32_t ret;
    AVDictionaryEntry *valPtr;

    valPtr = av_dict_get(formatContext_->metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss title info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_TITLE, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss title info in file");
        } 
    }

    valPtr = av_dict_get(formatContext_->metadata, "artist", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss artist info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_ARTIST, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss artist info in file");
        } 
    }

    valPtr = av_dict_get(formatContext_->metadata, "album", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss album info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_ALBUM, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss album info in file");
        } 
    }

    valPtr = av_dict_get(formatContext_->metadata, "album_artist", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss album_artist info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_ALBUM_ARTIST, valPtr->value);   
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss album_artist info in file");
        }  
    }
    
    valPtr = av_dict_get(formatContext_->metadata, "date", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss data info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_DATE, valPtr->value);  
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss data info in file");
        }   
    }
    
    valPtr = av_dict_get(formatContext_->metadata, "comment", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss comment info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_COMMENT, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss comment info in file");
        } 
    }
        
    valPtr = av_dict_get(formatContext_->metadata, "genre", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss genre info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_GENRE, valPtr->value);
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss genre info in file");
        } 
    }
        
    valPtr = av_dict_get(formatContext_->metadata, "copyright", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss copyright info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_COPYRIGHT, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss copyright info in file");
        } 
    }
       
    valPtr = av_dict_get(formatContext_->metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss language info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_LANGUAGE, valPtr->value);
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss language info in file");
        } 
    }
        
    valPtr = av_dict_get(formatContext_->metadata, "description", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss description info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_DESCRIPTION, valPtr->value);
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss description info in file");
        } 
    }
        
    valPtr = av_dict_get(formatContext_->metadata, "lyrics", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
        AVCODEC_LOGW("Put track info failed: miss lyrics info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_LYRICS, valPtr->value);    
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss lyrics info in file");
        }
    }
       
    ret = format.PutLongValue( AVSourceFormat::SOURCE_DURATION,formatContext_->duration);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Put track info failed: miss album_artist info in file");
    }

    valPtr = av_dict_get(formatContext_->metadata, "media_type", nullptr, AV_DICT_IGNORE_SUFFIX);
    if (valPtr == nullptr) {
         AVCODEC_LOGW("Put track info failed: miss media_type info in file");
    } else {
        ret = format.PutStringValue( AVSourceFormat::SOURCE_TYPE, valPtr->value);
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Put track info failed: miss media_type info in file");
        }
    }

    return AVCS_ERR_OK;
}


int32_t Source::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    AVCODEC_LOGI("Source::GetTrackFormat is on call");

    int ret=-1;
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION, "GetTrackFormat failed, formatContext_ is nullptr!");
    if ( trackIndex < 0 || trackIndex >= static_cast<uint32_t>(formatContext_->nb_streams)) {
        AVCODEC_LOGE("trackIndex is invalid!");
        return AVCS_ERR_INVALID_VAL;
    }
    auto stream = formatContext_->streams[trackIndex];
    ret = format.PutIntValue(AVSourceTrackFormat::TRACK_INDEX, trackIndex);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss index info in track %{public}d", trackIndex);
    }

    ret = format.PutLongValue(AVSourceTrackFormat::TRACK_SAMPLE_COUNT, stream->nb_frames);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss sample cout info in track %{public}d", trackIndex);
    }
    
    ret = format.PutStringValue(AVSourceTrackFormat::TRACK_TYPE, av_get_media_type_string(stream->codecpar->codec_type));
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss type info in track %{public}d", trackIndex);
    }

    ret = format.PutLongValue(AVSourceTrackFormat::TRACK_DURATION, stream->duration);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss duration info in track %{public}d", trackIndex);
    }
        
    ret = format.PutIntValue(AVSourceTrackFormat::VIDEO_TRACK_WIDTH, stream->codecpar->width);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss width info in track %{public}d", trackIndex);
    }

    ret = format.PutIntValue(AVSourceTrackFormat::VIDEO_TRACK_HEIGHT, stream->codecpar->height);
    if ( ret != AVCS_ERR_OK ) {
        AVCODEC_LOGW("Get track info failed:  miss height info in track %{public}d", trackIndex);
    }

    AVDictionaryEntry *pair = av_dict_get(stream->metadata, "rotate", nullptr, 0);
    if ( pair!=nullptr ) {
        ret = format.PutIntValue(AVSourceTrackFormat::VIDEO_TRACK_ROTATION, std::atoi(pair->value));
        if ( ret != AVCS_ERR_OK ) {
            AVCODEC_LOGW("Get track info failed:  miss rotate info in track %{public}d", trackIndex);
        }
    } else {
        AVCODEC_LOGW("Get track info failed:  miss rotate info in track %{public}d", trackIndex);
    }
    return AVCS_ERR_OK;
}


uintptr_t Source::GetSourceAddr()
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_INVALID_OPERATION, "GetSourceAddr failed, formatContext_ is nullptr!");

    return (uintptr_t)(formatContext_.get());
}


int32_t Source::Create(std::string& uri)
{
    AVCODEC_LOGI("Source::Create is called");
    int32_t ret = LoadDynamicPlugin(uri);
    CHECK_AND_RETURN_RET_LOG(ret != AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when load source plugin!");

    std::shared_ptr<MediaSource> mediaSource = std::make_shared<MediaSource>(uri);
    AVCODEC_LOGD("mediaSource Init: %{public}s", mediaSource->GetSourceUri().c_str());
    if( sourcePlugin_==nullptr ){
        AVCODEC_LOGE("load sourcePlugin_ fail !");
        return AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED;
    }

    Status pluginRet = sourcePlugin_->SetSource(mediaSource);
    CHECK_AND_RETURN_RET_LOG(pluginRet == Status::OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when set data source for plugin!");

    ret = LoadDemuxerList();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when load demuxerlist!");

    ret = SniffInputFormat(uri);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when find input format!");
    CHECK_AND_RETURN_RET_LOG(inputFormat_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when find input format, cannnot match any input format!");

    ret = InitAVFormatContext();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when parse source info!");
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when init AVFormatContext!");

    return AVCS_ERR_OK;
}


int32_t Source::LoadDemuxerList() 
{
    const AVInputFormat* plugin = nullptr;
    void* i = nullptr;
    while ((plugin = av_demuxer_iterate(&i))) {
        if (plugin->long_name != nullptr) {
            if (!strncmp(plugin->long_name, "pcm ", 4)) {
                continue;
            }
        }
        if (!IsInputFormatSupported(plugin->name)) {
            continue;
        }
        std::string pluginName = "avdemux_" + std::string(plugin->name);
        ReplaceDelimiter(".,|-<> ", '_', pluginName);
        g_pluginInputFormat[pluginName] =
            std::shared_ptr<AVInputFormat>(const_cast<AVInputFormat*>(plugin), [](void*) {});
    }
    if (g_pluginInputFormat.empty()) {
        AVCODEC_LOGW("cannot load any format demuxer");
        return AVCS_ERR_INVALID_OPERATION;
    }
    return AVCS_ERR_OK;
}


int32_t Source::LoadDynamicPlugin(const std::string& path)
{   
    AVCODEC_LOGI("LoadDynamicPlugin: %{public}s", path.c_str());
    std::string protocol;
    if (!ParseProtocol(path, protocol)) {
        AVCODEC_LOGE("Couldn't find valid protocol for %{public}s", path.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }

    if (pluginMap.count(protocol) == 0) {
        AVCODEC_LOGE("Unsupport protocol: %{public}s", protocol.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }

    std::string libFileName = pluginMap[protocol];
    std::string filePluginPath = OH_FILE_PLUGIN_PATH + g_fileSeparator + libFileName;
    std::string pluginName =
        libFileName.substr(g_libFileHead.size(), libFileName.size() - g_libFileHead.size() - g_libFileTail.size());
    
    std::string registerFuncName = "register_" + pluginName;
    RegisterFunc registerFunc = OpenFilePlugin(filePluginPath, pluginName);
    if (registerFunc) {
        register_ = std::make_shared<FfmpegRegister>();
        registerFunc(register_);
        sourcePlugin_ = register_->sourcePlugin;
        AVCODEC_LOGD("regist source plugin successful");
        return AVCS_ERR_OK;
    } else {
        AVCODEC_LOGD("regist source plugin failed, sourcePlugin path: %{public}s", filePluginPath.c_str());
        return AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED;
    }

}

int32_t Source::GuessInputFormat(const std::string& uri, std::shared_ptr<AVInputFormat> &bestInputFormat)
{
    std::string uriSuffix = GetUriSuffix(uri);
    if (uriSuffix.empty()) {
        AVCODEC_LOGW("can't found suffix ,please check the file %{public}s's suffix",uri.c_str());
        return AVCS_ERR_INVALID_OPERATION;
    }
    std::map<std::string, std::shared_ptr<AVInputFormat>>::iterator iter;
    for (iter = g_pluginInputFormat.begin(); iter != g_pluginInputFormat.end(); iter++) {
        std::shared_ptr<AVInputFormat> inputFormat = iter->second;
        if (uriSuffix == inputFormat->name) {
            bestInputFormat = inputFormat;
            AVCODEC_LOGD("find input fromat successful: %{public}s", inputFormat->name);
            break;
        }
    }
    return AVCS_ERR_OK;
}

int32_t Source::SniffInputFormat(const std::string& uri)
{
    size_t bufferSize = DEFAULT_READ_SIZE;
    size_t fileSize = 0;
    if (!static_cast<int>(sourcePlugin_->GetSize(fileSize))) {
        bufferSize = (bufferSize < fileSize) ? bufferSize : fileSize;
    }

    std::vector<uint8_t> buff(bufferSize);
    auto bufferInfo = std::make_shared<Buffer>();
    auto bufferMemory = bufferInfo->WrapMemory(buff.data(), bufferSize, 0);
    if (bufferMemory==nullptr) {
       return AVCS_ERR_NO_MEMORY;
    }
    auto ret = static_cast<int>(sourcePlugin_->Read(bufferInfo, bufferSize));
    CHECK_AND_RETURN_RET_LOG(ret >= 0, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source service failed when probe source format!");

    AVProbeData probeData = {"", buff.data(), static_cast<int>(bufferSize), ""};

    constexpr int probThresh = 50;
    int maxProb = 0;
    std::map<std::string, std::shared_ptr<AVInputFormat>>::iterator iter;
    for (iter = g_pluginInputFormat.begin(); iter != g_pluginInputFormat.end(); iter++) {
        std::string vtype = iter->first;
        std::shared_ptr<AVInputFormat> inputFormat = iter -> second;
        if (inputFormat->read_probe) {
            auto prob = inputFormat->read_probe(&probeData);  
            if (prob > probThresh) {
                inputFormat_ = inputFormat;
                break;
            }
            if (prob > maxProb) {
                maxProb = prob;
                inputFormat_ = inputFormat;
            }
        }
    }
    if ( inputFormat_ == nullptr ){
        AVCODEC_LOGW("sniff input format failed, will guess one!");
        return GuessInputFormat(uri, inputFormat_);
    }
    return AVCS_ERR_OK;
}  

void Source::InitAVIOContext(int flags) 
{

    constexpr int bufferSize = 4096;
    customIOContext_.sourcePlugin = sourcePlugin_.get();
    Status pluginRet = sourcePlugin_->GetSize(customIOContext_.fileSize);
    if (pluginRet !=Status::OK){
         AVCODEC_LOGE("get file size failed when set data source for plugin!");
         return;
    }
    pluginRet = sourcePlugin_->SeekTo(0);
    if (pluginRet !=Status::OK){
         AVCODEC_LOGE("seek to 0 failed when set data source for plugin!");
         return;
    }
    // CHECK_AND_RETURN_RET_LOG(pluginRet == Status::OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when set data source for plugin!");

    customIOContext_.offset=0;
    customIOContext_.eof=false;
    auto buffer = static_cast<unsigned char*>(av_malloc(bufferSize));
    auto bufferVector = std::make_shared<Buffer>();
    customIOContext_.bufMemory = bufferVector;
    auto bufMemory = bufferVector->WrapMemory(buffer, bufferSize, 0);
    
    if (buffer == nullptr) {
        AVCODEC_LOGE("AllocAVIOContext failed to av_malloc...");
        return;

    }
    avioContext_ = avio_alloc_context(buffer, bufferSize, flags & AVIO_FLAG_WRITE, (void*)(&customIOContext_),
                                                  AVReadPacket, NULL, AVSeek);
    customIOContext_.avioContext = avioContext_;
    if (avioContext_ == nullptr) {
        AVCODEC_LOGE("AllocAVIOContext failed to avio_alloc_context...");
        av_free(buffer);

    }
    Seekable seekable = sourcePlugin_->GetSeekable();
    AVCODEC_LOGD("seekable_ is %{public}d", (int)seekable);
    avioContext_->seekable = (seekable == Seekable::SEEKABLE) ? AVIO_SEEKABLE_NORMAL : 0;
    if (!(static_cast<uint32_t>(flags) & static_cast<uint32_t>(AVIO_FLAG_WRITE))) {
        avioContext_->buf_ptr = avioContext_->buf_end;
        avioContext_->write_flag = 0;
    }
}

int64_t Source::AVSeek(void *opaque,int64_t offset, int whence)
{
    auto customIOContext = static_cast<CustomIOContext*>(opaque);    
    uint64_t newPos = 0;
   
    switch (whence) {
        case SEEK_SET:
            newPos = static_cast<uint64_t>(offset);
            customIOContext->offset = newPos;
            break;
        case SEEK_CUR:
            newPos = customIOContext->offset + offset;
            break;
        case SEEK_END:
        case AVSEEK_SIZE: {
            size_t mediaDataSize = 0;
            customIOContext->sourcePlugin->GetSize(mediaDataSize);
            if (mediaDataSize > 0) {
                newPos = mediaDataSize + offset;
            }
            break;
        }
        default:
            AVCODEC_LOGW("AVSeek unexpected whence: %{oublic}d", whence);
            break;
    }

    if (whence != AVSEEK_SIZE) {
        customIOContext->offset = newPos;
    }
    return newPos;
}

int Source::AVReadPacket(void *opaque, uint8_t *buf, int bufSize)
{
    int rtv = -1;
    auto readSize = bufSize;
    auto customIOContext = static_cast<CustomIOContext*>(opaque);
    auto bufferVector = customIOContext->bufMemory;

    if ((customIOContext->avioContext->seekable == (int) Seekable::SEEKABLE)&&(customIOContext->fileSize!=0)) {
        if ( customIOContext->offset > customIOContext->fileSize) {
            AVCODEC_LOGW("ERROR: offset: %{public}zu is larger than totalSize: %{public}zu" ,customIOContext->offset, customIOContext->fileSize);
            return AVCS_ERR_SEEK_FAILED;
        }
        if( static_cast<size_t>(customIOContext->offset+bufSize) > customIOContext->fileSize) {
            readSize = customIOContext->fileSize - customIOContext->offset;
        }
        if (buf!=nullptr && bufferVector->GetMemory()!=nullptr) {
            auto memSize = static_cast<int>(bufferVector->GetMemory()->GetCapacity());
            readSize = (readSize > memSize) ? memSize : readSize;
        }
        if (customIOContext->position != customIOContext->offset){
            int err = (int)customIOContext->sourcePlugin->SeekTo(customIOContext->offset);
            if(err < 0){
                AVCODEC_LOGD("ERROR: Seek to %{public}zu fail,err=%{public}d\n", customIOContext->offset,err);
                return AVCS_ERR_SEEK_FAILED;
            }
            customIOContext->position = customIOContext->offset;
        }

        int result =static_cast<int>(customIOContext->sourcePlugin->Read(bufferVector, static_cast<size_t>(readSize)));
        AVCODEC_LOGD("AVReadPacket read data size = %{public}d", static_cast<int>(bufferVector->GetMemory()->GetSize()));
        if (result == 0) {
            rtv = bufferVector->GetMemory()->GetSize();

            customIOContext->offset += rtv;
            customIOContext->position += rtv;
            
        } else if (static_cast<int>(result) == 1) { 
            customIOContext->eof=true;
            rtv = AVERROR_EOF;
        } else {
            AVCODEC_LOGE("AVReadPacket failed with rtv = %{public}d", static_cast<int>(result));
        }
    }

    return rtv;
}

int32_t Source::InitAVFormatContext()
{
    AVFormatContext *formatContext = avformat_alloc_context();
    if (formatContext == nullptr) {
        AVCODEC_LOGE("InitAVFormatContext failed, because  alloc AVFormatContext failed.");
        return AVCS_ERR_INVALID_OPERATION;
    }
    InitAVIOContext(AVIO_FLAG_READ);
    if (avioContext_==nullptr ) {
        AVCODEC_LOGE("InitAVFormatContext failed, because  init AVIOContext failed.");
        return AVCS_ERR_INVALID_OPERATION;
    }

    formatContext->pb = avioContext_;
    formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

    int ret = -1;
    ret = avformat_open_input(&formatContext, nullptr, inputFormat_.get(), nullptr);

    if (ret == 0) {
        formatContext_ = std::shared_ptr<AVFormatContext>(formatContext, [](AVFormatContext* ptr) {
            if (ptr) {
                auto ctx = ptr->pb;
                if (ctx) {
                    av_freep(&ctx->buffer);
                    av_free(ctx);
                }
            }
        });
    } else {
        AVCODEC_LOGE("avformat_open_input failed by %{public}s", inputFormat_->name);
        return AVCS_ERR_INVALID_OPERATION;
    }
    return AVCS_ERR_OK;
}



// Source::~Source()
// {   
//     if (state_ == STARTED) {
//         que_.SetActive(false);
//         StopThread();
//     }
//     state_ = STOPPED;
//     que_.SetActive(false, false);

//     formatContext_ = nullptr;
//     inputFormat_ = nullptr;
//     sourcePlugin_ = nullptr;
//     register_ = nullptr;
//     avioContext_ = nullptr;
//     AVCODEC_LOGI("Source::~Source is on call");
// }

// int32_t Source::StartThread(std::string name)
// {
//     threadName_ = name;
//     if (thread_ != nullptr) {
//         AVCODEC_LOGW("Started already! [%{public}s]", threadName_.c_str());
//         return AVCS_ERR_OK;
//     }
//     isThreadExit_ = false;
//     thread_ = std::make_unique<std::thread>(&Source::ReadSourceLoop, this);
//     AVCodecTrace::TraceBegin("demuxer_write_thread", FAKE_POINTER(thread_.get()));
//     AVCODEC_LOGD("thread started! [%{public}s]", threadName_.c_str());
//     return AVCS_ERR_OK;
// }

// int32_t Source::StopThread() noexcept
// {
//     if (isThreadExit_) {
//         AVCODEC_LOGD("Stopped already! [%{public}s]", threadName_.c_str());
//         return AVCS_ERR_OK;
//     }
//     if (std::this_thread::get_id() == thread_->get_id()) {
//         AVCODEC_LOGD("Stop at the task thread, reject");
//         return AVCS_ERR_INVALID_OPERATION;
//     }

//     std::unique_ptr<std::thread> t;
//     isThreadExit_ = true;
//     cond_.notify_all();
//     std::swap(thread_, t);
//     if (t != nullptr && t->joinable()) {
//         t->join();
//     }
//     thread_ = nullptr;
//     return AVCS_ERR_OK;
// }

// void Source::ReadSourceLoop()
// {
//     auto bufferVector = std::make_shared<Buffer>();
//     int result =static_cast<int>(customIOContext->sourcePlugin->Read(bufferVector, static_cast<size_t>(readSize)));
//     que_.Push(bufferVector);
// }

// int32_t Source::Create(std::string& uri)
// {
//     AVCODEC_LOGI("Source::Create is called");
//     int32_t ret = LoadDynamicPlugin(uri);
//     CHECK_AND_RETURN_RET_LOG(ret != AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when load source plugin!")

//     std::shared_ptr<MediaSource> mediaSource = std::make_shared<MediaSource>(uri);
//     AVCODEC_LOGD("mediaSource Init: %{public}s", mediaSource->GetSourceUri().c_str());
//     if( sourcePlugin_==nullptr ){
//         AVCODEC_LOGE("load sourcePlugin_ fail !");
//         return AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED;
//     }

//     Status pluginRet = sourcePlugin_->SetSource(mediaSource);
//     CHECK_AND_RETURN_RET_LOG(pluginRet == Status::OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when set data source for plugin!")

//     ret = LoadDemuxerList();
//     CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when load demuxerlist!")

//     ret = SniffInputFormat(uri);
//     CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when find input format!")
//     CHECK_AND_RETURN_RET_LOG(inputFormat_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when find input format, cannnot match any input format!")

//     ret = InitAVFormatContext();
//     CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when parse source info!")
//     CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, AVCS_ERR_CREATE_SOURCE_SUB_SERVICE_FAILED, "create source failed when init AVFormatContext!")

//     state_ = STARTED;
//     StartThread("source_read_loop");

//     return AVCS_ERR_OK;
// }

// int Source::AVReadPacket(void *opaque, uint8_t *buf, int bufSize)
// {
//     int rtv = -1;
//     auto readSize = bufSize;
//     auto customIOContext = static_cast<CustomIOContext*>(opaque);
    

//     if ((customIOContext->avioContext->seekable == (int) Seekable::SEEKABLE)&&(customIOContext->fileSize!=0)) {
//         if ( customIOContext->offset > customIOContext->fileSize) {
//             AVCODEC_LOGW("ERROR: offset: %{public}zu is larger than totalSize: %{public}zu" ,customIOContext->offset, customIOContext->fileSize);
//             return AVCS_ERR_SEEK_FAILED;
//         }
//         if( static_cast<size_t>(customIOContext->offset+bufSize) > customIOContext->fileSize) {
//             readSize = customIOContext->fileSize - customIOContext->offset;
//         }
//         if (buf!=nullptr && bufferVector->GetMemory()!=nullptr) {
//             auto memSize = static_cast<int>(bufferVector->GetMemory()->GetCapacity());
//             readSize = (readSize > memSize) ? memSize : readSize;
//         }
//         if (customIOContext->position != customIOContext->offset){
//             int err = (int)customIOContext->sourcePlugin->SeekTo(customIOContext->offset);
//             if(err < 0){
//                 AVCODEC_LOGD("ERROR: Seek to %{public}zu fail,err=%{public}d\n", customIOContext->offset,err);
//                 return AVCS_ERR_SEEK_FAILED;
//             }
//             customIOContext->position = customIOContext->offset;
//             que_.clear();
//             ReadSourceLoop();
//         }
//         // AVCODEC_LOGD("AVReadPacket read data size = %{public}d", static_cast<int>(bufferVector->GetMemory()->GetSize()));
//         // if (result == 0) {
//         //     rtv = bufferVector->GetMemory()->GetSize();

//         //     customIOContext->offset += rtv;
//         //     customIOContext->position += rtv;
            
//         // } else if (static_cast<int>(result) == 1) { 
//         //     customIOContext->eof=true;
//         //     rtv = AVERROR_EOF;
//         // } else {
//         //     AVCODEC_LOGE("AVReadPacket failed with rtv = %{public}d", static_cast<int>(result));
//         // }
//     }

//     auto bufferVector = que_.Pop();

//     return rtv;
// }

} // namespace Media
} // namespace OHOS