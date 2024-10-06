#define MAX_BILLBOARDS              (500)
#define MAX_BILLBOARD_TEXT          (2048)
#define BILLBOARD_3DTEXT_DISTANCE   (5.0)
#define MAX_PLAYER_BILLBOARD        (4)
#define DIALOG_BILLBOARD            (1000)
#define DIALOG_BILLBOARD_MY         DIALOG_BILLBOARD + 1
#define DIALOG_BILLBOARD_OPTIONS    DIALOG_BILLBOARD + 2
#define DIALOG_BILLBOARD_BCOLOR     DIALOG_BILLBOARD + 3
#define DIALOG_BILLBOARD_FCOLOR     DIALOG_BILLBOARD + 4
#define DIALOG_BILLBOARD_FONTSIZE   DIALOG_BILLBOARD + 5
#define DIALOG_BILLBOARD_FONTFACE   DIALOG_BILLBOARD + 6
#define DIALOG_BILLBOARD_TEXT       DIALOG_BILLBOARD + 7
#define DIALOG_BILLBOARD_SALE       DIALOG_BILLBOARD + 8

enum billboardtypes {
    Id,
    OBJId,
    Float:X,
    Float:Y,
    Float:Z,
    Float:RX,
    Float:RY,
    Float:RZ,
    Text[MAX_BILLBOARD_TEXT],
    Idx,
    Size,
    FontFace[MAX_PLAYER_NAME],
    FontSize,
    Bold,
    FontColor,
    BackColor,
    Alignment,
    Owner[MAX_PLAYER_NAME],
    Price,
    Sale,
    TagOId,
    Text3D:TagTId
};

new BillBoard[MAX_BILLBOARDS][billboardtypes], BillBoardCounts, myGGPs[MAX_PLAYERS][MAX_PLAYER_BILLBOARD];
new pBBEditId[MAX_PLAYERS];

new SPDBBColors[][7] = {
    "FF0000",
	"FFA500",
	"FFFF00",
	"00FF00",
	"00FFFF",
	"0000FF",
	"800080",
	"FFFFFF",
	"000000"
};

new BBColors[] = {
	0xFFFF0000,
	0xFFFFA500,
	0xFFFFFF00,
	0xFF00FF00,
	0xFF00FFFF,
	0xFF0000FF,
	0xFF800080,
	0xFFFFFFFF,
	0xFF000000
};

stock BillBoard_changeNtoN(playerid, newname[]) {
	for(new i = 0; i < MAX_PLAYER_BILLBOARD; i ++) {
		new id = myGGPs[playerid][i];
		if(id != -1) {
			if(BillBoard[id][Id] != 0 && strcmp(BillBoard[id][Owner], GetName(playerid)) == 0) {
				format(BillBoard[id][Owner], MAX_PLAYER_NAME, newname);
				BillBoardUpdate_String(id, "owner", newname);
				UpdateBillBoard3DText(id);
			}
		}
	}
}

stock GetNearbyBillBoard(playerid) {
    for(new i = 0; i < BillBoardCounts; i ++) {
        if(BillBoard[i][Id] != 0) {
            if(IsPlayerInRangeOfPoint(playerid, 3.0, BillBoard[i][X], BillBoard[i][Y], BillBoard[i][Z])) {
                return i;
            }
        }
    }
    return -1;
}

CMD:buyggp(playerid, params[]) {
    new id = GetNearbyBillBoard(playerid);
    if(id == -1) {
        DIALOG_node(playerid, "你的周围没有公告牌");
        return 1;
    }
    if(myGGPs[playerid][MAX_PLAYER_BILLBOARD-1] != -1) {
        DIALOG_node(playerid, "你不能购买更多的公告牌了");
        return 1;
    }
    if(BillBoard[id][Sale] == 0) {
        if(strcmp(BillBoard[id][Owner], "*") != 0) {
            DIALOG_node(playerid, "这个公告牌已经被别人买了");
            return 1;
        }
        if(GetMoney(playerid) < BillBoard[id][Price]) {
            DIALOG_node(playerid, "你的钱不够");
            return 1;
        }
        BillBoardBuy(playerid, id, BillBoard[id][Price], 0);
        return 1;
    } else {
        if(GetMoney(playerid) < BillBoard[id][Sale]) {
            DIALOG_node(playerid, "你的钱不够");
            return 1;
        }
        if(strcmp(GetName(playerid), BillBoard[id][Owner]) == 0) {
            DIALOG_node(playerid, "自己买自己的很好玩吗?");
            return 1;
        }
        BillBoardBuy(playerid, id, BillBoard[id][Sale], 1);
    }
    return 1;
}

