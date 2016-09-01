
var en_us_loc = {
}
var zh_cn_loc = {
	"Share Link": "分享链接",
	"Link has been created!": "链接已经创建！",
	"Copy the link and share it to your friends.": "复制链接分享给好友吧。",
	"Password : ": "密码：",
	"Add Password": "添加密码",
	"Remove": "取消密码",
	"Link : ": "链接：",
	"Stop Sharing": "取消分享",
	"Copy Link": "复制链接",
	"Copy Link & Password": "复制链接及密码",
	"Share to Weibo": "分享到微博",
	"Share to WeChat": "分享到微信",
	"Open WeChat to scan the QR code and share the note with your friends.": "用微信“扫一扫”左侧的二维码，即可把笔记分享给微信好友或朋友圈。",
	"Hide": "隐藏",
	"(%1 Day(s) Left)": "（剩余 %1 天）",
	"(%1 Time(s) Left)": "（剩余 %1 次）",
	"(Unlimited)":"（不限）",
	"Access Limitations": "链接访问限制",
	"Automatically stop sharing after the limit is exceeded.": "超出访问限制后将自动停止分享。",
	"Reading Time": "阅读次数",
	"Sharing Days": "分享天数",
	"30 times": "30次",
	"50 times": "50次",
	"100 times": "100次",
	"Unlimited": "不限",
	"1 day": "1天",
	"3 days": "3天",
	"7 days": "7天",
	"Save Modified": "保存修改",
	"Create link failed!": "创建链接失败！",
	"Please check network and retry.": "请检查网络后重试。",
	"WizNote internal error.(token invalid)": "为知笔记内部错误。（token 失效）",
	"Do you want to create a public link?": "即将为您创建公开链接！",
	"The public link can be visited by anyone, you can add a password or set access limitations for it.": "创建后所有人可以查看，您可为链接添加密码或设置访问限制哦~",
	"Don't prompt": "不再提示",
	"Cancel": "取消",
	"Create Now": "继续创建",
	"Getting information, please waiting...": "正在获取信息，请稍后...",
	"Link copied successfully!": "链接复制成功！",
	"Link and password copied successfully!": "链接及密码复制成功！",
	"Unknown error.": "未知错误。",
	"Canceled successfully!": "取消成功！",
	"Modified successfully!": "修改成功！",
	"Network error! Please check the network and retry.": "网络错误，请检查网络后重试！",
	"Server error.": "服务器错误。",
	"Only VIP user can modify access limitations.": "仅 VIP 用户可以修改访问限制。",
	"Account unverified! Please verify it and retry.": "账号未验证，请验证后重试！",
	"No permission to access the knowledge base.": "没有权限访问知识库。",
	"The note has not been uploaded to server! Please upload it and retry.": "笔记未上传，请上传后重试！",
	"Only VIP user can create link, please retry after upgrading to VIP and syncing with server.": "仅 VIP 用户能创建链接，请升级 VIP 并同步客户端后重试！",
	"Sorry, due to some restriction, you can't use link sharing service temporarily.": "抱歉，由于相关限制，你暂时无法使用分享笔记链接功能。"
};
var zh_tw_loc = {
} ;
//
function GetBrowserLanguage() {
    return external.localLanguage;
}
//

function wizTranslate (str) {

    var ShareLocale = (function() {
        var lng = GetBrowserLanguage().toLowerCase();
        //
        var objLng;
        switch (lng) {
            case "zh_cn":
                objLng = zh_cn_loc;
                break;
            default:
                objLng = en_us_loc;
        };
        //
        return objLng;
    })();

	if (!ShareLocale || !ShareLocale[str])
		return str;
	//
	return ShareLocale[str];
}
function wizInitLang() {
    window.sharelocale_remain_days = wizTranslate("(%1 Day(s) Left)");
    window.sharelocale_remain_times = wizTranslate("(%1 Time(s) Left)");
    window.sharelocale_token_invalid = wizTranslate("WizNote internal error.(token invalid)");
    window.sharelocale_copy_link_succeeded = wizTranslate("Link copied successfully!");
    window.sharelocale_copy_link_password_succeeded = wizTranslate("Link and password copied successfully!");
    window.sharelocale_unknown_error = wizTranslate("Unknown error.");
    window.sharelocale_cancel_succeeded = wizTranslate("Canceled successfully!");
    window.sharelocale_modify_succeeded = wizTranslate("Modified successfully!");
    window.sharelocale_network_error = wizTranslate("Network error! Please check the network and retry.");
    window.sharelocale_server_error = wizTranslate("Server error.");
    window.sharelocale_free_account_set_limit = wizTranslate("Only VIP user can modify access limitations.");
    window.sharelocale_mail_unverified = wizTranslate("Account unverified! Please verify it and retry.");
    window.sharelocale_access_other_kb = wizTranslate("No permission to access the knowledge base.");
    window.sharelocale_note_not_in_server = wizTranslate("The note has not been uploaded to server! Please upload it and retry.");
    window.sharelocale_free_account_create_link = wizTranslate("Only VIP user can create link, please retry after upgrading to VIP and syncing with server.");
    window.sharelocale_blacklist = wizTranslate("Sorry, due to some restriction, you can't use link sharing service temporarily.");
}
