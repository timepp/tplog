var shell = new ActiveXObject("WScript.Shell");
var tempdir = shell.ExpandEnvironmentStrings("%temp%");
application.MessageBox(tempdir);