stock BillBoardBuy(playerid, id, money, typeid) {
    SCM(playerid, COLOR_WHITE, "[系统] 购买处理中, 请稍等...");
    new name[MAX_PLAYER_NAME], msg[128], oldowner[MAX_PLAYER_NAME];
    format(oldowner, sizeof oldowner, BillBoard[id][Owner]);
    format(name, sizeof name, GetName(playerid));
    format(BillBoard[id][Owner], MAX_PLAYER_NAME, name);
    UpdateBillBoard3DText(id);
    BillBoardUpdate_String(id, "owner", name);
    if(typeid == 0) {
        format(msg, sizeof msg, "购买公告牌ID:%d", BillBoard[id][Id]);
        log_money(name, GetIp(playerid), GetPlayerUID(playerid), "billboard", "billboard", 0, -money, msg);
        GiveMoney(playerid, -money);
        accountSaveData(playerid);
        format(msg, sizeof msg, "你购买了一个公告牌");
        DIALOG_node(playerid, msg);
        BillBoardConnect(playerid);
    } else {
        new pid = isNameOnline(oldowner);
        if(pid == -1) {
            GiveMoneyOffline(oldowner, money);
            format(msg, sizeof msg, "出售公告牌ID:%d", BillBoard[id][Id]);
            log_money(oldowner, "offline", GetNameUID(oldowner), name, GetIp(playerid), GetPlayerUID(playerid), money, msg);
            format(msg, sizeof msg, "购买公告牌ID:%d", BillBoard[id][Id]);
            log_money(name, GetIp(playerid), GetPlayerUID(playerid), oldowner, "offline", GetNameUID(oldowner), -money, msg);
        } else {
            format(msg, sizeof msg, "出售公告牌ID:%d", BillBoard[id][Id]);
            log_money(oldowner, GetIp(pid), GetPlayerUID(pid), name, GetIp(playerid), GetPlayerUID(playerid), money, msg);
            GiveMoney(pid, money);
            accountSaveData(pid);
            format(msg, sizeof msg, "购买公告牌ID:%d", BillBoard[id][Id]);
            log_money(name, GetIp(playerid), GetPlayerUID(playerid), oldowner, GetIp(pid), GetPlayerUID(pid), -money, msg);
            BillBoardConnect(pid);
        }
        GiveMoney(playerid, -money);
        accountSaveData(playerid);
        format(msg, sizeof msg, "你从 %s 那里购买了一个公告牌", oldowner);
        DIALOG_node(playerid, msg);
        BillBoardConnect(playerid);
    }
}

stock BillBoardConnect(playerid) {
    for(new l = 0; l < MAX_PLAYER_BILLBOARD; l ++) {
        myGGPs[playerid][l] = -1;
    }
    new myname[MAX_PLAYER_NAME], full = 0;
    format(myname, sizeof myname, GetName(playerid));
    for(new i = 0; i < BillBoardCounts; i ++) {
        if(BillBoard[i][Id] != 0) {
            if(full == 1) {
                break;
            }
            if(strcmp(BillBoard[i][Owner], myname) == 0) {
                for(new l = 0; l < MAX_PLAYER_BILLBOARD; l ++) {
                    if(myGGPs[playerid][l] == -1) {
                        myGGPs[playerid][l] = i;
                        if(l == MAX_PLAYER_BILLBOARD-1) {
                            full = 1;
                        }
						break;
                    }
                }
            }
        }
    }
    pBBEditId[playerid] = -1;
}

CMD:ggp(playerid, params) {
    BillBoardWode(playerid);
    return 1;
}

