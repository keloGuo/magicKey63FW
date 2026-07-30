//页面加载完成，获取当前层和，当前层数据
//层按键，设置当前激活层，并返回当前激活层按钮

const MAGIC63_SETTINGS_PAGE_VERSION = "20260730.2";
var magic63Lang = "zh";
var magic63LastFsInfo = null;
var magic63LastVersionInfo = null;

var MAGIC63_I18N = {
    zh: {
        languageToggle: "English",
        showKeyboardPage: "键盘",
        showMacroPage: "宏",
        showBounceDiagPage: "轴抖动",
        showUploadPage: "上传页",
        deviceReboot: "重启设备",
        showVersionInfo: "版本信息",
        versionPopupTitle: "版本信息",
        versionDefault: "版本 --",
        fsDefault: "FS --",
        macroPageTitle: "宏编辑",
        macroModePress: "按下触发",
        macroModeLoop: "按下循环",
        macroNew: "新建宏",
        macroDelete: "删除宏",
        macroSave: "保存",
        macroActionsTitle: "动作",
        macroAddAction: "添加按键",
        macroAddDelay: "添加延时",
        macroDeleteAction: "删除按键",
        macroDeleteDelay: "删除延时",
        macroTypeDown: "按下",
        macroTypeTap: "按下松开",
        macroTypeUp: "松开",
        macroListTitle: "宏列表",
        macroKeySelectTitle: "按键选择",
        macroActionUnit: " 动作",
        macroUnsaved: "未保存",
        macroCountUnit: " 个",
        bouncePageTitle: "机械轴抖动检测",
        bounceDiagStart: "重新检测",
        bounceDiagCancel: "取消检测",
        bounceStepIdleNo: "步骤 1",
        bounceStepIdleTitle: "按任意键开始检测",
        bounceStepIdleText: "进入页面后设备会暂停键盘上报，第一次按下的按键就是本次检测目标。",
        bounceStepSelectNo: "步骤 2",
        bounceStepSelectTitle: "等待目标按键",
        bounceStepSelectText: "按下任意机械轴，页面会将这个按键作为本次检测目标。",
        bounceStepReleaseNo: "步骤 3",
        bounceStepReleaseTitle: "松开按键并等待稳定",
        bounceStepReleaseText: "设备会等待所有列信号释放并稳定 300ms，然后才开始正式采集。",
        bounceStepPressNo: "步骤 4",
        bounceStepPressTitle: "正式按下测试",
        bounceStepPressText: "再次按下同一个按键并保持到页面提示松开，PIO 会记录按下过程中的跳变次数和间隔。",
        bounceStepDoneNo: "步骤 5",
        bounceStepDoneTitle: "松开并查看结果",
        bounceStepDoneText: "页面提示松开后正常松开按键，完成后会显示按下和松开两段抖动数据。",
        bounceMetricStateLabel: "当前状态",
        bounceMetricKeyLabel: "目标按键",
        bounceMetricEdgesLabel: "按下跳变",
        bounceMetricReleaseEdgesLabel: "松开跳变",
        bounceMetricPressTotalLabel: "按下到稳定",
        bounceMetricReleaseTotalLabel: "松开到稳定",
        bounceIntervalTitle: "按下跳变间隔",
        bounceReleaseIntervalTitle: "松开跳变间隔",
        bounceHistoryTitle: "本页历史记录",
        bounceState0: "等待开始",
        bounceState1: "步骤 1：请按任意键开始检测",
        bounceState2: "步骤 3：请松开按键，等待设备确认释放稳定",
        bounceState3: "步骤 4：请正式按下目标按键并保持",
        bounceState4: "正在采集：请继续保持按下",
        bounceState5: "步骤 5：按下已稳定，请松开目标按键",
        bounceState6: "正在采集松开抖动",
        bounceState7: "检测完成，按任意键继续",
        bounceState8: "检测异常",
        bounceStateUnknown: "未知状态",
        bounceKeyPos: "第 {row} 行，第 {col} 列",
        bounceTitleMore: "{title}（共 {total} 次，显示前 {shown} 次）",
        bounceNoIntervals: "暂无跳变间隔",
        bounceIntervalItem: "第 {index} 次 {value}us",
        bounceMoreIntervals: "其余 {count} 次未缓存",
        bounceNoHistory: "暂无历史记录",
        bounceHistoryPress: "按下抖动",
        bounceHistoryRelease: "松开抖动",
        bounceHistoryPressStable: "按下稳定",
        bounceHistoryReleaseStable: "松开稳定",
        deviceConnectFail: "无法连接设备",
        gifUploadTitle: "GIF 上传",
        gifUploadInfo: "160x80，最大 512K",
        gifUploadState: "等待选择文件",
        waitTestResult: "等待测试结果",
        gifUploadStart: "上传 GIF",
        gifUploadCancel: "取消",
        gifCanceled: "已取消",
        gifDone: "上传完成",
        gifUploading: "上传中 {percent}%",
        gifReadFail: "读取失败",
        gifUploadFail: "上传失败",
        gifWriteFail: "写入失败",
        gifSelect: "请选择 GIF",
        gifTypeFail: "文件类型错误",
        gifTooLarge: "超过 512K",
        gifResolutionFail: "分辨率必须 160x80",
        gifStartFail: "启动失败",
        fsInfo: "FS 剩余 {free} / {total}",
        versionInfo: "固件：{firmware}<br>配置：{config}<br>USB：{usb}<br>设置页：{settings}<br>上传页：{upload}",
        encoderLeft: "编码器左",
        encoderPress: "编码器按下",
        encoderRight: "编码器右",
        volDown: "音量减",
        mute: "静音",
        volUp: "音量加",
        media: "播放器",
        prevTrack: "上一曲",
        playPause: "暂停/播放",
        nextTrack: "下一曲",
        stop: "停止",
        browser: "浏览器",
        mouseLeft: "鼠标左键",
        mouseMiddle: "鼠标中键",
        mouseRight: "鼠标右键",
        wheelUp: "滚轮上",
        wheelDown: "滚轮下",
        statusPage: "状态页",
        layerPage: "层页",
        lightPage: "灯页",
        ratePage: "速率页",
        codexPage: "Codex页",
        macroRecPage: "录宏页",
        apmPage: "APM页",
        meritPage: "功德页",
        networkPage: "网络页",
        bootPage: "Boot页",
        statusInner: "状态内",
        layerInner: "层内",
        lightInner: "灯内",
        rateInner: "速率内",
        codexInner: "Codex内",
        macroRecInner: "录宏内",
        meritInner: "功德内",
        networkInner: "网络内"
    },
    en: {
        languageToggle: "中文",
        showKeyboardPage: "Keyboard",
        showMacroPage: "Macro",
        showBounceDiagPage: "Bounce",
        showUploadPage: "Upload",
        deviceReboot: "Reboot",
        showVersionInfo: "Version",
        versionPopupTitle: "Version Info",
        versionDefault: "Version --",
        fsDefault: "FS --",
        macroPageTitle: "Macro Editor",
        macroModePress: "Press Trigger",
        macroModeLoop: "Press Loop",
        macroNew: "New Macro",
        macroDelete: "Delete Macro",
        macroSave: "Save",
        macroActionsTitle: "Actions",
        macroAddAction: "Add Key",
        macroAddDelay: "Add Delay",
        macroDeleteAction: "Delete Key",
        macroDeleteDelay: "Delete Delay",
        macroTypeDown: "Down",
        macroTypeTap: "Tap",
        macroTypeUp: "Up",
        macroListTitle: "Macro List",
        macroKeySelectTitle: "Key Picker",
        macroActionUnit: " actions",
        macroUnsaved: "Unsaved",
        macroCountUnit: " item(s)",
        bouncePageTitle: "Switch Bounce Test",
        bounceDiagStart: "Retest",
        bounceDiagCancel: "Cancel",
        bounceStepIdleNo: "Step 1",
        bounceStepIdleTitle: "Press any key to start",
        bounceStepIdleText: "The device pauses keyboard reports. The first pressed switch becomes the test target.",
        bounceStepSelectNo: "Step 2",
        bounceStepSelectTitle: "Waiting for target key",
        bounceStepSelectText: "Press one mechanical switch. This action only selects the target and is not part of the final result.",
        bounceStepReleaseNo: "Step 3",
        bounceStepReleaseTitle: "Release and wait for stable",
        bounceStepReleaseText: "The device waits until all column signals are released and stable for 300 ms before sampling.",
        bounceStepPressNo: "Step 4",
        bounceStepPressTitle: "Press test",
        bounceStepPressText: "Press the same key again and hold it until the page asks you to release. PIO records edge count and intervals.",
        bounceStepDoneNo: "Step 5",
        bounceStepDoneTitle: "Release and view result",
        bounceStepDoneText: "Release the key normally. The page shows both press and release bounce data after completion.",
        bounceMetricStateLabel: "State",
        bounceMetricKeyLabel: "Target",
        bounceMetricEdgesLabel: "Press edges",
        bounceMetricReleaseEdgesLabel: "Release edges",
        bounceMetricPressTotalLabel: "Press stable",
        bounceMetricReleaseTotalLabel: "Release stable",
        bounceIntervalTitle: "Press edge intervals",
        bounceReleaseIntervalTitle: "Release edge intervals",
        bounceHistoryTitle: "Current Page History",
        bounceState0: "Waiting",
        bounceState1: "Step 1: press any key to start",
        bounceState2: "Step 3: release the key and wait for stable detection",
        bounceState3: "Step 4: press and hold the target key",
        bounceState4: "Sampling: keep holding the key",
        bounceState5: "Step 5: press is stable, release the target key",
        bounceState6: "Sampling release bounce",
        bounceState7: "Done, press any key to continue",
        bounceState8: "Test error",
        bounceStateUnknown: "Unknown state",
        bounceKeyPos: "Row {row}, Col {col}",
        bounceTitleMore: "{title} ({total} total, showing first {shown})",
        bounceNoIntervals: "No edge intervals",
        bounceIntervalItem: "#{index} {value}us",
        bounceMoreIntervals: "{count} more not cached",
        bounceNoHistory: "No history",
        bounceHistoryPress: "Press bounce",
        bounceHistoryRelease: "Release bounce",
        bounceHistoryPressStable: "Press stable",
        bounceHistoryReleaseStable: "Release stable",
        deviceConnectFail: "Device unreachable",
        gifUploadTitle: "GIF Upload",
        gifUploadInfo: "160x80, max 512K",
        gifUploadState: "Waiting for file",
        waitTestResult: "Waiting for result",
        gifUploadStart: "Upload GIF",
        gifUploadCancel: "Cancel",
        gifCanceled: "Canceled",
        gifDone: "Upload complete",
        gifUploading: "Uploading {percent}%",
        gifReadFail: "Read failed",
        gifUploadFail: "Upload failed",
        gifWriteFail: "Write failed",
        gifSelect: "Select a GIF",
        gifTypeFail: "Wrong file type",
        gifTooLarge: "Over 512K",
        gifResolutionFail: "Resolution must be 160x80",
        gifStartFail: "Start failed",
        fsInfo: "FS free {free} / {total}",
        versionInfo: "Firmware: {firmware}<br>Config: {config}<br>USB: {usb}<br>Settings: {settings}<br>Upload: {upload}",
        encoderLeft: "Encoder Left",
        encoderPress: "Encoder Press",
        encoderRight: "Encoder Right",
        volDown: "Vol-",
        mute: "Mute",
        volUp: "Vol+",
        media: "Media",
        prevTrack: "Previous",
        playPause: "Play/Pause",
        nextTrack: "Next",
        stop: "Stop",
        browser: "Browser",
        mouseLeft: "Mouse Left",
        mouseMiddle: "Mouse Middle",
        mouseRight: "Mouse Right",
        wheelUp: "Wheel Up",
        wheelDown: "Wheel Down",
        statusPage: "Status Page",
        layerPage: "Layer Page",
        lightPage: "Light Page",
        ratePage: "Rate Page",
        codexPage: "Codex Page",
        macroRecPage: "Macro Rec",
        apmPage: "APM Page",
        meritPage: "Merit Page",
        networkPage: "Network Page",
        bootPage: "Boot Page",
        statusInner: "Status Inner",
        layerInner: "Layer Inner",
        lightInner: "Light Inner",
        rateInner: "Rate Inner",
        codexInner: "Codex Inner",
        macroRecInner: "Macro Rec Inner",
        meritInner: "Merit Inner",
        networkInner: "Network Inner"
    },
};

