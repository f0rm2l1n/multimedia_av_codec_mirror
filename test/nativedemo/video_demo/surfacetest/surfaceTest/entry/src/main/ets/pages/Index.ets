import mediaLibrary from '@ohos.multimedia.mediaLibrary';
import napitest from '@ohos.napitest';
import Logger from '../model/Logger';
import MediaLibraryUtils from '../model/MediaLibraryUtils';
import DateTimeUtil from '../model/DateTimeUtils';

const TAG: string = 'FCodec';

@Entry
@Component
struct Index {
  @State ratio: number = 1.0;
  @State mediaLibUtils: MediaLibraryUtils = new MediaLibraryUtils();
  @State videoName: string = 'bigbuckbunny_480x272.h264';
  private videoWidth: number = 480;
  private videoHeight: number = 272;
  private lastSaveTime = '';
  private saveIndex = 0;
  private VideoOutFile: any;
  private fileDescriptor: napitest.AVFileDescriptor;
  private outFd: number = -1;
  private surfaceID: string = '';
  private mXcomponentController: XComponentController = new XComponentController();

  // 播放文件打开函数
  async openMediaFile(fileName: string): Promise<napitest.AVFileDescriptor> {
    let testFileDescriptor = await this.mediaLibUtils.openFileDescriptor(fileName);
    return testFileDescriptor;
  }

  private checkName(name: string): string {
    if (this.lastSaveTime == name) {
      this.saveIndex += 1;
      return `${name}_${this.saveIndex}`
    }
    this.lastSaveTime = name;
    this.saveIndex = 0;
    Logger.info(TAG, 'get video File name is: ' + name);
    return name;
  }

  // 创建输出fd
  async createVideoFd(): Promise<number> {
    const filePrefix = this.videoName.substring(0, this.videoName.lastIndexOf("."))
    const mDateTimeUtil = new DateTimeUtil();
    const displayName = this.checkName(`SFE_${mDateTimeUtil.getDate()}_${mDateTimeUtil.getTime()}`) + filePrefix + '.yuv';
    Logger.info(TAG, 'get video display name is: ' + displayName);

    const media = mediaLibrary.getMediaLibrary(globalThis.abilityContext)
    const fileKeyObj = mediaLibrary.FileKey;
    const mediaType = mediaLibrary.MediaType.FILE;
    // "/storage/media/100/local/files/Documents"
    let publicPath: string = await media.getPublicDirectory(mediaLibrary.DirectoryType.DIR_DOCUMENTS);
    Logger.info(TAG, 'get video File publicPath: ' + publicPath);

    try {
      const dataUri = await media.createAsset(mediaType, displayName, publicPath);

      if (dataUri != undefined) {
        const args = dataUri.id.toString();
        const fetchOp = {
          selections: fileKeyObj.ID + "=?",
          selectionArgs: [args],
        }
        const fetchFileResult = await media.getFileAssets(fetchOp);
        this.VideoOutFile = await fetchFileResult.getFirstObject();
        const fdNumber = await this.VideoOutFile.open('rw');
        Logger.info(TAG, 'get video File fd is: ' + fdNumber);
        return fdNumber
      }
    } catch (err) {
      Logger.info(TAG, 'create video File Fd ERROR: ' + err);
    }
  }

  // 初始化函数
  async initFcodec(): Promise<void> {
    Logger.info(TAG, 'initFcodec success');

    // surfaceID
    this.surfaceID = this.mXcomponentController.getXComponentSurfaceId();
    Logger.info(TAG, 'surfaceID is : ' + this.surfaceID);
  }

  async prepareFile(): Promise<void> {
    // 输入文件
    this.fileDescriptor = await this.openMediaFile(this.videoName);
    Logger.info(TAG, 'inFd: ' + this.fileDescriptor.fd + ', inOffset: ' + this.fileDescriptor.offset + ', inLen: ' + this.fileDescriptor.length);

    // 输出文件
    this.outFd = await this.createVideoFd();
    Logger.info(TAG, 'outUrl is: ' + this.outFd);
  }

  build() {
    Stack({ alignContent: Alignment.Bottom }) {
      Row() {
        // 当播放的是非音频时，加载XComponent控件
        XComponent({
          id: 'componentId',
          type: 'surface',
          controller: this.mXcomponentController
        })
          .onLoad(() => {
            // 加载完成后调用初始化播放器函数
            this.initFcodec();
          })
          .width('100%')
          .aspectRatio(this.ratio)
      }
      .height('80%')
      .width('100%')
      .justifyContent(FlexAlign.Center)
      .position({ x: 0, y: 0 })
      .alignItems(VerticalAlign.Top)

      Column() {
        Row() {
          Text(this.videoName).fontSize(15).width('50%').textAlign(TextAlign.Center)
        }
        .alignItems(VerticalAlign.Center)

        Row() {
          // 添加按钮，以响应用户点击
          Button() {
            Text('Start')
              .fontSize(30)
              .fontWeight(FontWeight.Bold)
          }
          .type(ButtonType.Capsule)
          .margin({
            top: 20
          })
          .backgroundColor('#0D9FFB')
          .width('40%')
          .height('5%')
          .onClick(() => {
            this.prepareFile()
              .then( rlt => {
                Logger.info(TAG, 'surfaceID is ' + this.surfaceID);
                return napitest.setSurfaceID(this.surfaceID, this.fileDescriptor, this.outFd, this.videoWidth, this.videoHeight)
              })
              .then(surfaceID => {
                Logger.info(TAG, 'set surfaceID is ' + surfaceID);
                return napitest.getSurfaceID()
              })
              .then(surfaceID => {
                Logger.info(TAG, 'get surfaceID is ' + surfaceID);
              })
              .catch(error => {
                Logger.info(TAG, 'get surfaceID error: ' + error);
              })
          })
        }
        .alignItems(VerticalAlign.Center)

      }
      .justifyContent(FlexAlign.Start)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }
}