stock BillBoard_Init() {
    new Cache:result, query[128];
    new dbid, objid, text[MAX_BILLBOARD_TEXT], idx, size, fface[MAX_PLAYER_NAME], fsize, bold, fcolor, bcolor, alignment, owner[MAX_PLAYER_NAME], price, sale, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz;
    format(query, sizeof query, "SELECT * FROM billboard");
    result = mysql_query(mariadb, query);
    cache_get_row_count(BillBoardCounts);
    for(new i = 0; i < BillBoardCounts; i ++) {
        cache_get_value_name_int(i, "id", dbid);
        cache_get_value_name_int(i, "objid", objid);
        cache_get_value_name_float(i, "x", x);
        cache_get_value_name_float(i, "y", y);
        cache_get_value_name_float(i, "z", z);
        cache_get_value_name_float(i, "rx", rx);
        cache_get_value_name_float(i, "ry", ry);
        cache_get_value_name_float(i, "rz", rz);
        cache_get_value_name(i, "text", text, MAX_BILLBOARD_TEXT);
        cache_get_value_name_int(i, "idx", idx);
        cache_get_value_name_int(i, "size", size);
        cache_get_value_name(i, "fontface", fface, MAX_PLAYER_NAME);
        cache_get_value_name_int(i, "fontsize", fsize);
        cache_get_value_name_int(i, "bold", bold);
        cache_get_value_name_int(i, "fontcolor", fcolor);
        cache_get_value_name_int(i, "backcolor", bcolor);
        cache_get_value_name_int(i, "alignment", alignment);
        cache_get_value_name(i, "owner", owner, MAX_PLAYER_NAME);
        cache_get_value_name_int(i, "price", price);
        cache_get_value_name_int(i, "sale", sale);
        CreateBillBoard(i, dbid, objid, text, idx, size, fface, fsize, bold, fcolor, bcolor, alignment, owner, price, sale, x, y, z, rx, ry, rz);
    }
    cache_delete(result);
    printf("billboard -> 已加载%d个公告牌", BillBoardCounts);
}

stock BillBoard_Create(playerid, objid, idx, price) {
    if(BillBoardCounts >= MAX_BILLBOARDS) {
        DIALOG_node(playerid, "公告牌已达到上限");
    } else {
        SCM(playerid, COLOR_WHITE, "[系统] 正在创建公告牌请稍等...");
        new id = BillBoardCounts, Float:x, Float:y, Float:z, msg[2048], Cache:result;
        BillBoardCounts ++;
        GetPlayerPos(playerid, x, y, z);
        SetPlayerPos(playerid, x+2, y+2, z+2);
        format(msg, sizeof msg, "\
        INSERT INTO billboard (objid,x,y,z,rx,ry,rz,text,idx,size,fontface,fontsize,bold,fontcolor,backcolor,alignment,owner,price,sale) VALUES \
        ('%d', '%.3f', '%.3f', '%.3f', '%.3f', '%.3f', '%.3f', '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%s', '%d', '%d')",
        objid, x, y, z, 0.0, 0.0, 0.0, "空", idx, 140, "Arial", 24, 0, 8, 7, 0, "*", price, 0);
        result = mysql_query(mariadb, msg);
        new dbid = cache_insert_id();
        cache_delete(result);
        CreateBillBoard(id, dbid, objid, "空", idx, 140, "Arial", 24, 0, 8, 7, 0, "*", price, 0, x, y, z, 0.0, 0.0, 0.0);
        format(msg, sizeof msg, "创建成功");
        DIALOG_node(playerid, msg);
    }
}

stock BillBoard_Delete(bid) {
    new id = -1;
    for(new i = 0; i < BillBoardCounts; i ++) {
        if(BillBoard[i][Id] == bid) {
            id = i;
            break;
        }
    }
    if(id == -1) {
        return 0;
    }
    new isonline = 0;
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(isonline == 1) {
            break;
        }
        if(IsPlayerConnected(i) == 1) {
            for(new l = 0; l < MAX_PLAYER_BILLBOARD; l ++) {
                if(myGGPs[i][l] != -1) {
                    if(BillBoard[myGGPs[i][l]][Id] == bid) {
                        BillBoardConnect(i);
                        isonline = 1;
                        break;
                    }
                }
            }
        }
    }
    new msg[128];
    format(msg, sizeof msg, "DELETE FROM billboard WHERE id='%d'", bid);
    mysql_query(mariadb, msg, false);
    DestroyBillBoard(id);
    return 1;
}