var MAGIC63_LABEL_IDS = {
    keyId70: "encoderLeft",
    keyId79: "encoderPress",
    keyId71: "encoderRight",
    IDM1: "volDown",
    IDM2: "mute",
    IDM0: "volUp",
    IDM4: "media",
    IDM7: "prevTrack",
    IDM6: "playPause",
    IDM8: "nextTrack",
    IDM9: "stop",
    IDM5: "browser",
    IDS0: "mouseLeft",
    IDS2: "mouseMiddle",
    IDS1: "mouseRight",
    IDS3: "wheelUp",
    IDS4: "wheelDown",
    IDP1: "statusPage",
    IDP2: "layerPage",
    IDP3: "lightPage",
    IDP4: "ratePage",
    IDP5: "codexPage",
    IDP6: "macroRecPage",
    IDP7: "apmPage",
    IDP8: "meritPage",
    IDP9: "networkPage",
    IDP10: "bootPage",
    IDQ1: "statusInner",
    IDQ2: "layerInner",
    IDQ3: "lightInner",
    IDQ4: "rateInner",
    IDQ5: "codexInner",
    IDQ6: "macroRecInner",
    IDQ8: "meritInner",
    IDQ9: "networkInner"
};

function i18nText(key, data)
{
    var table = MAGIC63_I18N[magic63Lang] || MAGIC63_I18N.zh;
    var text = table[key] != null ? table[key] : (MAGIC63_I18N.zh[key] || key);
    if(data != null)
    {
        for(var k in data) text = text.replace(new RegExp("\\{" + k + "\\}", "g"), data[k]);
    }
    return text;
}

function i18nSetText(id, key)
{
    var el = document.getElementById(id);
    if(el != null) el.innerHTML = i18nText(key);
}

function i18nSetValue(id, key)
{
    var el = document.getElementById(id);
    if(el != null) el.value = i18nText(key);
}

function i18nApplyLabels()
{
    for(var id in MAGIC63_LABEL_IDS)
    {
        var el = document.getElementById(id);
        if(el == null) continue;
        var text = i18nText(MAGIC63_LABEL_IDS[id]);
        el.innerHTML = text;
        el.setAttribute("name", text);
    }
}

function i18nApply()
{
    i18nSetValue("languageToggle", "languageToggle");
    i18nSetValue("showKeyboardPage", "showKeyboardPage");
    i18nSetValue("showMacroPage", "showMacroPage");
    i18nSetValue("showBounceDiagPage", "showBounceDiagPage");
    i18nSetValue("showUploadPage", "showUploadPage");
    i18nSetValue("deviceReboot", "deviceReboot");
    i18nSetValue("showVersionInfo", "showVersionInfo");
    i18nSetText("versionPopupTitle", "versionPopupTitle");
    i18nSetText("macroPageTitle", "macroPageTitle");
    i18nSetText("macroModePress", "macroModePress");
    i18nSetText("macroModeLoop", "macroModeLoop");
    i18nSetValue("macroNew", "macroNew");
    i18nSetValue("macroDelete", "macroDelete");
    i18nSetValue("macroSave", "macroSave");
    i18nSetText("macroActionsTitle", "macroActionsTitle");
    i18nSetValue("macroAddAction", "macroAddAction");
    i18nSetValue("macroAddDelay", "macroAddDelay");
    i18nSetValue("macroDeleteAction", "macroDeleteAction");
    i18nSetValue("macroDeleteDelay", "macroDeleteDelay");
    i18nSetValue("macroTypeDown", "macroTypeDown");
    i18nSetValue("macroTypeTap", "macroTypeTap");
    i18nSetValue("macroTypeUp", "macroTypeUp");
    i18nSetText("macroListTitle", "macroListTitle");
    i18nSetText("macroKeySelectTitle", "macroKeySelectTitle");
    i18nSetText("bouncePageTitle", "bouncePageTitle");
    i18nSetValue("bounceDiagStart", "bounceDiagStart");
    i18nSetValue("bounceDiagCancel", "bounceDiagCancel");
    i18nSetText("bounceStepIdleNo", "bounceStepIdleNo");
    i18nSetText("bounceStepIdleTitle", "bounceStepIdleTitle");
    i18nSetText("bounceStepIdleText", "bounceStepIdleText");
    i18nSetText("bounceStepSelectNo", "bounceStepSelectNo");
    i18nSetText("bounceStepSelectTitle", "bounceStepSelectTitle");
    i18nSetText("bounceStepSelectText", "bounceStepSelectText");
    i18nSetText("bounceStepReleaseNo", "bounceStepReleaseNo");
    i18nSetText("bounceStepReleaseTitle", "bounceStepReleaseTitle");
    i18nSetText("bounceStepReleaseText", "bounceStepReleaseText");
    i18nSetText("bounceStepPressNo", "bounceStepPressNo");
    i18nSetText("bounceStepPressTitle", "bounceStepPressTitle");
    i18nSetText("bounceStepPressText", "bounceStepPressText");
    i18nSetText("bounceStepDoneNo", "bounceStepDoneNo");
    i18nSetText("bounceStepDoneTitle", "bounceStepDoneTitle");
    i18nSetText("bounceStepDoneText", "bounceStepDoneText");
    i18nSetText("bounceMetricStateLabel", "bounceMetricStateLabel");
    i18nSetText("bounceMetricKeyLabel", "bounceMetricKeyLabel");
    i18nSetText("bounceMetricEdgesLabel", "bounceMetricEdgesLabel");
    i18nSetText("bounceMetricReleaseEdgesLabel", "bounceMetricReleaseEdgesLabel");
    i18nSetText("bounceMetricPressTotalLabel", "bounceMetricPressTotalLabel");
    i18nSetText("bounceMetricReleaseTotalLabel", "bounceMetricReleaseTotalLabel");
    i18nSetText("bounceIntervalTitle", "bounceIntervalTitle");
    i18nSetText("bounceReleaseIntervalTitle", "bounceReleaseIntervalTitle");
    i18nSetText("bounceHistoryTitle", "bounceHistoryTitle");
    i18nSetText("gifUploadTitle", "gifUploadTitle");
    i18nSetText("gifUploadInfo", "gifUploadInfo");
    if(gifUploadBusy == 0) i18nSetText("gifUploadState", "gifUploadState");
    i18nSetValue("gifUploadStart", "gifUploadStart");
    i18nSetValue("gifUploadCancel", "gifUploadCancel");
    i18nApplyLabels();
    if(magic63LastFsInfo == null) i18nSetText("fsInfo", "fsDefault");
    if(magic63LastVersionInfo == null) i18nSetText("versionInfo", "versionDefault");
    var intervalList = document.getElementById("bounceIntervalList");
    if(intervalList != null && (intervalList.innerHTML == "等待测试结果" || intervalList.innerHTML == "Waiting for result")) intervalList.innerHTML = i18nText("waitTestResult");
    var releaseIntervalList = document.getElementById("bounceReleaseIntervalList");
    if(releaseIntervalList != null && (releaseIntervalList.innerHTML == "等待测试结果" || releaseIntervalList.innerHTML == "Waiting for result")) releaseIntervalList.innerHTML = i18nText("waitTestResult");
    if(magic63LastFsInfo != null) fsInfoShow(magic63LastFsInfo);
    if(magic63LastVersionInfo != null) versionInfoShow(magic63LastVersionInfo);
    macroRender();
    bounceDiagHistoryRender();
}

