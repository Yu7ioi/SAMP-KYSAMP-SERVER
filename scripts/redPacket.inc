#define MAX_REDPACKETS              (10)
#define MAX_REDPACKET_WORDS_LEN     (64)
#define REDPACKET_TIME_REQUIRED     (800)
#define DIALOG_REDP                 (1300)
#define DIALOG_REDP_TYPE            DIALOG_REDP + 1
#define DIALOG_REDP_MONEY           DIALOG_REDP + 2
#define DIALOG_REDP_AMOUNT          DIALOG_REDP + 3
#define DIALOG_REDP_WORD            DIALOG_REDP + 4
#define REDPACKET_RETURN_TIME       (10)//十分钟内没人领完就返回给发的人
#define REDPACKET_MONEY_LEAST       (10000)//最少发

enum redpackettypes {
    Id,
    Name[MAX_PLAYER_NAME],
    MoneyS,//最初金额
    MoneyN,//当前金额
    Amount,
    Word[MAX_REDPACKET_WORDS_LEN],
    TypeId,
    Created_At,
    ReceivedIds[MAX_PLAYERS],
    ReceivedCount
};

new RedPacketTypeNames[3][10] = {"普通红包", "运气红包", "口令红包"};
new RedPacket[MAX_REDPACKETS][redpackettypes], pRedPId[MAX_PLAYERS];

redPacketRuntime(unixnow) {
    for(new i = 0; i < MAX_REDPACKETS; i ++) {
        if(RedPacket[i][Id] != 0) {
            if(unixnow - RedPacket[i][Created_At] > (REDPACKET_RETURN_TIME * 60)) {
                RedPacketInit(i);
            }
        }
    }
}

stock RedPacketInit(id) {
    new msg[144], pid = isNameOnline(RedPacket[id][Name]);
    if(RedPacket[id][MoneyN] == 0) {
        new sec = gettime() - RedPacket[id][Created_At];
        format(msg, sizeof msg, "[红包] %s 发的%s在%d秒内被领完了.", RedPacket[id][Name], RedPacketTypeNames[RedPacket[id][TypeId]], sec);
        SCMToAll(COLOR_RED, msg);
    } else {
        if(pid != -1) {
            GiveSavings(pid, RedPacket[id][MoneyN]);
            log_money(RedPacket[id][Name], GetIp(pid), GetPlayerUID(pid), "redpacket", "红包未领完返回", 0, RedPacket[id][MoneyN], "红包未领完返回");
            format(msg, sizeof msg, "[红包] 你的%s还有$%d没人领, 已返回到你的银行余额", RedPacketTypeNames[RedPacket[id][TypeId]], RedPacket[id][MoneyN]);
            SCM(pid, COLOR_RED, msg);
        }
    }
    if(pid != -1) {
        pRedPId[pid] = -1;
    }
    RedPacket[id][Id] = 0;
    format(RedPacket[id][Name], 2, "");
    RedPacket[id][MoneyS] = 0;
    RedPacket[id][MoneyN] = 0;
    RedPacket[id][Amount] = 0;
    format(RedPacket[id][Word], 2, "");
    RedPacket[id][TypeId] = 0;
    RedPacket[id][Created_At] = 0;
}

stock RedPacketDisconnect(playerid) {
    if(pRedPId[playerid] != -1) {
        RedPacketInit(pRedPId[playerid]);
    }
}

stock RedPacketConnect(playerid) {
    pRedPId[playerid] = -1;
}

stock GetRedPacketNullId() {
    for(new i = 0; i < MAX_REDPACKETS; i ++) {
        if(RedPacket[i][Id] == 0) {
            return i;
        }
    }
    return -1;
}

