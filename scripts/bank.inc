#define DIALOG_BANK                 (200)//这个修改之后需要重新设置好友转账dialogid
#define DIALOG_BANK_DEPOSIT         DIALOG_BANK + 1
#define DIALOG_BANK_WITHDRAW        DIALOG_BANK + 2
#define DIALOG_BANK_TRANSFER        DIALOG_BANK + 3//这个修改之后需要重新设置好友转账dialogid
#define DIALOG_BANK_TRANSFER_TO     DIALOG_BANK + 4
#define BANK_OPTION_DEPOSIT         (0)
#define BANK_OPTION_WITHDRAW        (1)
#define BANK_OPTION_TRANSFER        (2)
#define BANK_OPTION_TRANSFER_TO     (3)
#define BAN_TRANSFER_TIME           (400)

enum banCQZposstruct {
    Float:X,
    Float:Y,
    Float:Z,
    Float:R,
    OptionID
}

new banCQZpos[3][banCQZposstruct] = {
    {1489.2218, 758.4795, 10.9859, 1.5, BANK_OPTION_DEPOSIT},
    {1493.3206, 758.4792, 10.9859, 1.5, BANK_OPTION_WITHDRAW},
    {1497.3821, 758.4822, 10.9859, 1.5, BANK_OPTION_TRANSFER}
};

new pBankTransToPID[MAX_PLAYERS];

bankInit() {
    new msg[44];
    for(new i = 0; i < sizeof banCQZpos; i ++) {
        if(banCQZpos[i][OptionID] == BANK_OPTION_DEPOSIT) msg = "存款\n按'N'操作";
        else if(banCQZpos[i][OptionID] == BANK_OPTION_WITHDRAW) msg = "取款\n按'N'操作";
        else if(banCQZpos[i][OptionID] == BANK_OPTION_TRANSFER) msg = "转账\n按'N'操作";
        CreateDynamic3DTextLabel(msg, COLOR_WHITE, banCQZpos[i][X], banCQZpos[i][Y], banCQZpos[i][Z], 10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID);
    }
}

stock bankShowDialog(playerid, id, err[] = " ") {
    new title[88], msg[128];
    format(title, sizeof title, "{FF0000}%s", err);
    if(id == BANK_OPTION_DEPOSIT) {
        format(msg, sizeof msg, "存款 -> 请输入金额(当前拥有存款 $%d)", GetSavings(playerid));
        ShowPlayerDialog(playerid, DIALOG_BANK_DEPOSIT, DIALOG_STYLE_INPUT, title, msg, "确定", "关闭");
    } else if(id == BANK_OPTION_WITHDRAW) {
        format(msg, sizeof msg, "取款 -> 请输入金额(当前拥有存款 $%d)", GetSavings(playerid));
        ShowPlayerDialog(playerid, DIALOG_BANK_WITHDRAW, DIALOG_STYLE_INPUT, title, msg, "确定", "关闭");
    } else if(id == BANK_OPTION_TRANSFER) {
        format(msg, sizeof msg, "转账 -> 请输入对方ID(当前拥有存款 $%d)", GetSavings(playerid));
        ShowPlayerDialog(playerid, DIALOG_BANK_TRANSFER, DIALOG_STYLE_INPUT, title, msg, "下一步", "关闭");
    } else if(id == BANK_OPTION_TRANSFER_TO) {
        format(msg, sizeof msg, "转账给 %s, 请输入金额", GetName(pBankTransToPID[playerid]));
        ShowPlayerDialog(playerid, DIALOG_BANK_TRANSFER_TO, DIALOG_STYLE_INPUT, title, msg, "转账", "关闭");
    }
}

bankOPKSC(playerid, newkeys) {
    if(newkeys == 131072) {
        new id = isPlayerInbanCQZposRange(playerid);
        if(id == -1) return 0;
        bankShowDialog(playerid, id);
        return 1;
    }
    return 0;
}

isPlayerInbanCQZposRange(playerid) {
    for(new i = 0; i < sizeof banCQZpos; i ++) {
        if(IsPlayerInRangeOfPoint(playerid, banCQZpos[i][R], banCQZpos[i][X], banCQZpos[i][Y], banCQZpos[i][Z])) {
            return banCQZpos[i][OptionID];
        }
    }
    return -1;
}