function i18nInit()
{
    var saved = localStorage.getItem("magic63Lang");
    if(saved == "zh" || saved == "en") magic63Lang = saved;
    else magic63Lang = (navigator.language || "").toLowerCase().indexOf("zh") == 0 ? "zh" : "en";
    var button = document.getElementById("languageToggle");
    if(button != null)
    {
        button.addEventListener("click", function(){
            magic63Lang = magic63Lang == "zh" ? "en" : "zh";
            localStorage.setItem("magic63Lang", magic63Lang);
            i18nApply();
            keymapGetData(0xff);
        });
    }
    i18nApply();
}

window.onload = function () {
    console.log("onload");
    console.log("MagicKey63 settings page version", MAGIC63_SETTINGS_PAGE_VERSION);

    KeyboardKeylisten();
    virtualKeyboardKeylisten();
    virtualKeyboardKeylistenMedia();
    virtualKeyboardKeylistenMouse();
    virtualKeyboardKeylistenLayer();
    virtualKeyboardKeylistenPage();
    layerKeylisten();
    deviceControlListen();
    pageControlListen();
    gifUploadInit();
    macroEditorInit();
    bounceDiagInit();
    i18nInit();
    keymapGetData(0xff);
    fsInfoGet();
    versionInfoGet();
  };


/*以下function是页面上的button被点击的时候的回调*/

var activationKeyId = 0xff;
var currentPage = "keyboard";
function keyBoardKeyHandle(v) //键盘按键被选中，对应实体键盘上的键位
{
    console.log(v.target.id);
    document.getElementById(v.target.id).style.background = "red";
    if(activationKeyId != 0xff) document.getElementById(activationKeyId).style.background = "#ffffff";
    if(activationKeyId == v.target.id)  activationKeyId = 0xff;
    else activationKeyId = v.target.id;
}

function virtualkeyBoardKeyHandle1(v)   //虚拟键盘功能键被选中
{
    if(currentPage == "macro")
    {
        var temp = v.target.id.slice(2,3);
        var numValue=parseInt(temp) - 1;
        macroUseVirtualKey(3, 0xe0 + numValue, v.target.innerHTML, v.target);
        return 0;
    }
    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);
    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);
    var temp = v.target.id.slice(2,3);
    var numValue=parseInt(temp)
    temp = activationKeyId.slice(5);
    var numID = parseInt(temp);
    numValue = numValue -1;
    keymapSet(numID,numValue);


    document.getElementById(activationKeyId).innerHTML = document.getElementById(v.target.id).innerHTML;
    document.getElementById(activationKeyId).style.background = "#ffffff";
     
    

    if(numID == 70) activationKeyId = 'keyId'+ (79); 
    else if(numID == 79) activationKeyId = 'keyId'+ (71); 
    else if(numID == 71) activationKeyId = 'keyId'+ (70); 
    else
    {
        for(var i = 1;;i++)
        {
            var temp = numID + i;
            if(temp == 71 || temp == 79 || temp == 70)
            {
                temp += 1;
            }
            if(temp > 80)  temp = temp - 80;
            activationKeyId = 'keyId'+ (temp);     
            if(document.getElementById(activationKeyId) != null) break;
        }
    }


    document.getElementById(activationKeyId).style.background = "red";
    console.log(numID,numValue,activationKeyId);

}

function virtualkeyBoardKeyHandle2(v) //虚拟键盘普通按键被被选中
{
    if(currentPage == "macro")
    {
        var temp = v.target.id.slice(2,6);
        var numValue=parseInt(temp);
        macroUseVirtualKey(3, numValue, v.target.innerHTML, v.target);
        return 0;
    }
    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);

    var temp = v.target.id.slice(2,6);
    var numValue=parseInt(temp)

    temp = activationKeyId.slice(5);
    var numID = parseInt(temp);

    keymapSet(numID,numValue + 8);

    document.getElementById(activationKeyId).innerHTML = document.getElementById(v.target.id).innerHTML;
    document.getElementById(activationKeyId).style.background = "#ffffff";
    if(numID == 70) activationKeyId = 'keyId'+ (79); 
    else if(numID == 79) activationKeyId = 'keyId'+ (71); 
    else if(numID == 71) activationKeyId = 'keyId'+ (70); 
    else
    {
        for(var i = 1;;i++)
        {
            var temp = numID + i;
            if(temp == 71 || temp == 79 || temp == 70)
            {
                temp += 1;
            }
            if(temp > 80)  temp = temp - 80;
            activationKeyId = 'keyId'+ (temp);   
            if(document.getElementById(activationKeyId) != null) break;
        }
    }


    document.getElementById(activationKeyId).style.background = "red";
    console.log(numID,numValue+8,activationKeyId);
}

function virtualkeyBoardKeyHandleMedia(v) //虚拟键盘普通按键被被选中 多媒体键
{
    console.log("virtualkeyBoardKeyHandleMedia")
    if(currentPage == "macro")
    {
        var temp = v.target.id.slice(3,4);
        var numValue=parseInt(temp);
        macroUseVirtualKey(5, numValue, v.target.getAttribute("name"), v.target);
        return 0;
    }

    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);

    var temp = v.target.id.slice(3,4);
    var numValue=parseInt(temp)
    
    console.log("temp",temp);

    temp = activationKeyId.slice(5);
    var numID = parseInt(temp);

    keymapSet(numID,numValue+0x0200);

    document.getElementById(activationKeyId).innerHTML =v.target.getAttribute("name");
    document.getElementById(activationKeyId).style.background = "#ffffff";
    
    if(numID == 70) activationKeyId = 'keyId'+ (79); 
    else if(numID == 79) activationKeyId = 'keyId'+ (71); 
    else if(numID == 71) activationKeyId = 'keyId'+ (70); 
    else
    {
        for(var i = 1;;i++)
        {
            var temp = numID + i;
            if(temp == 71 || temp == 79 || temp == 70)
            {
                temp += 1;
            }
            if(temp > 80)  temp = temp - 80;
            activationKeyId = 'keyId'+ (temp);   
            if(document.getElementById(activationKeyId) != null) break;
        }
    }

    document.getElementById(activationKeyId).style.background = "red";
    console.log(numID,numValue+8,activationKeyId);
}

function virtualkeyBoardKeyHandleMouse(v) //虚拟键盘普通按键被被选中 鼠标
{
    if(currentPage == "macro")
    {
        var temp = v.target.id.slice(3,4);
        var numValue=parseInt(temp);
        macroUseVirtualKey(6, numValue, v.target.getAttribute("name"), v.target);
        return 0;
    }
    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);

    var temp = v.target.id.slice(3,4);
    var numValue=parseInt(temp)
    
    console.log("temp",temp);

    temp = activationKeyId.slice(5);
    var numID = parseInt(temp);

    keymapSet(numID,numValue+0x0100);

    document.getElementById(activationKeyId).innerHTML =v.target.getAttribute("name");
    document.getElementById(activationKeyId).style.background = "#ffffff";
    if(numID == 70) activationKeyId = 'keyId'+ (79); 
    else if(numID == 79) activationKeyId = 'keyId'+ (71); 
    else if(numID == 71) activationKeyId = 'keyId'+ (70); 
    else
    {  
        for(var i = 1;;i++)
        {
            var temp = numID + i;
            if(temp == 71 || temp == 79 || temp == 70)
            {
                temp += 1;
            }
            if(temp > 80)  temp = temp - 80;
            activationKeyId = 'keyId'+ (temp);   
            if(document.getElementById(activationKeyId) != null) break;
        }
    }
    document.getElementById(activationKeyId).style.background = "red";
    console.log(numID,numValue+8,activationKeyId);
}

function virtualkeyBoardKeyHandleLayer(v) //虚拟键盘普通按键被被选中 鼠标
{
    if(currentPage == "macro")
    {
        return 0;
    }
    if(activationKeyId == 0xff) return 0;
    console.log(v.target.id);

    var temp = v.target.id.slice(3,4);
    var numValue=parseInt(temp)
    
    console.log("temp",temp);

    temp = activationKeyId.slice(5);
    var numID = parseInt(temp);

    keymapSet(numID,numValue+0x0400);

    document.getElementById(activationKeyId).innerHTML =v.target.getAttribute("name");
    document.getElementById(activationKeyId).style.background = "#ffffff";
    if(numID == 70) activationKeyId = 'keyId'+ (79); 
    else if(numID == 79) activationKeyId = 'keyId'+ (71); 
    else if(numID == 71) activationKeyId = 'keyId'+ (70); 
    else
    {  
        for(var i = 1;;i++)
        {
            var temp = numID + i;
            if(temp == 71 || temp == 79 || temp == 70)
            {
                temp += 1;
            }
            if(temp > 80)  temp = temp - 80;
            activationKeyId = 'keyId'+ (temp);   
            if(document.getElementById(activationKeyId) != null) break;
        }
    }
    document.getElementById(activationKeyId).style.background = "red";
    console.log(numID,numValue+8,activationKeyId);
}

function virtualkeyBoardKeyHandlePage(v)
{
    if(currentPage == "macro")
    {
        return 0;
    }
    if(activationKeyId == 0xff) return 0;

    var idPrefix = v.target.id.slice(0, 3);
    var numValue = parseInt(v.target.id.slice(3));
    if(numValue <= 0) return 0;
    if(idPrefix == "IDQ") numValue = numValue | 0x80;

    var temp = activationKeyId.slice(5);
    var numID = parseInt(temp);

    keymapSet(numID, numValue + 0x0500);

    document.getElementById(activationKeyId).innerHTML = v.target.getAttribute("name");
    document.getElementById(activationKeyId).style.background = "#ffffff";
    if(numID == 70) activationKeyId = 'keyId'+ (79);
    else if(numID == 79) activationKeyId = 'keyId'+ (71);
    else if(numID == 71) activationKeyId = 'keyId'+ (70);
    else
    {
        for(var i = 1;;i++)
        {
            var nextId = numID + i;
            if(nextId == 71 || nextId == 79 || nextId == 70)
            {
                nextId += 1;
            }
            if(nextId > 80) nextId = nextId - 80;
            activationKeyId = 'keyId'+ (nextId);
            if(document.getElementById(activationKeyId) != null) break;
        }
    }
    document.getElementById(activationKeyId).style.background = "red";
}

