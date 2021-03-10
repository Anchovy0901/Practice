var index = 0;
var runIt;
var lists = document.getElementsByClassName("third");
addIndex();
startNum(0);

function startNum(num){
    index = num;
    var temp = lists[index].getElementsByTagName("li");
    temp[0].click();

    var title = temp[0].getElementsByTagName("span");
    
    console.log("当前章节编号："+index+",章节标题："+title[0].innerText);
    start();
}

function addIndex(){
    for(var i = 0; i<lists.length;i++){
        var temp = lists[i].getElementsByTagName("li");
        var title = temp[0].getElementsByTagName("span")[0].innerText;
        temp[0].getElementsByTagName("span")[0].innerText = "#"+i+"#"+title;
    }
}

//开始视频播放
function start(){
    sta = document.getElementsByClassName("play-btn-tip")[0];
    console.log(sta);
    
    console.log("播放----");
    window.clearInterval(runIt);

    runIt= setInterval(next,5000);
    if(sta!=null){
        if(sta.innerText == "播放"){
            console.log("开始播放视频");
            document.getElementsByClassName("play-btn-tip")[0].click();
        }
    }
}

//停止视频播放
function stop(){
    start();
    sta = document.getElementsByClassName("play-btn-tip")[0];
    clearInterval(runIt);
    console.log("暂停----");
    if(sta!=null){
        if(sta.innerText == "暂停"){
            console.log("暂停视频");
            document.getElementsByClassName("play-btn-tip")[0].click();
        }
    }
}

//跳转下一节视频
function next(){
    // startNum(index);
    var video = document.getElementsByClassName("xt_video_player")[0];
    if(video == undefined){
        startNum(++index);
        console.log("作业或者讨论，5秒后跳转下一个视频，下一节编号："+index);
    }
    else if(video.length != 0){
        var staNow = document.getElementsByClassName("play-btn-tip")[0];
        if(staNow.innerText == "播放"){
            console.log("播放视频");
            staNow .click();
        }
        var c= video.currentTime;
        var d = video.duration;
        //不想关闭声音可以把此行代码删掉
        soundClose();
        speed();
        //视频播放进度超过95%跳转下一节视频
        if((c/d)>0.95){
            startNum(++index);
            console.log("本节播放完毕跳转到下一节，下一节编号："+index);
            console.log("本节观看百分比"+c/d);  
        }
    }else {
        console.log("未知错误！");
    }
}
//关闭视频声音
function soundClose(){
    var sound = document.getElementsByClassName("xt_video_player_common_icon_muted");
    if(sound.length == 0){
        document.getElementsByClassName("xt_video_player_common_icon")[0].click();
        console.log("视频声音关闭");
    }
}
//播放速度2.0
function speed(){
    var speed = document.getElementsByClassName("xt_video_player_common_list")[0];
    var speedChild = speed.firstChild;
    speedChild.click();
}