forward bankODR(playerid, dialogid, response, listitem, inputtext[]);
public bankODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == DIALOG_BANK_TRANSFER_TO) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER_TO, "金额不能为空");
            return 1;
        }
        new money = strval(inputtext);
        if(money < 1) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER_TO, "错误的金额");
            return 1;
        }
        if(money > GetSavings(playerid)) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER_TO, "你没有这么多的存款");
            return 1;
        }
        new pid = pBankTransToPID[playerid];
        if(IsPlayerConnected(pid) == 0) {
            DIALOG_node(playerid, "对方已离线");
            return 1;
        }
        if(GetPlayerStatus(pid) == 0) {
            DIALOG_node(playerid, "对方未登录");
            return 1;
        }
        new msg[128];
        if(userInfo[playerid][OnlineTime] < BAN_TRANSFER_TIME) {
            format(msg, sizeof msg, "使用该功能需要在线%d分钟", BAN_TRANSFER_TIME);
            DIALOG_node(playerid, msg);
            return 1;
        }
        GiveSavings(pid, money);
        accountSaveData(pid);
        GiveSavings(playerid, -money);
        accountSaveData(playerid);
        log_money(GetName(playerid), GetIp(playerid), userInfo[playerid][UID], GetName(pid), GetIp(pid), userInfo[pid][UID], -money, "转账");
        log_money(GetName(pid), GetIp(pid), userInfo[pid][UID], GetName(playerid), GetIp(playerid), userInfo[playerid][UID], money, "转账获得");
        format(msg, sizeof msg, "你转给了 %s $%d", GetName(pid), money);
        DIALOG_node(playerid, msg);
        format(msg, sizeof msg, "[银行] %s 转给了你 $%d", GetName(playerid), money);
        SCM(pid, COLOR_BLUE, msg);
        return 1;
    }
    if(dialogid == DIALOG_BANK_TRANSFER) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER, "对方ID不能为空");
            return 1;
        }
        new pid = strval(inputtext);
        if(IsPlayerConnected(pid) == 0 || GetPlayerStatus(pid) == 0) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER, "对方不在线或对方未登录");
            return 1;
        }
        if(pid == playerid) {
            bankShowDialog(playerid, BANK_OPTION_TRANSFER, "你不能转给你自己");
            return 1;
        }
        pBankTransToPID[playerid] = pid;
        bankShowDialog(playerid, BANK_OPTION_TRANSFER_TO);
        return 1;
    }
    if(dialogid == DIALOG_BANK_WITHDRAW) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            bankShowDialog(playerid, BANK_OPTION_WITHDRAW, "金额不能为空");
            return 1;
        }
        new money = strval(inputtext);
        if(money < 1) {
            bankShowDialog(playerid, BANK_OPTION_WITHDRAW, "错误的金额");
            return 1;
        }
        if(money > GetSavings(playerid)) {
            bankShowDialog(playerid, BANK_OPTION_WITHDRAW, "你没有这么多的存款");
            return 1;
        }
        new msg[128];
        format(msg, sizeof msg, "bf %d af %d", GetSavings(playerid), GetSavings(playerid)-money);
        GiveMoney(playerid, money);
        GiveSavings(playerid, -money);
        accountSaveData(playerid);
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "bank", msg, 0, -money, "取款");
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "bank", "取款", 0, money, "取款");
        format(msg, sizeof msg, "取款成功, 你取出了 $%d", money);
        DIALOG_node(playerid, msg);
        return 1;
    }
    if(dialogid == DIALOG_BANK_DEPOSIT) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            bankShowDialog(playerid, BANK_OPTION_DEPOSIT, "金额不能为空");
            return 1;
        }
        new money = strval(inputtext);
        if(money < 1) {
            bankShowDialog(playerid, BANK_OPTION_DEPOSIT, "错误的金额");
            return 1;
        }
        if(money > GetMoney(playerid)) {
            bankShowDialog(playerid, BANK_OPTION_DEPOSIT, "你没有这么多的现金");
            return 1;
        }
        new msg[128];
        format(msg, sizeof msg, "bf %d af %d", GetSavings(playerid), GetSavings(playerid)+money);
        GiveMoney(playerid, -money);
        GiveSavings(playerid, money);
        accountSaveData(playerid);
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "bank", msg, 0, money, "存款");
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "bank", "存款", 0, -money, "存款");
        format(msg, sizeof msg, "存款成功, 你存入了 $%d", money);
        DIALOG_node(playerid, msg);
        return 1;
    }
    return 1;
}
