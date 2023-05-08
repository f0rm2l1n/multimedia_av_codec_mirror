 av_codec部件

## 简介

av_codec部件为系统提供了统一的音视频编解码、封装、解封装能力，使得开发者能够直接调用软硬件编解码器。

av_codec部件提供了以下常用功能：
- 视频编码
- 视频解码
- 视频封装
- 视频解封装
- 音频编码
- 音频解码
- 音频封装
- 音频解封装

**图 1** av_codec部件架构图



## 目录

仓目录结构如下：

```
/foundation/multimedia/av_codec     # av_codec部件业务代码
├── BUILD.gn                        # 编译入口
├── bundle.json                     # 部件描述文件
├── frameworks                      # 部件无独立进程框架代码的实现
│   └── native                      # native c++实现
├── interfaces                      # 外部接口层
│   ├── inner_api                   # 系统内部件接口
│   └── kits                        # 应用接口
├── sa_profile                      # 部件配置
├── services                        # 服务C/S实现
│   ├── dfx                         # dfx代码
│   ├── engine                      # 功能实现
│   │   ├── base                    # 功能基类
│   │   ├── codec                   # 编解码功能实现
│   │   ├── codeclist               # 编解码能力查询功能实现
│   │   ├── common                  # 功能实现通用资源
│   │   ├── demuxer                 # 解封装功能实现
│   │   ├── factory                 # 功能实现工厂
│   │   ├── muxer                   # 封装功能实现
│   │   ├── plugin                  # 插件实现
│   │   └── source                  # 源读取器功能实现
│   ├── etc                         # 部件进程配置
│   ├── include                     # 服务对外头文件
│   ├── services                    # 服务框架
│   │   ├── codec                   # 编解码C/S框架
│   │   ├── codeclist               # 编解码能力查询C/S框架
│   │   ├── common                  # 服务框架通用资源
│   │   ├── demuxer                 # 解封装C/S框架
│   │   ├── factory                 # 服务框架工厂
│   │   ├── muxer                   # 封装C/S框架
│   │   ├── sa_avcodec              # 部件主进程C/S框架
│   │   └── source                  # 源读取器C/S框架
│   └── utils                       # 服务框架通用资源
└── test                            # 测试代码
```


## 编译构建

编译32位ARM系统av_codec部件
```
./build.sh --product-name {product_name} --ccache --build-target multimedia_av_codec
```

编译64位ARM系统av_codec部件
```
./build.sh --product-name {product_name} --ccache --target-cpu arm64 --build-target multimedia_av_codec
```

## 相关仓