function macroBindKeyHandle(v)
{
    if(currentPage != "keyboard") return 0;
    if(activationKeyId == 0xff) return 0;

    var macroId = parseInt(v.currentTarget.getAttribute("data-id"));
    if(macroId <= 0) return 0;

    var temp = activationKeyId.slice(5);
    var numID = parseInt(temp);
    keymapSet(numID, macroId + 0x0300);

    document.getElementById(activationKeyId).innerHTML = "M" + macroId;
    document.getElementById(activationKeyId).style.background = "#ffffff";

    if(numID == 70) activationKeyId = 'keyId'+ (79);
    else if(numID == 79) activationKeyId = 'keyId'+ (71);
    else if(numID == 71) activationKeyId = 'keyId'+ (70);
    else
    {
        for(var i = 1;;i++)
        {
            var nextId = numID + i;
            if(nextId == 71 || nextId == 79 || nextId == 70)
            {
                nextId += 1;
            }
            if(nextId > 80) nextId = nextId - 80;
            activationKeyId = 'keyId'+ (nextId);
            if(document.getElementById(activationKeyId) != null) break;
        }
    }
    document.getElementById(activationKeyId).style.background = "red";
}

var timeCount = 0;
function timeCountCallback() {
    timeCount = 0;
    console.log("timeCountCallback")
}

function layerSelectKeyHandle(v) //选中层
{
    console.log(v.target.id);
    var temp = v.target.id.slice(5,6);
    var numValue=parseInt(temp)
    //console.log(temp);
    document.getElementById("layer1").style.background = "#ffffff";
    document.getElementById("layer2").style.background = "#ffffff";
    document.getElementById("layer3").style.background = "#ffffff";
    document.getElementById("layer4").style.background = "#ffffff";
    document.getElementById("layer"+numValue).style.background = "red";
    console.log(numValue);
    if(numValue == 1)
    {
        if(timeCount == 0) setTimeout(timeCountCallback, 1000); //1秒的定时器
        timeCount ++ ;
        if(timeCount == 5) 
        {
            //console.log("trigger");
            webDataRequest("delete","{\"satate\":1}");
            return 
        }
        if(timeCount >5) 
        {
            return
        } 
    }
    //console.log(numValue);
    keymapGetData(numValue);
}


/*以下function是初始化页面用的。*/
function KeyboardKeylisten() //键盘按键监听
{
    var idHand = "keyId"
    for(var i = 1;i<81;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            var str = that.getAttribute('name') + ":" + "\"" + that.id + "\",";
            document.querySelector(tempId).addEventListener('click', keyBoardKeyHandle);
        }
    }
}

function virtualKeyboardKeylistenMedia() //虚拟键盘按键监听
{
    var idHand = "IDM"
    for(var i = 0;i<10;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandleMedia);
        }   
    }
}

function virtualKeyboardKeylistenMouse() //虚拟键盘按键监听
{
    var idHand = "IDS"

    console.log("temppppp xxxx")
    for(var i = 0;i<5;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandleMouse);
        }   
    }
}

function virtualKeyboardKeylistenLayer() //虚拟键盘按键监听
{
    var idHand = "IDL"

    console.log("temppppp xxxx")
    for(var i = 0;i<5;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandleLayer);
        }   
    }
}

function virtualKeyboardKeylistenPage()
{
    var pageIds = ["IDP1","IDP2","IDP3","IDP4","IDP5","IDP6","IDP7","IDP8","IDP9","IDP10",
                   "IDQ1","IDQ2","IDQ3","IDQ4","IDQ5","IDQ6","IDQ8","IDQ9"];
    for(var i = 0;i<pageIds.length;i++)
    {
        var that = document.getElementById(pageIds[i]);
        if(that != null)
        {
            var tempId = '#' + that.id;
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandlePage);
        }
    }
}

function virtualKeyboardKeylisten() //虚拟键盘按键监听
{
    var idHand = "ID"
    for(var i = 1;i<9;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            var str = that.getAttribute('name') + ":" + "\"" + that.id + "\",";
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandle1);
        }   
    }

    idHand = 'ID0x';
    for(var i = 1;i<0x80;i++)
    {
        var temp = i.toString(16);
        var temp2 = temp.toUpperCase()
        var that = document.getElementById(idHand+temp2);
        if(that != null)
        {
            var tempId = '#' + that.id;
            var str = that.getAttribute('name') + ":" + "\"" + that.id + "\",";
            document.querySelector(tempId).addEventListener('click', virtualkeyBoardKeyHandle2);
        }   
    }
}

var xmlHttp = new XMLHttpRequest;
/*以下function是请求数据用的*/
function webDataRequest(api,data) //发送数据
{
    var URL = '/api/' + api; 
    xmlHttp.open('POST', URL);
    xmlHttp.setRequestHeader('content-type', 'application/json');
    xmlHttp.send(data);
    xmlHttp.onreadystatechange = webDataHandle;
}

function deviceReboot()
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/reboot');
    req.setRequestHeader('content-type', 'application/json');
    req.send('{"state":1}');
}

function uploadPageOpen()
{
    pageShow("gifUpload");
}

function versionInfoToggle()
{
    var popup = document.getElementById("versionInfoPopup");
    if(popup == null) return 0;
    if(popup.className.indexOf("versionInfoPopupOpen") >= 0)
    {
        popup.className = "versionInfoPopup";
    }
    else
    {
        popup.className = "versionInfoPopup versionInfoPopupOpen";
        versionInfoGet();
    }
}

function deviceControlListen()
{
    var rebootButton = document.getElementById("deviceReboot");
    var uploadButton = document.getElementById("showUploadPage");
    var versionButton = document.getElementById("showVersionInfo");
    if(uploadButton != null) uploadButton.addEventListener('click', uploadPageOpen);
    if(rebootButton != null) rebootButton.addEventListener('click', deviceReboot);
    if(versionButton != null) versionButton.addEventListener('click', versionInfoToggle);
}

function pageShow(name)
{
    var keyboardPage = document.getElementById("keyboardPage");
    var macroPage = document.getElementById("macroPage");
    var gifUploadPage = document.getElementById("gifUploadPage");
    var bounceDiagPage = document.getElementById("bounceDiagPage");
    var keyboardButton = document.getElementById("showKeyboardPage");
    var macroButton = document.getElementById("showMacroPage");
    var uploadButton = document.getElementById("showUploadPage");
    var bounceButton = document.getElementById("showBounceDiagPage");
    var sharedKeyboard = document.getElementById("sharedVirtualKeyboard");
    var sharedHome = document.getElementById("sharedKeyboardHome");
    var macroHost = document.getElementById("macroKeyboardHost");
    var macroBindKeyboard = document.getElementById("macroBindKeyboard");
    if(keyboardPage == null || macroPage == null) return 0;
    if(currentPage == "bounceDiag" && name != "bounceDiag") bounceDiagCancelSilent();
    currentPage = name;
    if(sharedKeyboard != null)
    {
        if(name == "macro" && macroHost != null) macroHost.appendChild(sharedKeyboard);
        if(name == "keyboard" && sharedHome != null) sharedHome.appendChild(sharedKeyboard);
    }
    if(name == "keyboard" && macroSelectedKey != null)
    {
        macroSelectedKey.style.background = "#ffffff";
        macroSelectedKey = null;
    }
    keyboardPage.style.display = (name == "keyboard") ? "block" : "none";
    macroPage.style.display = (name == "macro") ? "block" : "none";
    if(gifUploadPage != null) gifUploadPage.style.display = (name == "gifUpload") ? "block" : "none";
    if(bounceDiagPage != null) bounceDiagPage.style.display = (name == "bounceDiag") ? "block" : "none";
    if(macroBindKeyboard != null) macroBindKeyboard.style.display = (name == "keyboard") ? "block" : "none";
    if(keyboardButton != null) keyboardButton.className = (name == "keyboard") ? "topButton topButtonActive" : "topButton";
    if(macroButton != null) macroButton.className = (name == "macro") ? "topButton topButtonActive" : "topButton";
    if(uploadButton != null) uploadButton.className = (name == "gifUpload") ? "topButton topButtonActive" : "topButton";
    if(bounceButton != null) bounceButton.className = (name == "bounceDiag") ? "topButton topButtonActive" : "topButton";
}

function pageControlListen()
{
    var keyboardButton = document.getElementById("showKeyboardPage");
    var macroButton = document.getElementById("showMacroPage");
    var bounceButton = document.getElementById("showBounceDiagPage");
    if(keyboardButton != null) keyboardButton.addEventListener('click', function(){ pageShow("keyboard"); });
    if(macroButton != null) macroButton.addEventListener('click', function(){ pageShow("macro"); });
    if(bounceButton != null) bounceButton.addEventListener('click', bounceDiagEnter);
}

var bounceDiagTimer = 0;
var bounceDiagLastState = -1;
var bounceDiagRunActive = 0;
var bounceDiagRunRecorded = 0;
var bounceDiagHistory = [];

function bounceDiagRequest(api, callback)
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/' + api);
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) {
            callback(null);
            return 0;
        }
        var obj = null;
        try { obj = JSON.parse(v.currentTarget.responseText); }
        catch(e) { obj = null; }
        callback(obj);
    };
    req.send('{"state":1}');
}

function bounceDiagCancelBeacon()
{
    if(navigator.sendBeacon)
    {
        var data = new Blob(['{"state":1}'], {type:'application/json'});
        navigator.sendBeacon('/api/bounceDiagCancel', data);
    }
}

function bounceDiagStateCn(state)
{
    if(state >= 0 && state <= 8) return i18nText("bounceState" + state);
    return i18nText("bounceStateUnknown");
}

function bounceDiagStepForState(state)
{
    if(state == 1) return "bounceStepSelect";
    if(state == 2) return "bounceStepRelease";
    if(state == 3 || state == 4) return "bounceStepPress";
    if(state == 5 || state == 6 || state == 7 || state == 8) return "bounceStepDone";
    return "bounceStepIdle";
}