stock UpdateBillBoardMaterialText(id) {
    for(new i = 0; i < strlen(BillBoard[id][Text]); i ++) {
        if(BillBoard[id][Text][i] == '*') {
            BillBoard[id][Text][i] = '\n';
        }
    }
    SetDynamicObjectMaterialText(BillBoard[id][TagOId], BillBoard[id][Idx], BillBoard[id][Text], BillBoard[id][Size], BillBoard[id][FontFace],
    BillBoard[id][FontSize], BillBoard[id][Bold], BBColors[BillBoard[id][FontColor]], BBColors[BillBoard[id][BackColor]], BillBoard[id][Alignment]);
}

stock UpdateBillBoard3DText(id) {
    if(IsValidDynamic3DTextLabel(BillBoard[id][TagTId]) == 1) {
        DestroyDynamic3DTextLabel(BillBoard[id][TagTId]);
    }
    new msg[128];
    if(BillBoard[id][Sale] == 0) {
        if(strcmp(BillBoard[id][Owner], "*") == 0) {
            format(msg, sizeof msg, "出售中\n价格: $%d\n购买:/buyggp\nID: %d", BillBoard[id][Price], BillBoard[id][Id]);
        } else {
            format(msg, sizeof msg, "拥有者:%s\nID:%d", BillBoard[id][Owner], BillBoard[id][Id]);
        }
    } else {
        format(msg, sizeof msg, "转卖中\n价格: $%d\n购买:/buyggp\nID: %d", BillBoard[id][Sale], BillBoard[id][Id]);
    }
    BillBoard[id][TagTId] = CreateDynamic3DTextLabel(msg, COLOR_WHITE, BillBoard[id][X], BillBoard[id][Y], BillBoard[id][Z]-1, BILLBOARD_3DTEXT_DISTANCE);
}

stock CreateBillBoard(id, dbid, objid, text[], idx, size, fface[], fsize, bold, fcolor, bcolor, alignment, owner[], price, sale, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz) {
    BillBoard[id][Id] = dbid, BillBoard[id][OBJId] = objid, BillBoard[id][Idx] = idx, BillBoard[id][Size] = size;
    BillBoard[id][FontSize] = fsize, BillBoard[id][Bold] = bold, BillBoard[id][Price] = price, BillBoard[id][FontColor] = fcolor;
    BillBoard[id][BackColor] = bcolor, BillBoard[id][Alignment] = alignment, BillBoard[id][RZ] = rz, BillBoard[id][Sale] = sale;
    BillBoard[id][X] = x, BillBoard[id][Y] = y, BillBoard[id][Z] = z, BillBoard[id][RX] = rx, BillBoard[id][RY] = ry;
    format(BillBoard[id][FontFace], MAX_PLAYER_NAME, fface);
    format(BillBoard[id][Text], MAX_BILLBOARD_TEXT, text);
    format(BillBoard[id][Owner], MAX_PLAYER_NAME, owner);
    BillBoard[id][TagOId] = CreateDynamicObject(objid, x, y, z, rx, ry, rz);
    UpdateBillBoardMaterialText(id);
    UpdateBillBoard3DText(id);
}

stock DestroyBillBoard(id) {
    BillBoard[id][Id] = 0;
    if(IsValidDynamicObject(BillBoard[id][TagOId]) == 1) {
        DestroyDynamicObject(BillBoard[id][TagOId]);
    }
    if(IsValidDynamic3DTextLabel(BillBoard[id][TagTId]) == 1) {
        DestroyDynamic3DTextLabel(BillBoard[id][TagTId]);
    }
}