forward REDPCODR(playerid, dialogid, response, listitem, inputtext[]);
public REDPCODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == DIALOG_REDP_TYPE) {
        if(!response) {
            RedPacketConnect(playerid);
            return 1;
        }
        if(pRedPId[playerid] != -1) {
            DIALOG_node(playerid, "你刚刚发的红包还没被领完, 请等等再发");
            return 1;
        }
        new id = GetRedPacketNullId();
        if(id == -1) {
            DIALOG_node(playerid, "当前红包数量已达到上限");
            return 1;
        }
        for(new i = 0; i < MAX_PLAYERS; i ++) {
            RedPacket[id][ReceivedIds][i] = -1;
        }
        RedPacket[id][ReceivedCount] = 0;
        RedPacket[id][Created_At] = gettime();
        pRedPId[playerid] = id;
        RedPacket[id][Id] = id + 1;
        format(RedPacket[id][Name], MAX_PLAYER_NAME, GetName(playerid));
        RedPacket[id][TypeId] = listitem;
        RedPacketSet(playerid, 1);
        return 1;
    }
    if(dialogid == DIALOG_REDP_MONEY) {
        if(!response) {
            RedPacketSet(playerid, 0);
            return 1;
        }
        new puts[128];
        format(puts, sizeof puts, inputtext);
        if(strcmp(puts, defaultIT) == 0) {
            RedPacketSet(playerid, 1, "金额不能为空");
            return 1;
        }
        new money = strval(puts);
        if(money < REDPACKET_MONEY_LEAST) {
            new msg[88];
            format(msg, sizeof msg, "金额不能低于$%d", REDPACKET_MONEY_LEAST);
            RedPacketSet(playerid, 1, msg);
            return 1;
        }
        if(GetSavings(playerid) < money) {
            RedPacketSet(playerid, 1, "你的银行余额不足");
            return 1;
        }
        RedPacket[pRedPId[playerid]][MoneyS] = money;
        RedPacket[pRedPId[playerid]][MoneyN] = money;
        RedPacketSet(playerid, 2);
        return 1;
    }
    if(dialogid == DIALOG_REDP_AMOUNT) {
        if(!response) {
            RedPacketSet(playerid, 1);
            return 1;
        }
        new puts[128];
        format(puts, sizeof puts, inputtext);
        if(strcmp(puts, defaultIT) == 0) {
            RedPacketSet(playerid, 1, "数量不能为空");
            return 1;
        }
        new msg[144], amount = strval(puts);
        if(amount < 1 || amount > GetPlayerPoolSize()+1) {
            format(msg, sizeof msg, "数量不能低于1或者大于%d", GetPlayerPoolSize()+1);
            RedPacketSet(playerid, 2, msg);
            return 1;
        }
        new id = pRedPId[playerid];
        RedPacket[id][Amount] = amount;
        if(RedPacket[id][TypeId] == 2) {
            RedPacketSet(playerid, 3);
        } else {
            format(msg, sizeof msg, "[红包] %s(%d) 发了一个%s, 输入'/hb %d'开始抢吧",
            RedPacket[id][Name], playerid, RedPacketTypeNames[RedPacket[id][TypeId]], id);
            SCMToAll(COLOR_RED, msg);
            GiveSavings(playerid, -RedPacket[id][MoneyN]);
            format(msg, sizeof msg, "发%s", RedPacketTypeNames[RedPacket[id][TypeId]]);
            log_money(RedPacket[id][Name], GetIp(playerid), GetPlayerUID(playerid), "redpacket", msg, 0, -RedPacket[id][MoneyN], msg);
        }
        return 1;
    }
    if(dialogid == DIALOG_REDP_WORD) {
        if(!response) {
            RedPacketSet(playerid, 2);
            return 1;
        }
        new puts[128];
        format(puts, sizeof puts, inputtext);
        if(strcmp(puts, defaultIT) == 0) {
            RedPacketSet(playerid, 3, "口令不能为空");
            return 1;
        }
        new msg[144];
        if(strlen(puts) > MAX_REDPACKET_WORDS_LEN) {
            format(msg, sizeof msg, "口令长度不能大于%d字符", MAX_REDPACKET_WORDS_LEN);
            RedPacketSet(playerid, 3, msg);
            return 1;
        }
        new id = pRedPId[playerid];
        format(RedPacket[id][Word], MAX_REDPACKET_WORDS_LEN, puts);
        format(msg, sizeof msg, "[红包] %s(%d) 发了一个%s, 打出'%s'抢吧!!!",
        RedPacket[id][Name], playerid, RedPacketTypeNames[RedPacket[id][TypeId]], puts);
        SCMToAll(COLOR_RED, msg);
        GiveSavings(playerid, -RedPacket[id][MoneyN]);
        format(msg, sizeof msg, "发%s", RedPacketTypeNames[RedPacket[id][TypeId]]);
        log_money(RedPacket[id][Name], GetIp(playerid), GetPlayerUID(playerid), "redpacket", msg, 0, -RedPacket[id][MoneyN], msg);
        return 1;
    }
    return 1;
}