function bounceDiagSetStep(state)
{
    var ids = ["bounceStepIdle","bounceStepSelect","bounceStepRelease","bounceStepPress","bounceStepDone"];
    var active = bounceDiagStepForState(state);
    var activeIndex = 0;
    for(var i = 0;i<ids.length;i++)
    {
        if(ids[i] == active) activeIndex = i;
    }
    for(var j = 0;j<ids.length;j++)
    {
        var el = document.getElementById(ids[j]);
        if(el == null) continue;
        var cls = "bounceStep";
        if(j < activeIndex) cls += " bounceStepDone";
        if(ids[j] == active) cls += " bounceStepActive";
        el.className = cls;
    }
}

function bounceDiagKeyText(obj)
{
    if(obj == null || obj.row == null || obj.col == null) return "--";
    if(parseInt(obj.row) >= 255 || parseInt(obj.col) >= 255) return "--";
    return i18nText("bounceKeyPos", {row: parseInt(obj.row) + 1, col: parseInt(obj.col) + 1});
}

function bounceDiagShowIntervals(rootId, titleId, baseTitle, list, edgeCount)
{
    var root = document.getElementById(rootId);
    var title = document.getElementById(titleId);
    if(root == null) return 0;
    root.innerHTML = "";
    edgeCount = parseInt(edgeCount) || 0;
    var shown = list == null ? 0 : list.length;
    if(title != null)
    {
        if(edgeCount > shown) title.innerHTML = i18nText("bounceTitleMore", {title: baseTitle, total: edgeCount, shown: shown});
        else title.innerHTML = baseTitle;
    }
    if(list == null || list.length == 0)
    {
        root.innerHTML = i18nText("bounceNoIntervals");
        return 0;
    }
    for(var i = 0;i<list.length;i++)
    {
        var box = document.createElement("div");
        box.className = "bounceIntervalBox";
        box.innerHTML = i18nText("bounceIntervalItem", {index: i + 1, value: list[i]});
        root.appendChild(box);
    }
    if(edgeCount > list.length)
    {
        var more = document.createElement("div");
        more.className = "bounceIntervalBox bounceIntervalMore";
        more.innerHTML = i18nText("bounceMoreIntervals", {count: edgeCount - list.length});
        root.appendChild(more);
    }
}

function bounceDiagTimeText(us)
{
    us = parseInt(us) || 0;
    if(us <= 0) return "--";
    if(us < 1000) return us + " us";
    return (us / 1000).toFixed(1) + " ms";
}

function bounceDiagHistoryPush(obj)
{
    if(obj == null) return 0;
    bounceDiagHistory.unshift({
        key: bounceDiagKeyText(obj),
        edgeCount: parseInt(obj.edgeCount) || 0,
        releaseEdgeCount: parseInt(obj.releaseEdgeCount) || 0,
        pressTotalUs: parseInt(obj.pressTotalUs) || 0,
        releaseTotalUs: parseInt(obj.releaseTotalUs) || 0
    });
    if(bounceDiagHistory.length > 12) bounceDiagHistory.pop();
    bounceDiagHistoryRender();
}

function bounceDiagHistoryRender()
{
    var root = document.getElementById("bounceHistoryList");
    if(root == null) return 0;
    root.innerHTML = "";
    if(bounceDiagHistory.length == 0)
    {
        root.innerHTML = i18nText("bounceNoHistory");
        return 0;
    }
    for(var i = 0;i<bounceDiagHistory.length;i++)
    {
        var item = bounceDiagHistory[i];
        var row = document.createElement("div");
        row.className = "bounceHistoryItem";
        row.innerHTML = "<strong>" + item.key + "</strong>" +
            "<span>" + i18nText("bounceHistoryPress") + " <strong>" + item.edgeCount + "</strong></span>" +
            "<span>" + i18nText("bounceHistoryRelease") + " <strong>" + item.releaseEdgeCount + "</strong></span>" +
            "<span>" + i18nText("bounceHistoryPressStable") + " <strong>" + bounceDiagTimeText(item.pressTotalUs) + "</strong></span>" +
            "<span>" + i18nText("bounceHistoryReleaseStable") + " <strong>" + bounceDiagTimeText(item.releaseTotalUs) + "</strong></span>";
        root.appendChild(row);
    }
}

function bounceDiagShow(obj)
{
    if(obj == null)
    {
        var stateFail = document.getElementById("bounceDiagState");
        if(stateFail != null) stateFail.innerHTML = i18nText("deviceConnectFail");
        return 0;
    }
    var state = parseInt(obj.state) || 0;
    var stateText = bounceDiagStateCn(state);
    var keyText = bounceDiagKeyText(obj);
    if(bounceDiagLastState == 7 && state != 7 && state != 0 && state != 8)
    {
        bounceDiagRunActive = 1;
        bounceDiagRunRecorded = 0;
    }
    bounceDiagSetStep(state);

    var stateTop = document.getElementById("bounceDiagState");
    var targetTop = document.getElementById("bounceDiagTarget");
    var metricState = document.getElementById("bounceMetricState");
    var metricKey = document.getElementById("bounceMetricKey");
    var metricEdges = document.getElementById("bounceMetricEdges");
    var metricReleaseEdges = document.getElementById("bounceMetricReleaseEdges");
    var metricPressTotal = document.getElementById("bounceMetricPressTotal");
    var metricReleaseTotal = document.getElementById("bounceMetricReleaseTotal");
    var start = document.getElementById("bounceDiagStart");
    var cancel = document.getElementById("bounceDiagCancel");

    if(stateTop != null) stateTop.innerHTML = stateText;
    if(targetTop != null) targetTop.innerHTML = keyText;
    if(metricState != null) metricState.innerHTML = stateText;
    if(metricKey != null) metricKey.innerHTML = keyText;
    if(metricEdges != null) metricEdges.innerHTML = obj.edgeCount == null ? "--" : obj.edgeCount;
    if(metricReleaseEdges != null) metricReleaseEdges.innerHTML = obj.releaseEdgeCount == null ? "--" : obj.releaseEdgeCount;
    if(metricPressTotal != null) metricPressTotal.innerHTML = bounceDiagTimeText(obj.pressTotalUs);
    if(metricReleaseTotal != null) metricReleaseTotal.innerHTML = bounceDiagTimeText(obj.releaseTotalUs);
    if(start != null) start.disabled = state != 0 && state != 7 && state != 8;
    if(cancel != null) cancel.disabled = state == 0 || state == 8;

    bounceDiagShowIntervals("bounceIntervalList", "bounceIntervalTitle", i18nText("bounceIntervalTitle"), obj.intervals || [], obj.edgeCount);
    bounceDiagShowIntervals("bounceReleaseIntervalList", "bounceReleaseIntervalTitle", i18nText("bounceReleaseIntervalTitle"), obj.releaseIntervals || [], obj.releaseEdgeCount);
    if(state == 7 && bounceDiagRunActive && bounceDiagRunRecorded == 0)
    {
        bounceDiagHistoryPush(obj);
        bounceDiagRunRecorded = 1;
    }
    if(state == 8)
    {
        bounceDiagRunRecorded = 1;
        bounceDiagRunActive = 0;
    }
    bounceDiagLastState = state;
}

function bounceDiagStatus()
{
    if(currentPage != "bounceDiag") return 0;
    bounceDiagRequest("bounceDiagStatus", bounceDiagShow);
}

function bounceDiagEnter()
{
    pageShow("bounceDiag");
    bounceDiagRequest("bounceDiagStatus", function(obj) {
        bounceDiagShow(obj);
        if(obj == null) return 0;
        var state = parseInt(obj.state) || 0;
        if(state == 0 || state == 8) bounceDiagStart();
    });
}

function bounceDiagStart()
{
    bounceDiagRunActive = 1;
    bounceDiagRunRecorded = 0;
    bounceDiagRequest("bounceDiagStart", function(obj) {
        bounceDiagStatus();
    });
}

function bounceDiagCancel()
{
    bounceDiagRequest("bounceDiagCancel", function(obj) {
        bounceDiagRunActive = 0;
        bounceDiagRunRecorded = 1;
        bounceDiagStatus();
    });
}

function bounceDiagCancelSilent()
{
    bounceDiagRunActive = 0;
    bounceDiagRunRecorded = 1;
    bounceDiagRequest("bounceDiagCancel", function(obj) {});
}

function bounceDiagInit()
{
    var start = document.getElementById("bounceDiagStart");
    var cancel = document.getElementById("bounceDiagCancel");
    if(start == null) return 0;
    start.addEventListener('click', bounceDiagStart);
    if(cancel != null) cancel.addEventListener('click', bounceDiagCancel);
    window.addEventListener('pagehide', function(){
        if(currentPage == "bounceDiag") bounceDiagCancelBeacon();
    });
    window.addEventListener('beforeunload', function(){
        if(currentPage == "bounceDiag") bounceDiagCancelBeacon();
    });
    bounceDiagHistoryRender();
    if(bounceDiagTimer == 0) bounceDiagTimer = setInterval(bounceDiagStatus, 250);
}

var gifUploadChunkSize = 1024;
var gifUploadMaxSize = 512 * 1024;
var gifUploadBusy = 0;
var gifUploadCancelFlag = 0;

function gifUploadSetState(text)
{
    var state = document.getElementById("gifUploadState");
    if(state != null) state.innerHTML = text;
}

function gifUploadRequestJson(api, data, callback)
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/' + api);
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) {
            callback(false);
            return 0;
        }
        var obj = {};
        try { obj = JSON.parse(v.currentTarget.responseText); }
        catch(e) { callback(false); return 0; }
        callback(obj.success != "fail" && obj.state != "fail");
    };
    req.send(JSON.stringify(data));
}

