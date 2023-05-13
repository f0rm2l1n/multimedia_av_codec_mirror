# av_codec

## 代码目录

```
/foundation/multimedia/av_codec   # codec原子能力代码仓
|---interfaces                    # 北向接口层
|---frameworks                    # 框架层
|   |---codeclist                 # 编解码能力查询北向接口实现
|   |---codec                     # 编解码北向接口实现
|   |---muxer              	      # 封装北向接口实现
|   |---demuxer                   # 解封装北向接口实现
|   |---common            	      # 框架公用资源
|---services                 	  # 服务C/S实现
|   |---utils                     # 服务基础资源
|   |---dfx                       # dfx实现
|   |---services                  # 服务框架
|   |    |---codeclist            # 编解码能力查询C/S框架
|   |    |---codec                # 编解码C/S框架
|   |    |---muxer                # 封装C/S框架
|   |    |---demuxer              # 解封装C/S框架
|   |    |---factory              # 引擎工厂
|   |    |---sa_avcodec           # avcodec主进程C/S框架
|   |    |---common               # 服务公用资源
|   |---engine                    # 引擎实现
|   |   |---base                  # 内部接口base文件
|   |   |   |---include           # base头文件
|   |   |---common           	  # 引擎公用资源
|   |   |---factory               # 引擎工厂实现
|   |   |---codeclist             # 编解码能力查询引擎
|   |   |---codec                 # 编解码引擎
|   |   |---muxer                 # 封装引擎
|   |   |---demuxer               # 解封装引擎
|   |   |---plugin                # 插件
|   |   |   |---interfaces        # 插件接口
|   |   |   |---core              # 插件框架
|   |   |   |---plugins           # 插件实现
|   |   |   |   |---codec         # 编解码插件
|   |   |   |   |---muxer         # 封装插件
|   |   |   |   |---demuxer       # 解封装插件
|   |   |   |---common    	      # 插件公用资源
|   |---etc                       # 其它文件，如codec_caps.xml 
|---test                          # 测试代码
```

