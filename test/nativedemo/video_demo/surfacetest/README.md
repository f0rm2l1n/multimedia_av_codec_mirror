# surface_test
## 环境
+ OpenHarmony 3.2 Release
+ DevEco Studio 3.1
+ SDK版本：API version 9 (SDK9)
+ 支持设备：RK3568

## 烧录镜像
+ 下载OpenHarmony 3.2 Release，并把napitest目录放置在openharmony的fundation目录中
+ 在/build/subsystem_config.json中增加子系统选项
```
"napitest": {
    "project": "hmf/napitest",
    "path": "foundation/napitest",
    "name": "napitest",
    "dir": "foundation"
  }
```
+ 在/productdefine/common/base/base_product.json中增加功能模块
```
{
    "subsystem": "napitest",
    "components": [
       {
          "component": "napitest_interface",
          "features": []
       }
    ]
}
```
+ 在/productdefine/common/inherit/_ohos_common_parts.json增加功能模块
```
"napitest:napitest_interface":{}
```
+ 在/productdefine/common/inherit/base.json中增加功能模块
```
{
    "subsystem": "napitest",
    "components": [
       {
          "component": "napitest_interface",
          "features": []
       }
    ]
}
```
+ 在/productdefine/common/inherit/rich.json中增加功能模块
```
{
    "subsystem": "napitest",
    "components": [
       {
          "component": "napitest_interface",
          "features": []
       }
    ]
}
```
+ 编译镜像并烧录
(注: 后续只需替换/lib/module/libnapitest.z.so即可，不需重走烧录流程)

## 编译库
+ 编译libfcodec.z.so，并推送至板子的/system/lib目录

## 启动hap
+ 将@ohos.napitest.d.ts文件放置在SDK9目录下
```
C:\Users\95234\AppData\Local\OpenHarmony\Sdk\9\ets\api
```
+ 使用DevEco Studio打开surfaceTest目录
+ 安装surfaceTest，并测试即可