function gifUploadRequestChunk(fileObj, start, len, callback)
{
    var req = new XMLHttpRequest;
    var fd = new FormData();
    fd.append('file', fileObj.slice(start, start + len, "application/octet-stream"));
    req.open('POST', '/api/updateP');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) {
            callback(false);
            return 0;
        }
        var obj = {};
        try { obj = JSON.parse(v.currentTarget.responseText); }
        catch(e) { callback(false); return 0; }
        callback(obj.success != "fail");
    };
    req.send(fd);
}

function gifUploadChecksum(fileObj, start, len, callback)
{
    var reader = new FileReader();
    reader.onload = function() {
        var data = new DataView(reader.result);
        var sum = 0;
        for(var i = 0; i < data.byteLength; i++) sum += data.getUint8(i);
        callback(sum);
    };
    reader.onerror = function() {
        callback(-1);
    };
    reader.readAsArrayBuffer(fileObj.slice(start, start + len));
}

function gifUploadCheckSize(fileObj, callback)
{
    var reader = new FileReader();
    reader.onload = function() {
        var data = new DataView(reader.result);
        if(data.byteLength < 10) {
            callback(false);
            return;
        }
        var sign = String.fromCharCode(data.getUint8(0), data.getUint8(1), data.getUint8(2));
        var width = data.getUint16(6, true);
        var height = data.getUint16(8, true);
        callback(sign == "GIF" && width == 160 && height == 80);
    };
    reader.onerror = function() {
        callback(false);
    };
    reader.readAsArrayBuffer(fileObj.slice(0, 10));
}

function gifUploadProgress(offset, total)
{
    var bar = document.getElementById("gifUploadProgress");
    if(bar == null) return;
    bar.max = total;
    bar.value = offset;
}

function gifUploadSendNext(fileObj, offset)
{
    if(gifUploadCancelFlag) {
        gifUploadBusy = 0;
        gifUploadSetState(i18nText("gifCanceled"));
        return;
    }
    if(offset >= fileObj.size) {
        gifUploadBusy = 0;
        gifUploadProgress(fileObj.size, fileObj.size);
        gifUploadSetState(i18nText("gifDone"));
        fsInfoGet();
        return;
    }

    var len = (offset + gifUploadChunkSize > fileObj.size) ? (fileObj.size - offset) : gifUploadChunkSize;
    gifUploadSetState(i18nText("gifUploading", {percent: Math.floor((offset * 100) / fileObj.size)}));
    gifUploadChecksum(fileObj, offset, len, function(sum) {
        if(sum < 0) {
            gifUploadBusy = 0;
            gifUploadSetState(i18nText("gifReadFail"));
            return;
        }
        gifUploadRequestChunk(fileObj, offset, len, function(ok) {
            if(!ok) {
                gifUploadBusy = 0;
                gifUploadSetState(i18nText("gifUploadFail"));
                return;
            }
            gifUploadRequestJson("webFileUpdatePpakgEnter", {
                offset: offset,
                cleckSum: sum,
                len: len,
                over: (offset + len >= fileObj.size) ? 1 : 0
            }, function(ok2) {
                if(!ok2) {
                    gifUploadBusy = 0;
                    gifUploadSetState(i18nText("gifWriteFail"));
                    return;
                }
                gifUploadProgress(offset + len, fileObj.size);
                gifUploadSendNext(fileObj, offset + len);
            });
        });
    });
}

function gifUploadStart()
{
    if(gifUploadBusy) return;
    var input = document.getElementById("gifUploadFile");
    if(input == null || input.files.length == 0) {
        gifUploadSetState(i18nText("gifSelect"));
        return;
    }
    var fileObj = input.files[0];
    if(fileObj.name.slice(fileObj.name.length - 4) != ".gif") {
        gifUploadSetState(i18nText("gifTypeFail"));
        return;
    }
    if(fileObj.size > gifUploadMaxSize) {
        gifUploadSetState(i18nText("gifTooLarge"));
        return;
    }
    gifUploadCheckSize(fileObj, function(ok) {
        if(!ok) {
            gifUploadSetState(i18nText("gifResolutionFail"));
            return;
        }
        gifUploadBusy = 1;
        gifUploadCancelFlag = 0;
        gifUploadProgress(0, fileObj.size);
        gifUploadRequestJson("updateStart", {size: fileObj.size, type: 2}, function(ok2) {
            if(!ok2) {
                gifUploadBusy = 0;
                gifUploadSetState(i18nText("gifStartFail"));
                return;
            }
            gifUploadSendNext(fileObj, 0);
        });
    });
}

function gifUploadInit()
{
    var start = document.getElementById("gifUploadStart");
    var cancel = document.getElementById("gifUploadCancel");
    if(start != null) start.addEventListener('click', gifUploadStart);
    if(cancel != null) cancel.addEventListener('click', function() {
        gifUploadCancelFlag = 1;
    });
}

var macroMaxActions = 64;
var currentMacro = {id:1, mode:0, actions:[]};
var macroFiles = [];
var macroSelectedIndex = -1;
var macroPendingIndex = -1;
var macroSavedState = "";
var macroSelectedKey = null;

function macroNormalizeAction(action)
{
    if(action == null) action = {type:3,value:4,param:0};
    return {
        type: parseInt(action.type) || 0,
        value: parseInt(action.value) || 0,
        param: parseInt(action.param) || 0
    };
}

function macroCollectActions()
{
    var actions = [];
    for(var i = 0;i<currentMacro.actions.length;i++)
    {
        actions.push(macroNormalizeAction(currentMacro.actions[i]));
    }
    return actions;
}

function macroCurrentState()
{
    return JSON.stringify({
        id: parseInt(currentMacro.id) || 1,
        mode: parseInt(currentMacro.mode) || 0,
        actions: macroCollectActions()
    });
}

function macroIsDirty()
{
    return macroSavedState != macroCurrentState();
}

function macroSelectedAction()
{
    if(macroSelectedIndex < 0 || macroSelectedIndex >= currentMacro.actions.length) return null;
    return currentMacro.actions[macroSelectedIndex];
}

function macroModeText(v)
{
    if(parseInt(v) == 1) return i18nText("macroModeLoop");
    return i18nText("macroModePress");
}

function macroUsageLabel(value)
{
    var usage = parseInt(value) || 0;
    var modifierNames = ["Ctrl","Shift","Alt","Win","Ctrl","Shift","Alt","Win"];
    if(usage >= 0xe0 && usage <= 0xe7) return modifierNames[usage - 0xe0];
    var id = "ID0x" + usage.toString(16).toUpperCase();
    var key = document.getElementById(id);
    if(key != null) return key.innerHTML;
    return "0x" + usage.toString(16).toUpperCase();
}

function macroActionLabel(action)
{
    var type = parseInt(action.type) || 0;
    var value = parseInt(action.value) || 0;
    if(type == 4) return (parseInt(action.param) || 0) + "ms";
    if(type == 5)
    {
        var media = document.getElementById("IDM" + value);
        if(media != null) return media.getAttribute("name");
    }
    if(type == 6)
    {
        var mouse = document.getElementById("IDS" + value);
        if(mouse != null) return mouse.getAttribute("name");
    }
    return macroUsageLabel(value);
}

function macroActionClass(action)
{
    var type = parseInt(action.type) || 0;
    if(type == 1) return "macroActionBox macroActionDown";
    if(type == 2) return "macroActionBox macroActionUp";
    if(type == 4) return "macroActionBox macroActionDelay";
    return "macroActionBox macroActionTap";
}

function macroNextId()
{
    for(var id = 1;id < 256;id++)
    {
        var used = 0;
        for(var i = 0;i<macroFiles.length;i++)
        {
            if(parseInt(macroFiles[i].id) == id)
            {
                used = 1;
                break;
            }
        }
        if(used == 0) return id;
    }
    return 0;
}

function macroIsSaved(id)
{
    for(var i = 0;i<macroFiles.length;i++)
    {
        if(parseInt(macroFiles[i].id) == parseInt(id)) return 1;
    }
    return 0;
}

function macroBindRender()
{
    var root = document.getElementById("macroBindList");
    if(root == null) return 0;
    root.innerHTML = "";
    for(var i = 0;i<macroFiles.length;i++)
    {
        var id = parseInt(macroFiles[i].id);
        if(id <= 0) continue;
        var key = document.createElement("div");
        key.className = "keyB macroBindKey";
        key.setAttribute("data-id", id);
        key.innerHTML = "M" + id;
        key.addEventListener("click", macroBindKeyHandle);
        root.appendChild(key);
    }
    var clear = document.createElement("div");
    clear.style.clear = "both";
    root.appendChild(clear);
}

function macroRenderMeta()
{
    var title = document.getElementById("macroCurrentTitle");
    var count = document.getElementById("macroActionCount");
    var dirty = document.getElementById("macroDirtyBadge");
    if(title != null) title.innerHTML = "M" + currentMacro.id;
    if(count != null) count.innerHTML = currentMacro.actions.length + i18nText("macroActionUnit");
    if(dirty != null) dirty.innerHTML = macroIsDirty() ? i18nText("macroUnsaved") : "";
}

function macroSetSelected(index)
{
    macroSelectedIndex = parseInt(index);
    if(macroSelectedIndex < 0 || macroSelectedIndex >= currentMacro.actions.length) macroSelectedIndex = -1;
    macroRender();
}

function macroInsertAction(action)
{
    if(currentMacro.actions.length >= macroMaxActions) return 0;
    var item = macroNormalizeAction(action);
    if(macroSelectedIndex >= 0 && macroSelectedIndex < currentMacro.actions.length)
    {
        currentMacro.actions.splice(macroSelectedIndex + 1, 0, item);
        macroSelectedIndex += 1;
    }
    else
    {
        currentMacro.actions.push(item);
        macroSelectedIndex = currentMacro.actions.length - 1;
    }
    macroRender();
}

function macroAddActionRow(action)
{
    macroInsertAction(action);
}

function macroRefreshIndex()
{
    macroRender();
}

function macroAddKeyAction()
{
    macroInsertAction({type:3,value:4,param:0});
}

function macroAddDelay()
{
    macroInsertAction({type:4,value:0,param:100});
}