stock BillBoardWode(playerid) {
    new msg[256], counts = 0;
    format(msg, sizeof msg, "我的公告牌\tID");
    for(new i = 0; i < MAX_PLAYER_BILLBOARD; i ++) {
        if(myGGPs[playerid][i] != -1) {
            counts ++;
            format(msg, sizeof msg, "%s\n我的公告牌%d\t%d", msg, counts, BillBoard[myGGPs[playerid][i]][Id]);
        }
    }
    if(counts == 0) {
        DIALOG_node(playerid, "你没有公告牌");
    } else {
        ShowPlayerDialog(playerid, DIALOG_BILLBOARD_MY, DIALOG_STYLE_TABLIST_HEADERS, "我的公告牌", msg, "选择", "关闭");
    }
}

stock BillBoardOptions(playerid) {
    new id = pBBEditId[playerid], title[88], msg[256], alstatus[3][5] = {"左", "居中", "右"};
    format(title, sizeof title, "我的公告牌%d - 操作", id+1);
    format(msg, sizeof msg, "\
    拉到身边\n\
    编辑位置\n\
    更改文字\n\
    更改分辨率:%d\n\
    更改字体\n\
    更改文字大小:%d\n\
    更改文字粗细:%s\n\
    更改文字颜色\n\
    更改背景颜色\n\
    更改文字对齐:%s\n\
    出售公告牌\
    ", BillBoard[id][Size], BillBoard[id][FontSize], (BillBoard[id][Bold] == 0 ? ("正常") : ("粗体")), alstatus[BillBoard[id][Alignment]]);
    ShowPlayerDialog(playerid, DIALOG_BILLBOARD_OPTIONS, DIALOG_STYLE_LIST, title, msg, "选择", "返回");
    return 1;
}

stock InitPlayerBillBoardVars(playerid) {
    pBBEditId[playerid] = -1;
}

