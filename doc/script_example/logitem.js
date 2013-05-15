var center = application.GetLogCenter();
var count = center.GetLogCount();

var l = 0;
var ts = 0;
var te = 0;
for (var i = 0; i < count; i++) {
    var item = center.GetLogItem(i);
    l += item.content.length;
    if (i == 0) ts = item.time;
    if (i == count-1) te = item.time;
}

// msgbox
application.MessageBox("日志总长度: " + l);
application.MessageBox("日志时间跨度： " + (te-ts));