function macroDeleteAction()
{
    if(macroSelectedIndex < 0 || macroSelectedIndex >= currentMacro.actions.length) return 0;
    if(parseInt(currentMacro.actions[macroSelectedIndex].type) == 4) return 0;
    currentMacro.actions.splice(macroSelectedIndex, 1);
    if(macroSelectedIndex >= currentMacro.actions.length) macroSelectedIndex = currentMacro.actions.length - 1;
    macroRender();
}

function macroDeleteDelay()
{
    var action = macroSelectedAction();
    if(action == null || parseInt(action.type) != 4) return 0;
    currentMacro.actions.splice(macroSelectedIndex, 1);
    if(macroSelectedIndex >= currentMacro.actions.length) macroSelectedIndex = currentMacro.actions.length - 1;
    macroRender();
}

function macroSetSelectedType(type)
{
    var action = macroSelectedAction();
    if(action == null) return 0;
    action.type = parseInt(type) || 3;
    if(action.type == 4)
    {
        action.value = 0;
        if(parseInt(action.param) <= 0) action.param = 100;
    }
    else
    {
        if(action.value == null) action.value = 4;
        action.param = 0;
    }
    macroRender();
}

function macroDelayChanged()
{
    var action = macroSelectedAction();
    if(action == null || parseInt(action.type) != 4) return 0;
    var input = document.getElementById("macroDelayValue");
    var value = parseInt(input.value) || 1;
    if(value < 1) value = 1;
    if(value > 60000) value = 60000;
    input.value = value;
    action.param = value;
    macroRender();
}

function macroUseVirtualKey(type, value, label, key)
{
    var action = macroSelectedAction();
    if(action == null) return 0;
    if(macroSelectedKey != null) macroSelectedKey.style.background = "#ffffff";
    macroSelectedKey = key;
    if(macroSelectedKey != null) macroSelectedKey.style.background = "#4d93db";
    action.type = parseInt(type) || 3;
    action.value = parseInt(value) || 0;
    action.param = 0;
    macroRender();
}

function macroRenderEditor()
{
    var action = macroSelectedAction();
    var selected = action != null;
    var selectedDelay = selected && parseInt(action.type) == 4;
    var typeDown = document.getElementById("macroTypeDown");
    var typeTap = document.getElementById("macroTypeTap");
    var typeUp = document.getElementById("macroTypeUp");
    var delayInput = document.getElementById("macroDelayValue");

    if(typeDown != null) typeDown.disabled = !selected || selectedDelay;
    if(typeTap != null) typeTap.disabled = !selected || selectedDelay;
    if(typeUp != null) typeUp.disabled = !selected || selectedDelay;
    if(delayInput != null)
    {
        delayInput.disabled = !selectedDelay;
        if(selectedDelay) delayInput.value = parseInt(action.param) || 100;
    }

    if(typeDown != null) typeDown.className = selected && parseInt(action.type) == 1 ? "macroButtonActive" : "";
    if(typeTap != null) typeTap.className = selected && parseInt(action.type) == 3 ? "macroButtonActive" : "";
    if(typeUp != null) typeUp.className = selected && parseInt(action.type) == 2 ? "macroButtonActive" : "";
}

function macroUpdateButtons()
{
    var selected = macroSelectedAction() != null;
    var selectedDelay = selected && parseInt(macroSelectedAction().type) == 4;
    var canInsert = currentMacro.actions.length < macroMaxActions;
    var save = document.getElementById("macroSave");
    var addAction = document.getElementById("macroAddAction");
    var addDelay = document.getElementById("macroAddDelay");
    var deleteDelay = document.getElementById("macroDeleteDelay");
    var deleteAction = document.getElementById("macroDeleteAction");
    var newMacro = document.getElementById("macroNew");
    var deleteMacro = document.getElementById("macroDelete");

    if(save != null) save.disabled = !macroIsDirty();
    if(newMacro != null) newMacro.disabled = macroIsDirty() || macroNextId() == 0;
    if(deleteMacro != null) deleteMacro.disabled = !macroIsSaved(currentMacro.id) && currentMacro.actions.length == 0 && !macroIsDirty();
    if(addAction != null) addAction.disabled = !canInsert || (!selected && currentMacro.actions.length > 0);
    if(addDelay != null) addDelay.disabled = !canInsert || (!selected && currentMacro.actions.length > 0);
    if(deleteDelay != null) deleteDelay.disabled = !selectedDelay;
    if(deleteAction != null) deleteAction.disabled = !selected || selectedDelay;
    macroRenderEditor();
}

function macroRender()
{
    macroRenderList();
    macroRenderMeta();
    macroUpdateButtons();
}

function macroStoreFile(obj)
{
    if(obj == null) return 0;
    var id = parseInt(obj.id) || 0;
    if(id <= 0) return 0;
    var item = {
        id: id,
        mode: parseInt(obj.mode) || 0,
        actions: []
    };
    var actions = obj.actions || [];
    for(var i = 0;i<actions.length;i++) item.actions.push(macroNormalizeAction(actions[i]));
    for(var j = 0;j<macroFiles.length;j++)
    {
        if(parseInt(macroFiles[j].id) == id)
        {
            macroFiles[j] = item;
            return 1;
        }
    }
    macroFiles.push(item);
    return 1;
}

function macroRenderList()
{
    var macroList = document.getElementById("macroList");
    if(macroList == null) return 0;

    macroList.innerHTML = "";
    var shownCurrent = 0;
    for(var i = 0;i<macroFiles.length;i++)
    {
        var macro = macroFiles[i];
        if(parseInt(macro.id) == parseInt(currentMacro.id))
        {
            macro = {id:currentMacro.id, mode:currentMacro.mode, actions:currentMacro.actions};
            shownCurrent = 1;
        }
        macroRenderCard(macroList, macro, parseInt(macro.id) == parseInt(currentMacro.id));
    }
    if(shownCurrent == 0 && (currentMacro.actions.length > 0 || macroIsDirty()))
    {
        macroRenderCard(macroList, currentMacro, true);
    }
}

function macroRenderCard(root, macro, selected)
{
    var card = document.createElement("div");
    card.className = selected ? "macroCard macroCardActive" : "macroCard";
    card.setAttribute("data-id", macro.id);
    card.onclick = function(v){ macroLoadById(parseInt(v.currentTarget.getAttribute("data-id"))); };

    var info = document.createElement("div");
    info.className = "macroCardInfo";

    var idBox = document.createElement("div");
    idBox.className = "macroActionBox macroInfoBox";
    idBox.innerHTML = "M" + macro.id;
    info.appendChild(idBox);

    var modeBox = document.createElement("div");
    modeBox.className = "macroActionBox macroInfoBox macroModeBox";
    modeBox.innerHTML = macroModeText(macro.mode);
    info.appendChild(modeBox);

    var countBox = document.createElement("div");
    countBox.className = "macroActionBox macroInfoBox macroCountBox";
    countBox.innerHTML = (macro.actions || []).length + i18nText("macroCountUnit");
    info.appendChild(countBox);
    card.appendChild(info);

    var actionRoot = document.createElement("div");
    actionRoot.className = "macroCardActions";

    var actions = macro.actions || [];
    for(var i = 0;i<actions.length;i++)
    {
        var box = document.createElement("div");
        box.className = macroActionClass(actions[i]) + (selected && i == macroSelectedIndex ? " macroActionSelected" : "");
        box.innerHTML = macroActionLabel(actions[i]);
        box.setAttribute("data-index", i);
        box.onclick = function(v){
            v.stopPropagation();
            macroPendingIndex = parseInt(v.currentTarget.getAttribute("data-index"));
            var id = parseInt(card.getAttribute("data-id"));
            if(id == parseInt(currentMacro.id))
            {
                macroSetSelected(macroPendingIndex);
                macroPendingIndex = -1;
            }
            else
            {
                macroLoadById(id);
            }
        };
        actionRoot.appendChild(box);
    }
    card.appendChild(actionRoot);
    root.appendChild(card);
}

function macroLoadObject(obj)
{
    if(obj.maxActions != null) macroMaxActions = obj.maxActions;
    currentMacro = {
        id: parseInt(obj.id) || 1,
        mode: parseInt(obj.mode) || 0,
        actions: []
    };
    document.getElementById("macroId").value = currentMacro.id;
    document.getElementById("macroMode").value = currentMacro.mode;
    var actions = obj.actions || [];
    for(var i = 0;i<actions.length;i++) currentMacro.actions.push(macroNormalizeAction(actions[i]));
    macroSelectedIndex = macroPendingIndex;
    macroPendingIndex = -1;
    if(macroSelectedIndex >= currentMacro.actions.length) macroSelectedIndex = -1;
    macroSavedState = macroCurrentState();
    macroRender();
}

function macroShow(obj)
{
    macroStoreFile(obj);
    macroLoadObject(obj);
}

function macroListShow(obj)
{
    if(obj.maxActions != null) macroMaxActions = obj.maxActions;
    var ids = obj.ids || [];
    if(ids.length == 0 && obj.macros != null)
    {
        for(var m = 0;m<obj.macros.length;m++) ids.push(parseInt(obj.macros[m].id));
    }
    macroFiles = [];
    for(var n = 0;n<ids.length;n++)
    {
        var id = parseInt(ids[n]) || 0;
        if(id > 0) macroFiles.push({id:id, mode:0, actions:[]});
    }
    macroBindRender();
    if(macroFiles.length > 0)
    {
        var loadId = parseInt(macroFiles[0].id);
        var currentId = parseInt(currentMacro.id) || loadId;
        for(var i = 0;i<macroFiles.length;i++)
        {
            if(parseInt(macroFiles[i].id) == currentId)
            {
                loadId = currentId;
                break;
            }
        }
        macroLoadById(loadId, 1);
        for(var j = 0;j<macroFiles.length;j++)
        {
            var id = parseInt(macroFiles[j].id);
            if(id != loadId) macroFetchListItem(id);
        }
    }
    else
    {
        macroLoadObject({id:1, mode:0, actions:[], maxActions:macroMaxActions});
    }
}