stock RedPacketSet(playerid, stepid, err[] = " ") {
    new msg[144];
    if(stepid == 0) {
        ShowPlayerDialog(playerid, DIALOG_REDP_TYPE, DIALOG_STYLE_LIST, "请选择要发送的红包类型", "普通红包\n运气红包\n口令红包", "下一步", "关闭");
    } else if(stepid == 1) {
        format(msg, sizeof msg, "请输入红包总金额\n{FF0000}%s", err);
        ShowPlayerDialog(playerid, DIALOG_REDP_MONEY, DIALOG_STYLE_INPUT, " ", msg, "下一步", "返回");
    } else if(stepid == 2) {
        format(msg, sizeof msg, "请输入红包数量\n{FF0000}%s", err);
        ShowPlayerDialog(playerid, DIALOG_REDP_AMOUNT, DIALOG_STYLE_INPUT, " ", msg, "下一步", "返回");
    } else if(stepid == 3) {
        format(msg, sizeof msg, "请输入红包口令\n{FF0000}%s", err);
        ShowPlayerDialog(playerid, DIALOG_REDP_WORD, DIALOG_STYLE_INPUT, " ", msg, "下一步", "返回");
    }
}

stock RedPacketReceive_Wrods(playerid, text[]) {
    new id = -1;
    for(new i = 0; i < MAX_REDPACKETS; i ++) {
        if(RedPacket[i][Id] != 0 && RedPacket[i][TypeId] == 2) {
            if(strcmp(text, RedPacket[i][Word]) == 0) {
                id = i;
                i = MAX_REDPACKETS + 1;
            }
        }
    }
    if(id != -1) {
        RedPacketReceive(playerid, id);
    }
}

stock bool:RedPacketDidIReceivedIt(playerid, id) {
    for(new i = 0; i < RedPacket[id][ReceivedCount]; i ++) {
        if(RedPacket[id][ReceivedIds][i] == playerid) {
            return true;
        }
    }
    return false;
}

stock RedPacketReceive(playerid, id) {
    if(RedPacket[id][MoneyN] == 0) {
        SCM(playerid, COLOR_RED, "[红包] 这个红包已经被领完了.");
    } else {
        if(RedPacketDidIReceivedIt(playerid, id) == true) {
            DIALOG_node(playerid, "你已经领过这个红包了, 不能重复领取");
        } else {
            RedPacket[id][ReceivedIds][RedPacket[id][ReceivedCount]] = playerid;
            RedPacket[id][ReceivedCount] ++;
            new msg[144], money;
            if(RedPacket[id][TypeId] == 0) {
                money = RedPacket[id][MoneyN] / RedPacket[id][Amount];
            } else {
                money = random(RedPacket[id][MoneyN]);
                if(RedPacket[id][Amount] == 1) {
                    money = RedPacket[id][MoneyN];
                }
            }
            RedPacket[id][MoneyN] -= money;
            RedPacket[id][Amount] --;
            GiveSavings(playerid, money);
            format(msg, sizeof msg, "[红包]收到了一个%s发的%s", RedPacket[id][Name], RedPacketTypeNames[RedPacket[id][TypeId]]);
            log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "redpacket", "收红包", 0, money, msg);
            format(msg, sizeof msg, "[红包] 你收了一个 %s 发的%s +$%d", RedPacket[id][Name], RedPacketTypeNames[RedPacket[id][TypeId]], money);
            SCM(playerid, COLOR_RED, msg);
            format(msg, sizeof msg, "[红包] %s(%d) 收了一个 %s 发的%s,红包金额:$%d 还剩%d个.",
			GetName(playerid), playerid, RedPacket[id][Name], RedPacketTypeNames[RedPacket[id][TypeId]], money, RedPacket[id][Amount]);
            SCMToAll(COLOR_RED, msg);
            if(RedPacket[id][Amount] == 0) {
                RedPacketInit(id);
            }
        }
    }
}

CMD:hb(playerid, params[]) {
    if(strcmp(params, "setse", true) == 0) {
        if(GetPlayerOTime(playerid) < REDPACKET_TIME_REQUIRED) {
            new msg[128];
            format(msg, sizeof msg, "发红包需要在线时间超过%d分钟", REDPACKET_TIME_REQUIRED);
            DIALOG_node(playerid, msg);
            return 1;
        }
        RedPacketSet(playerid, 0);
        return 1;
    }
    new id = strval(params);
    if(id < 0 || id > MAX_REDPACKETS) {
        SCM(playerid, COLOR_RED, "[红包] 错误的红包ID.");
        return 1;
    }
    if(RedPacket[id][Id] == 0 || RedPacket[id][TypeId] == 2) {
        SCM(playerid, COLOR_RED, "[红包] 错误的红包ID.");
        return 1;
    }
    RedPacketReceive(playerid, id);
    return 1;
}