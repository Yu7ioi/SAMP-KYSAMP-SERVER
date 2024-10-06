#define DIALOG_CMDS             (1100)

CMD:help(playerid, params[]) {
    ShowPlayerDialog(playerid, DIALOG_CMDS, DIALOG_STYLE_LIST, "帮助菜单", "\
    账号帮助\n\
    帮派帮助\n\
    爱车帮助\n\
	房屋帮助\n\
    DM帮助\n\
	传送帮助\n\
    rp指令帮助\n\
    ", "选择", "关闭");
    return 1;
}

forward CMDSODR(playerid, dialogid, response, listitem, inputtext[]);
public CMDSODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == DIALOG_CMDS) {
        if(!response) return 1;
        new msg[3223];
        if(listitem == 0) {
            format(msg, sizeof msg, "/zhxx - 查看账号信息\n/xgmz - 修改名字(需要40分钟在线) \n/xgmm - 修改密码(需要40分钟在线)\n/sz - 个人设置\n/yh - 传送到银行(存钱,取钱,转账在此完成)\n--双击我返回上一页--");
        } else if(listitem == 0) {
            format(msg, sizeof msg, "/t list - 查看帮派列表\n/t wode - 查看我的帮派信息\n/t [内容] - 帮派内聊天\n/t goto [id] - 传送到某位帮派成员身边\n/t get [id] - 拉帮派某位成员到身边(仅帮派2级队员可用)\n说明:帮派成员分为三个等级:1级.普通成员,2级.帮派管理员,3级.帮派总管\n--双击我返回上一页--");
        } else if(listitem == 1) {
            format(msg, sizeof msg, "/t list - 查看帮派列表\n/t wode - 查看我的帮派信息\n/t [内容] - 帮派内聊天\n/t goto [id] - 传送到某位帮派成员身边\n/t get [id] - 拉帮派某位成员到身边(仅帮派2级队员可用)\n说明:帮派成员分为三个等级:1级.普通成员,2级.帮派管理员,3级.帮派总管\n--双击我返回上一页--");
        } else if(listitem == 2) {
            format(msg, sizeof msg, "/acgmd - 传送到爱车购买点\n/ac color [ID1] [ID2] - 修改爱车颜色\n/ac list - 查看/编辑爱车的装绑(需要玩家在其中一辆爱车上)\n/ac wode [ID] - 召唤我的爱车(ID为选填,不填默认召唤你的第一辆爱车)\n/ac lock [ID] - 锁住爱车(锁住爱车后其他玩家无法上你的车,ID为选填,不填则默认锁住当前的爱车)\n/ac goto [ID] - 传送到爱车(ID为选填,不填则默认传送到你的第一辆爱车)\n/ac park - 泊车(泊车后,爱车将会保存在这个坐标)\n/ac buy - 购买爱车(需要在爱车购买点)\n");
			format(msg, sizeof msg, "%s/ac buyobj - 购买爱车装扮(需要在爱车装扮购买点)\n/ac sell - 出售爱车(可以选择出售给其他玩家或者半价给系统)\n/ac dl - 显示/隐藏爱车文字标记\n注意:如果超过五个月未登录游戏,您的爱车将被强制收回\n--双击我返回上一页--", msg);
		} else if(listitem == 3) {
            format(msg, sizeof msg, "/houselist - 房屋列表\n/buyhouse - 购买房子\n/sellhouse - 一半的价格出售房子给系统\n/resalehouse - 出售房子给其他玩家\n按键'Y' - 开启/关闭房门\n按键'H' - 操作移动门\n/changehousename - 修改房名\n/adp- 添加某人某房子的权限\n/rdp - 删除某人某房子的权限\n/gdp - 查询某人是否拥有某房子的权限\n/plist - 查看指定房子的权限列\n注意:如果超过三个月未登录游戏,您的房子将被强制收回\n--双击我返回上一页--");
		} else if(listitem == 4) {
            format(msg, sizeof msg, "/dm [ID] - 进入DM\n/dm exit - 退出DM\n--双击我返回上一页--");
        } else if(listitem == 5) {
            format(msg, sizeof msg, "/telemenu - 官方传送点列表\n/vmake - 自制传送点\n/tp [ID] - 传送到某位玩家身边\n--双击我返回上一页--");
		} else if(listitem == 6) {
            format(msg, sizeof msg, "/hy - 好友系统(好友转账省去去银行,还可以互相传送)\n/fxq - 召唤火箭飞行器\n/sz - 个性化设置\n/ww - 切换到小世界(免受其他玩家打扰)\n/wuqi - 刷武器\n/chattail - 购买/编辑小尾巴\n/wdch - 查看/编辑我的称号\n/pm [ID] - 私聊对方\n/cdk - 兑换CDK\n/dongzuo - 动作帮助\n/c list - 图片刷车\n/stopsong - 停止播放音乐\n/Attire 查看装扮\n点歌，购买装扮请关注微信公众号:samp狂野之城\n--双击我返回上一页--");
        /*} else if(listitem == 7) {
		    format(msg, sizeof msg, ＂/me 动作信息 /do动作消息 /s 周围大喊＂);*/
		}
        ShowPlayerDialog(playerid, DIALOG_CMDS+1, DIALOG_STYLE_LIST, inputtext, msg, "返回", "关闭");
        return 1;
    }
    if(dialogid == DIALOG_CMDS+1) {
        if(!response) return 1;
        pc_cmd_help(playerid, "ojbk");
        return 1;
    }
    return 1;
}