function macroFetchListItem(id)
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/getMacro');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        macroStoreFile(JSON.parse(v.currentTarget.responseText));
        macroBindRender();
        macroRender();
    };
    req.send(JSON.stringify({id:id}));
}

function macroLoadById(id, force)
{
    if(force == null) force = 0;
    if(force == 0 && macroIsDirty()) return 0;
    document.getElementById("macroId").value = id;
    macroGet();
}

function macroNew()
{
    if(macroIsDirty()) return 0;
    var id = macroNextId();
    if(id == 0) return 0;
    currentMacro = {id:id, mode:0, actions:[]};
    macroSelectedIndex = -1;
    macroPendingIndex = -1;
    document.getElementById("macroId").value = id;
    document.getElementById("macroMode").value = 0;
    macroSavedState = "";
    macroRender();
}

function macroDeleteCurrent()
{
    if(confirm((magic63Lang == "en" ? "Delete M" : "删除 M") + currentMacro.id + "?") == false) return 0;
    if(macroIsSaved(currentMacro.id) == 0)
    {
        macroLoadObject({id:1, mode:0, actions:[], maxActions:macroMaxActions});
        macroListGet();
        return 0;
    }

    var req = new XMLHttpRequest;
    req.open('POST', '/api/deleteMacro');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        currentMacro = {id:1, mode:0, actions:[]};
        macroSelectedIndex = -1;
        macroSavedState = macroCurrentState();
        macroListGet();
        fsInfoGet();
    };
    req.send(JSON.stringify({id:parseInt(currentMacro.id) || 1}));
}

function macroGet()
{
    var req = new XMLHttpRequest;
    var id = parseInt(document.getElementById("macroId").value);
    req.open('POST', '/api/getMacro');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        macroShow(JSON.parse(v.currentTarget.responseText));
    };
    req.send(JSON.stringify({id:id}));
}

function macroListGet()
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/listMacro');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        macroListShow(JSON.parse(v.currentTarget.responseText));
    };
    req.send('{"state":1}');
}

function macroSave()
{
    if(!macroIsDirty()) return 0;
    var req = new XMLHttpRequest;
    var data = {
        id: parseInt(currentMacro.id) || 1,
        mode: parseInt(currentMacro.mode) || 0,
        actions: macroCollectActions()
    };
    req.open('POST', '/api/setMacro');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        macroSavedState = macroCurrentState();
        macroUpdateButtons();
        macroListGet();
        fsInfoGet();
    };
    req.send(JSON.stringify(data));
}

function macroEditorInit()
{
    var macroId = document.getElementById("macroId");
    if(macroId == null) return 0;
    document.getElementById("macroMode").addEventListener('change', function(v){ currentMacro.mode = parseInt(v.target.value) || 0; macroRender(); });
    document.getElementById("macroNew").addEventListener('click', macroNew);
    document.getElementById("macroDelete").addEventListener('click', macroDeleteCurrent);
    document.getElementById("macroSave").addEventListener('click', macroSave);
    document.getElementById("macroAddAction").addEventListener('click', macroAddKeyAction);
    document.getElementById("macroAddDelay").addEventListener('click', macroAddDelay);
    document.getElementById("macroDeleteDelay").addEventListener('click', macroDeleteDelay);
    document.getElementById("macroDeleteAction").addEventListener('click', macroDeleteAction);
    document.getElementById("macroTypeDown").addEventListener('click', function(){ macroSetSelectedType(1); });
    document.getElementById("macroTypeTap").addEventListener('click', function(){ macroSetSelectedType(3); });
    document.getElementById("macroTypeUp").addEventListener('click', function(){ macroSetSelectedType(2); });
    document.getElementById("macroDelayValue").addEventListener('change', macroDelayChanged);
    macroUpdateButtons();
    macroListGet();
}

function formatBytes(v)
{
    if(v >= 1024 * 1024) return (v / 1024 / 1024).toFixed(1) + " MiB";
    if(v >= 1024) return (v / 1024).toFixed(1) + " KiB";
    return v + " B";
}

function fsInfoShow(obj)
{
    var show = document.getElementById("fsInfo");
    if(show == null) return 0;
    magic63LastFsInfo = obj;
    show.innerHTML = i18nText("fsInfo", {free: formatBytes(obj.fsFree), total: formatBytes(obj.fsTotal)});
}

function fsInfoGet()
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/fsInfo');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) return 0;
        fsInfoShow(JSON.parse(v.currentTarget.responseText));
    };
    req.send('{"state":1}');
}

function versionInfoShow(obj)
{
    var show = document.getElementById("versionInfo");
    if(show == null) return 0;
    magic63LastVersionInfo = obj;
    var firmwareVersion = obj && obj.firmware ? obj.firmware : "--";
    var uploadPageVersion = obj && obj.uploadPage ? obj.uploadPage : "--";
    var configVersion = obj && obj.configVersion !== undefined ? obj.configVersion : "--";
    var usbBcdDevice = obj && obj.usbBcdDevice ? obj.usbBcdDevice : "--";
    show.innerHTML = i18nText("versionInfo", {
        firmware: firmwareVersion,
        config: configVersion,
        usb: usbBcdDevice,
        settings: MAGIC63_SETTINGS_PAGE_VERSION,
        upload: uploadPageVersion
    });
}

function versionInfoGet()
{
    var req = new XMLHttpRequest;
    req.open('POST', '/api/versionInfo');
    req.setRequestHeader('content-type', 'application/json');
    req.onreadystatechange = function(v) {
        if(v.currentTarget.readyState != 4) return 0;
        if(v.currentTarget.status != 200) {
            versionInfoShow({firmware: "--"});
            return 0;
        }
        versionInfoShow(JSON.parse(v.currentTarget.responseText));
    };
    req.send('{"state":1}');
}

function keymapSet(id,value)//设置指定按键的键值
{
    var temp = '{"id":0,"value":0}';
    var obj = JSON.parse(temp);

    obj.id = id;
    obj.value = value;
    temp = JSON.stringify(obj);

    console.log(temp)
    webDataRequest('setKeyValue',temp);
}


function keymapGetData(v)//获取指定层的数据，会同时将当前层设置成激活层
{
    var temp = '{"layer":0}';
    
    var obj = JSON.parse(temp);
    obj.layer = v;
    temp = JSON.stringify(obj);
    console.log(temp)
    webDataRequest('getLaye',temp);
}

/*以下function 是处理服务器端返回的数据用的 */

function webDataHandle(v)//发送数据回调
{ 
    if(v.currentTarget.readyState != 4) return 0 ;
    //console.log(v,v.currentTarget.readyState,v.currentTarget.status)
    if (v.currentTarget.readyState == 4 && v.currentTarget.status == 200) {
        //console.log(v.currentTarget.responseText)
        webServerDataHandle(v.currentTarget.responseText);
    }
}

function layerKeylisten() //键盘按键监听
{
    var idHand = "layer"
    for(var i = 1;i<5;i++)
    {
        var that = document.getElementById(idHand+i);
        if(that != null)
        {
            var tempId = '#' + that.id;
            var str = that.getAttribute('name') + ":" + "\"" + that.id + "\",";
            document.querySelector(tempId).addEventListener('click', layerSelectKeyHandle);
        }
    }
}

//根据从设备获取到的数据更新页面 
function webServerDataHandle(v)
{
    var obj = JSON.parse(v);

    if(obj.reload != null)
    {
        location.reload();
        return ;
    }
    if(obj.keyMap != null)
    {

        for(var layerIndex = 1;layerIndex < 5;layerIndex++)
        {
            var layerObj = document.getElementById("layer" + layerIndex);
            if(layerObj != null) layerObj.style.background = "#ffffff";
        }
        var activeLayerObj = document.getElementById("layer" + obj.layer);
        if(activeLayerObj != null) activeLayerObj.style.background = "red";

        for(var i = 0; i<80 ; i++)
        {
             
            var idShow = null;
            console.log(i,obj.keyMap[i])
            if(obj.keyMap[i] == 0xffff) continue;
            if((obj.keyMap[i] >> 8)  == 2 )
            {
                var number = (obj.keyMap[i]  & 0xff);
                var tempID ='IDM' + number ;
                var tempObj = document.getElementById(tempID);
                if(tempObj != null) idShow = tempObj.getAttribute("name");
            }
            else if((obj.keyMap[i] >> 8)  == 1 )
            {
                var number = (obj.keyMap[i]  & 0xff);
                var tempID ='IDS' + number ;
                var tempObj = document.getElementById(tempID);
                if(tempObj != null) idShow = tempObj.getAttribute("name");
            }
            else if((obj.keyMap[i] >> 8)  == 3 )
            {
                var number = (obj.keyMap[i]  & 0xff);
                idShow = "M" + number;
            }
            else if((obj.keyMap[i] >> 8)  == 5 )
            {
                var number = (obj.keyMap[i]  & 0xff);
                var tempID = (number & 0x80) ? ('IDQ' + (number & 0x7f)) : ('IDP' + number);
                var tempObj = document.getElementById(tempID);
                if(tempObj != null) idShow = tempObj.getAttribute("name");
            }
            else if((obj.keyMap[i] >> 8)  > 0 ) 
            {
                continue; //等下处理
            }
            else
            {
                if(obj.keyMap[i] < 8)
                {
                    var number = obj.keyMap[i] + 1;
                    var tempID  ='ID' + number;
                    var tempObj = document.getElementById(tempID);
                    if(tempObj != null) idShow = tempObj.innerHTML
                }
                else
                {
                    var number = obj.keyMap[i] -8;
                    var temp = number.toString(16);
                    var temp2 = temp.toUpperCase()
                    var tempID ='ID0x' + temp2;
                    var tempObj = document.getElementById(tempID);
                    if(tempObj != null) idShow = tempObj.innerHTML
                }
            }
            //console.log(idShow)
            if(idShow == null) continue ;
            var idKey = "keyId" + (i+1);
            if(document.getElementById(idKey) == null) continue ;
            
            document.getElementById(idKey).innerHTML = idShow

        
        }
    }
}