forward BillBoardODR(playerid, dialogid, response, listitem, inputtext[]);
public BillBoardODR(playerid, dialogid, response, listitem, inputtext[]) {
    new id = pBBEditId[playerid];
    if(dialogid == DIALOG_BILLBOARD_MY) {
        if(!response) {
            return 1;
        }
        pBBEditId[playerid] = myGGPs[playerid][listitem];
        BillBoardOptions(playerid);
        return 1;
    }
    if(id == -1) {
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_OPTIONS) {
        if(!response) {
            InitPlayerBillBoardVars(playerid);
            BillBoardWode(playerid);
            return 1;
        }
        if(listitem == 0) {
            new Float:x, Float:y, Float:z, msg[256];
            GetPlayerPos(playerid, x, y, z);
            SetPlayerPos(playerid, x+1, y+1, z+1);
            BillBoard[id][X] = x, BillBoard[id][Y] = y, BillBoard[id][Z] = z;
			SetDynamicObjectPos(BillBoard[id][TagOId], x, y, z);
			UpdateBillBoard3DText(id);
            format(msg, sizeof msg, "UPDATE billboard SET x='%.3f', y='%.3f', z='%.3f' WHERE id='%d'", x, y, z, BillBoard[id][Id]);
            mysql_query(mariadb, msg, false);
            DIALOG_node(playerid, "你把公告牌拉到了身边");
        } else if(listitem == 1) {
			SCM(playerid, COLOR_RED, "提示:已进入公告牌编辑模式!按住空格可移动视角,如无法移动,请按'T'发送一条消息后重试!");
            EditDynamicObject(playerid, BillBoard[id][TagOId]);
        } else if(listitem == 2) {
            new title[88];
            format(title, sizeof title, "我的公告牌%d - 更改文字", id+1);
            ShowPlayerDialog(playerid, DIALOG_BILLBOARD_TEXT, DIALOG_STYLE_INPUT, title, "请输入内容(*换行, &续写)", "确定", "返回");
        } else if(listitem == 3) {
            BillBoard[id][Size] += 10;
            if(BillBoard[id][Size] > 140) {
                BillBoard[id][Size] = 10;
            }
            BillBoardOptions(playerid);
            BillBoardUpdate_Int(id, "size", BillBoard[id][Size]);
            UpdateBillBoardMaterialText(id);
        } else if(listitem == 4) {
            new title[88], msg[128];
            format(title, sizeof title, "我的公告牌%d - 更改字体", id+1);
            format(msg, sizeof msg, "当前字体:%s\n请输入字体名称", BillBoard[id][FontFace]);
            ShowPlayerDialog(playerid, DIALOG_BILLBOARD_FONTFACE, DIALOG_STYLE_INPUT, title, msg, "确定", "返回");
        } else if(listitem == 5) {
            new title[88], msg[128];
            format(title, sizeof title, "我的公告牌%d - 更改文字大小", id+1);
            format(msg, sizeof msg, "当前文字大小:%d\n请输入大小(1-255)", BillBoard[id][FontSize]);
            ShowPlayerDialog(playerid, DIALOG_BILLBOARD_FONTSIZE, DIALOG_STYLE_INPUT, title, msg, "确定", "返回");
        } else if(listitem == 6) {
            BillBoard[id][Bold] = (BillBoard[id][Bold] == 0 ? (1) : (0));
            BillBoardOptions(playerid);
            BillBoardUpdate_Int(id, "bold", BillBoard[id][Bold]);
            UpdateBillBoardMaterialText(id);
        } else if(listitem == 7) {
            BillBoardChangeColor(playerid, 0);
        } else if(listitem == 8) {
            BillBoardChangeColor(playerid, 1);
        } else if(listitem == 9) {
            BillBoard[id][Alignment] ++;
            if(BillBoard[id][Alignment] > 2) {
                BillBoard[id][Alignment] = 0;
            }
            BillBoardOptions(playerid);
            BillBoardUpdate_Int(id, "alignment", BillBoard[id][Alignment]);
            UpdateBillBoardMaterialText(id);
        } else if(listitem == 10) {
            new title[88], msg[128];
            format(title, sizeof title, "我的公告牌%d - 出售", id+1);
            format(msg, sizeof msg, "请输入出售价格(0=不出售)");
            ShowPlayerDialog(playerid, DIALOG_BILLBOARD_SALE, DIALOG_STYLE_INPUT, title, msg, "确定", "返回");
        } 
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_SALE) {
        if(!response) {
            BillBoardOptions(playerid);
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            DIALOG_node(playerid, "内容不能为空");
            InitPlayerBillBoardVars(playerid);
            return 1;
        }
        new money = strval(inputtext);
        if(money < 0 || money > 99999999) {
            DIALOG_node(playerid, "出售价格不能小于0或者大于99999999");
            InitPlayerBillBoardVars(playerid);
            return 1;
        }
        new msg[128];
        BillBoard[id][Sale] = money;
        UpdateBillBoard3DText(id);
        format(msg, sizeof msg, "公告牌出售价格设置成功");
        DIALOG_node(playerid, msg);
        BillBoardUpdate_Int(id, "sale", money);
        InitPlayerBillBoardVars(playerid);
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_TEXT) {
        if(!response) {
            BillBoardOptions(playerid);
            return 1;
        }
        if(strcmp(inputtext, defaultIT) == 0) {
            DIALOG_node(playerid, "内容不能为空");
            InitPlayerBillBoardVars(playerid);
            return 1;
        }
        if(inputtext[0] == '&') {
            strdel(inputtext, 0, 1);
            format(BillBoard[id][Text], MAX_BILLBOARD_TEXT, "%s%s", BillBoard[id][Text], inputtext);
        } else {
            format(BillBoard[id][Text], MAX_BILLBOARD_TEXT, inputtext);
        }
        DIALOG_node(playerid, "公告牌文字更改成功");
        UpdateBillBoardMaterialText(id);
        new msg[2148];
        format(msg, sizeof msg, "UPDATE billboard SET text='%s' WHERE id='%d'", BillBoard[id][Text], BillBoard[id][Id]);
        mysql_query(mariadb, msg, false);
        InitPlayerBillBoardVars(playerid);
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_FONTFACE) {
        if(response) {
            if(strcmp(inputtext, defaultIT) == 0) {
                DIALOG_node(playerid, "字体名称不能为空");
                InitPlayerBillBoardVars(playerid);
                return 1;
            }
            format(BillBoard[id][FontFace], MAX_PLAYER_NAME, inputtext);
            UpdateBillBoardMaterialText(id);
            BillBoardUpdate_String(id, "fontface", inputtext);
        }
        BillBoardOptions(playerid);
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_FONTSIZE) {
        if(response) {
            new size = strval(inputtext);
            if(size < 1 || size > 255) {
                DIALOG_node(playerid, "文字大小范围1-255");
                InitPlayerBillBoardVars(playerid);
                return 1;
            }
            BillBoard[id][FontSize] = strval(inputtext);
            UpdateBillBoardMaterialText(id);
            BillBoardUpdate_Int(id, "fontsize", size);
        }
        BillBoardOptions(playerid);
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_FCOLOR) {
        if(response) {
            BillBoard[id][FontColor] = listitem;
            UpdateBillBoardMaterialText(id);
            BillBoardUpdate_Int(id, "fontcolor", listitem);
        }
        BillBoardOptions(playerid);
        return 1;
    }
    if(dialogid == DIALOG_BILLBOARD_BCOLOR) {
        if(response) {
            BillBoard[id][BackColor] = listitem;
            UpdateBillBoardMaterialText(id);
            BillBoardUpdate_Int(id, "backcolor", listitem);
        }
        BillBoardOptions(playerid);
        return 1;
    }
    return 1;
}

stock BillBoardChangeColor(playerid, typeid) {
    new title[88], msg[512];
    for(new i = 0; i < sizeof SPDBBColors; i ++) {
        format(msg, sizeof msg, "%s\n{%s}▇▇▇", msg, SPDBBColors[i]);
    }
    format(title, sizeof title, "我的公告牌%d - 更改%s颜色", pBBEditId[playerid]+1, (typeid == 0 ? ("文字") : ("背景")));
    ShowPlayerDialog(playerid, (typeid == 0 ? (DIALOG_BILLBOARD_FCOLOR) : (DIALOG_BILLBOARD_BCOLOR)), DIALOG_STYLE_LIST, title, msg, "确定", "返回");
}

stock BillBoardUpdate_String(id, columnname[], value[]) {
    if(BillBoard[id][Id] != 0) {
        new msg[128];
        format(msg, sizeof msg, "UPDATE billboard SET %s='%s' WHERE id='%d'", columnname, value, BillBoard[id][Id]);
        mysql_query(mariadb, msg, false);
    }
}

stock BillBoardUpdate_Int(id, columnname[], value) {
    if(BillBoard[id][Id] != 0) {
        new msg[128];
        format(msg, sizeof msg, "UPDATE billboard SET %s='%d' WHERE id='%d'", columnname, value, BillBoard[id][Id]);
        mysql_query(mariadb, msg, false);
    }
}

stock BillBoardOPEDO(playerid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz) {
    if(pBBEditId[playerid] != -1) {
        if(response == EDIT_RESPONSE_CANCEL) {
            pBBEditId[playerid] = -1;
        } else if(response == EDIT_RESPONSE_FINAL) {
            new id = pBBEditId[playerid];
            pBBEditId[playerid] = -1;
            if(BillBoard[id][Id] != 0 && strcmp(BillBoard[id][Owner], GetName(playerid)) == 0) {
                BillBoard[id][X] = x, BillBoard[id][Y] = y, BillBoard[id][Z] = z;
                BillBoard[id][RX] = rx, BillBoard[id][RY] = ry, BillBoard[id][RZ] = rz;
                new msg[256];
                format(msg, sizeof msg, "UPDATE billboard SET x='%.3f', y='%.3f', z='%.3f', rx='%.3f', ry='%.3f', rz='%.3f' WHERE id='%d'",
                x, y, z, rx, ry, rz, BillBoard[id][Id]);
				mysql_query(mariadb, msg, false);
                UpdateBillBoard3DText(id);
                SetDynamicObjectPos(BillBoard[id][TagOId], x, y, z);
                SetDynamicObjectRot(BillBoard[id][TagOId], rx, ry, rz);
            } else {
                DIALOG_node(playerid, "这不是你的公告牌");
            }
            
        }
    }
}