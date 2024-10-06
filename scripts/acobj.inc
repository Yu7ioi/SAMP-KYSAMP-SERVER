#define MAX_ACOBJECTS                   (15)
#define BUY_ACOBJ_POS_X                 (-1987.258) // 购买爱车obj坐标x
#define BUY_ACOBJ_POS_Y                 (304.701) // 购买爱车obj坐标y
#define BUY_ACOBJ_POS_Z                 (35.215) // 购买爱车obj坐标z35.215
#define BUY_ACOBJ_POS_R                 (10.0) // 购买爱车obj坐标范围
#define MAX_ACOBJ_NAME_LEN              (44)
#define DIALOG_ACOBJ_VIEW               DIALOG_ACOBJ + 1
#define DIALOG_ACOBJ_LIST               DIALOG_ACOBJ + 2
#define DIALOG_ACOBJ_LIST_OPTIONS       DIALOG_ACOBJ + 3
#define DIALOG_ACOBJ_SELL_CONFIRM		DIALOG_ACOBJ + 4

stock ACOBJ3DTEXT() {
    CreateDynamic3DTextLabel("在此输入/ac buyobj\n购买爱车装扮", COLOR_WHITE, BUY_ACOBJ_POS_X, BUY_ACOBJ_POS_Y, BUY_ACOBJ_POS_Z, 25.0);
}

enum acobjtypes {
    Id,
    ACId,
    Name[MAX_ACOBJ_NAME_LEN],
    ModelId,
    Float:X,
    Float:Y,
    Float:Z,
    Float:RX,
    Float:RY,
    Float:RZ,
    Show,
    TagId
};

enum acobjviewtype {
    Id,
    TagId
};

new ACOBJ[MAX_VEHICLES][MAX_ACOBJECTS][acobjtypes], pACOBJView[MAX_PLAYERS][acobjviewtype], pACOBJEditVID[MAX_PLAYERS], pACOBJEditId[MAX_PLAYERS];

enum acobjbuytype {
    Name[MAX_ACOBJ_NAME_LEN],
    ModelId,
    Money,
    OTime
};

new ACOBJBuy[][acobjbuytype] = {
    /*{"装扮名称", 模型ID, 购买价格, 需要时间分},*/
    {"霓虹灯(蓝色)", 18648, 80000, 3000},
	{"霓虹灯(白色)", 18652, 80000, 3000},
	{"霓虹灯(红色)", 18647, 80000, 3000},
	{"霓虹灯(绿色)", 18649, 80000, 3000},
	{"霓虹灯(紫色)", 18651, 80000, 3000},
	{"新警灯", 19620, 110000, 3500},
	{"老警灯", 19419, 110000, 3500},
	{"小警灯", 18646, 110000, 3500},
	{"尾翼1", 1146, 1100000, 4000},
	{"尾翼2", 1001, 1100000, 4000},
	{"尾翼3", 1002, 1010000, 4000},
	{"尾翼4", 1023, 1010000, 4000},
	{"尾翼5", 1000, 1010000, 4000},
	{"尾翼6", 1147, 1010000, 4000},
	{"小斜坡", 13593, 150000, 5000},
	{"鲨鱼", 1608, 8100000, 4000},
	{"乌龟", 1609, 200000, 4000},
	{"鹦鹉", 19079, 90000, 2000},
	{"海豚", 1607, 400000, 3000},
	{"TAXI1", 19308, 300000, 6000},
	{"TAXI2", 19311, 300000, 6000},
	{"鹿", 19315, 180000, 4000},
	{"防空炮", 3267, 3100000, 4200},
	{"导弹", 3790, 990000, 4000},
	{"爆闪灯", 19150, 1660000, 4000},
	{"日光灯[小]", 19279, 1500000, 4000},
	{"日光灯[大]", 19280, 1500000, 4000},
	{"复古大炮", 2064, 1010000, 4000},
	{"喷火", 18694, 1900000, 4000},
	{"加特林", 2985, 1900000, 4000},
	{"火焰[中]", 18694, 2900000, 8000},
	{"火龙头", 3528, 9800000, 10000},
	{"水花", 18740, 800000, 5000}
};

stock ACOBJRuntime(playerid) {
    if(pACOBJView[playerid][Id] != -1) {
        if(GetPlayerDistanceFromPoint(playerid, BUY_ACOBJ_POS_X, BUY_ACOBJ_POS_Y, BUY_ACOBJ_POS_Z) >= BUY_ACOBJ_POS_R) {
            ACOBJViewInit(playerid);
            DisableRemoteVehicleCollisions(playerid, 0);
            //SCM(playerid, COLOR_RED, "[爱车系统] 你离开了爱车装扮预览范围.");
        }
    }
}

stock ACOBJViewInit(playerid) {
    pACOBJEditId[playerid] = -1;
    pACOBJView[playerid][Id] = -1;
    if(IsValidDynamicObject(pACOBJView[playerid][TagId]) == 1) {
        DestroyDynamicObject(pACOBJView[playerid][TagId]);
    }
}

stock ACOBJBuyCMD(playerid) {
    if(GetPlayerDistanceFromPoint(playerid, BUY_ACOBJ_POS_X, BUY_ACOBJ_POS_Y, BUY_ACOBJ_POS_Z) >= BUY_ACOBJ_POS_R) {
        SCM(playerid, COLOR_ORANGE, "[爱车系统] 你不在爱车装扮购买点.请输入{FF00FF} /MCD{FEA979}传送至买车点");
        return 1;
    }
    if(pACOBJView[playerid][Id] == -1) {
        DisableRemoteVehicleCollisions(playerid, 1);
        new msg[2048];
        format(msg, sizeof msg, "装扮名称\t购买价格\t需在线时间\n");
        for(new i = 0; i < sizeof ACOBJBuy; i ++) {
            format(msg, sizeof msg, "%s%s\t$%d\t%d\n", msg, ACOBJBuy[i][Name], ACOBJBuy[i][Money], ACOBJBuy[i][OTime]);
        }
        ShowPlayerDialog(playerid, DIALOG_ACOBJ_VIEW, DIALOG_STYLE_TABLIST_HEADERS, "购买爱车装扮", msg, "预览", "关闭");
    } else {
        new vid = GetPlayerVehicleID(playerid);
        if(IsVIDEqualMyAC(playerid, vid) == -1) {
            DIALOG_node(playerid, "你不在爱车内");
            return 1;
        }
        if(GetPlayerVehicleSeat(playerid) != 0) {
            DIALOG_node(playerid, "你不在爱车驾驶座");
            return 1;
        }
        if(ACOBJNullId(vid) == -1) {
            DIALOG_node(playerid, "该爱车装扮已达至上限, 不能拥有更多装扮了.");
            return 1;
        }
        new id = pACOBJView[playerid][Id];
        ACOBJViewInit(playerid);
        if(userInfo[playerid][OnlineTime] < ACOBJBuy[id][OTime]) {
            DIALOG_node(playerid, "你的在线时间不够");
            return 1;
        }
        if(GetMoney(playerid) < ACOBJBuy[id][Money]) {
            DIALOG_node(playerid, "你的钱不够");
            return 1;
        }
        GiveMoney(playerid, -ACOBJBuy[id][Money]);
        accountSaveData(playerid);
        new msg[128];
        format(msg, sizeof msg, "购买爱车装扮, 名称:%s", ACOBJBuy[id][Name]);
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "acobj", "acobj", 0, -ACOBJBuy[id][Money], msg);
        ACOBJCreate(vid, ACOBJBuy[id][Name], ACOBJBuy[id][ModelId]);
        DIALOG_node(playerid, "购买成功");
    }
    return 1;
}

stock ACOBJNullId(vid) {
    if(ACVeh[vid][Id] != 0) {
        for(new slot = 0 ; slot < MAX_ACOBJECTS; slot ++) {
            if(ACOBJ[vid][slot][Id] == 0) {
                return slot;
            }
        }
    }
    return -1;
}

stock ACOBJCreate(vid, name[], modelid) {
    new query[512], Cache:result, dbid;
    format(query, sizeof query, "INSERT INTO acobj (acid,name,modelid,x,y,z,rx,ry,rz,`show`,created_at) VALUES ('%d','%s','%d','0.0','0.0','0.0','0.0','0.0','0.0','0','%d');", 
    ACVeh[vid][Id], name, modelid, gettime());
    result = mysql_query(mariadb, query);
    dbid = cache_insert_id();
    cache_delete(result);
    new slot = ACOBJNullId(vid);
    ACOBJ[vid][slot][Id] = dbid;
    ACOBJ[vid][slot][ACId] = ACVeh[vid][Id];
    format(ACOBJ[vid][slot][Name], MAX_ACOBJ_NAME_LEN, name);
    ACOBJ[vid][slot][ModelId] = modelid;
    ACOBJ[vid][slot][X] = 0.0;
    ACOBJ[vid][slot][Y] = 0.0;
    ACOBJ[vid][slot][Z] = 0.0;
    ACOBJ[vid][slot][RX] = 0.0;
    ACOBJ[vid][slot][RY] = 0.0;
    ACOBJ[vid][slot][RZ] = 0.0;
    ACOBJ[vid][slot][Show] = 0;
}

stock ACOBJViewing(playerid, id) {
    new vid = GetPlayerVehicleID(playerid);
    if(IsVIDEqualMyAC(playerid, vid) == -1) {
        DIALOG_node(playerid, "你不在爱车内");
    } else if(GetPlayerVehicleSeat(playerid) != 0) {
        DIALOG_node(playerid, "你不在爱车驾驶座");
    } else {
        if(id < 0) {
            id = sizeof ACOBJBuy - 1;
        } else if(id >= sizeof ACOBJBuy) {
            id = 0;
        }
        pACOBJView[playerid][Id] = id;
        if(IsValidDynamicObject(pACOBJView[playerid][TagId]) == 1) {
            DestroyDynamicObject(pACOBJView[playerid][TagId]);
        }
        pACOBJView[playerid][TagId] = CreateDynamicObject(ACOBJBuy[id][ModelId], 0.0, 0.0, -100.0, 0.0, 0.0, 0.0);
        AttachDynamicObjectToVehicle(pACOBJView[playerid][TagId], vid, 0.0, 0.0, 0.75, 0.0, 0.0, 0.0);
        Streamer_Update(playerid);
        new msg[144];
        format(msg, sizeof msg, "[爱车系统] 你正在预览爱车装扮'{FFFF00}%s{FEA979}', 价格'${00FF00}%d{FEA979}', 所需时间分'{FFFF00}%d{FEA979}'", ACOBJBuy[id][Name], ACOBJBuy[id][Money], ACOBJBuy[id][OTime]);
        SCM(playerid, COLOR_ORANGE, msg);
    }
}

stock ACOBJOPKSC(playerid, newkeys) {
    if(newkeys == KEY_LOOK_LEFT && pACOBJView[playerid][Id] != -1) {
        ACOBJViewing(playerid, pACOBJView[playerid][Id]-1);
        return 1;
    } else if(newkeys == KEY_LOOK_RIGHT && pACOBJView[playerid][Id] != -1) {
        ACOBJViewing(playerid, pACOBJView[playerid][Id]+1);
        return 1;
    }
    return 0;
}

stock ACOBJList(playerid) {
    new vid = GetPlayerVehicleID(playerid);
    if(IsVIDEqualMyAC(playerid, vid) == -1) {
        DIALOG_node(playerid, "你不在爱车内");
        return 1;
    }
    if(GetPlayerVehicleSeat(playerid) != 0) {
        DIALOG_node(playerid, "你不在爱车驾驶座");
        return 1;
    }
    new msg[2048], has = 0;
    format(msg, sizeof msg, "ID\t名称\t状态\n");
    for(new slot = 0; slot < MAX_ACOBJECTS; slot ++) {
        if(ACOBJ[vid][slot][ModelId] != 0) {
            format(msg, sizeof msg, "%s%d\t%s\t%s\n", msg, slot, ACOBJ[vid][slot][Name], (ACOBJ[vid][slot][Show] == 0 ? ("未装扮") : ("装扮中")));
            has ++;
        }
    }
    if(has == 0) {
        DIALOG_node(playerid, "你的这辆爱车没有装扮");
    } else {
        ShowPlayerDialog(playerid, DIALOG_ACOBJ_LIST, DIALOG_STYLE_TABLIST_HEADERS, "我的爱车装扮列表", msg, "选择", "关闭");
    }
    return 1;
}

stock ACOBJUpdate(vid, slot) {
    new msg[256];
    format(msg, sizeof msg, "UPDATE acobj SET acid='%d',name='%s',modelid='%d',x='%.3f',y='%.3f',z='%.3f',rx='%.3f',ry='%.3f',rz='%.3f',`show`='%d' WHERE id='%d'",
    ACOBJ[vid][slot][ACId], ACOBJ[vid][slot][Name], ACOBJ[vid][slot][ModelId], ACOBJ[vid][slot][X], ACOBJ[vid][slot][Y], ACOBJ[vid][slot][Z], ACOBJ[vid][slot][RX], 
    ACOBJ[vid][slot][RY], ACOBJ[vid][slot][RZ], ACOBJ[vid][slot][Show], ACOBJ[vid][slot][Id]);
    mysql_query(mariadb, msg, false);
}

stock ACOBJOPEDO(playerid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz) {
    if(pACOBJEditId[playerid] != -1) {
        if(response == EDIT_RESPONSE_CANCEL) {
            new vid = pACOBJEditVID[playerid];
            if(IsVIDEqualMyAC(playerid, vid) != -1) {
				if(IsValidDynamicObject(ACOBJ[vid][pACOBJEditId[playerid]][TagId]) == 1) {
					DestroyDynamicObject(ACOBJ[vid][pACOBJEditId[playerid]][TagId]);
				}
                ACOBJAttach(vid, pACOBJEditId[playerid]);
            }
            pACOBJEditId[playerid] = -1;
        } else if(response == EDIT_RESPONSE_FINAL) {
            new vid = pACOBJEditVID[playerid];
            if(IsVIDEqualMyAC(playerid, vid) != -1) {
                new Float:vx, Float:vy, Float:vz, slot = pACOBJEditId[playerid];
				if(IsValidDynamicObject(ACOBJ[vid][slot][TagId]) == 1) {
					DestroyDynamicObject(ACOBJ[vid][slot][TagId]);
				}
                if(IsPlayerInRangeOfPoint(playerid, 20.0, x, y, z) == 1) {
                    GetVehiclePos(vid, vx, vy, vz);
                    x -= vx, y -= vy, z -= vz;
                    ACOBJ[vid][slot][X] = x, ACOBJ[vid][slot][Y] = y, ACOBJ[vid][slot][Z] = z;
                    ACOBJ[vid][slot][RX] = rx, ACOBJ[vid][slot][RY] = ry, ACOBJ[vid][slot][RZ] = rz;
                    ACOBJUpdate(vid, slot);
                    ACOBJAttach(vid, slot);
                    PutPlayerInVehicle(playerid, vid, 0);
                } else {
                    DIALOG_node(playerid, "装扮距离载具太远");
                }
            }
            pACOBJEditId[playerid] = -1;
        }
    }
}

forward ACOBJDelayEditing(playerid);
public ACOBJDelayEditing(playerid) {
    EditDynamicObject(playerid, ACOBJ[pACOBJEditVID[playerid]][pACOBJEditId[playerid]][TagId]);
    return 1;
}

forward ACOBJODR(playerid, dialogid, response, listitem, inputtext[]);
public ACOBJODR(playerid, dialogid, response, listitem, inputtext[]) {
    new vid = GetPlayerVehicleID(playerid);
    if(IsVIDEqualMyAC(playerid, vid) == -1) {
        DIALOG_node(playerid, "你不在爱车内");
        return 1;
    }
    if(GetPlayerVehicleSeat(playerid) != 0) {
        DIALOG_node(playerid, "你不在爱车驾驶座");
        return 1;
    }
    if(dialogid == DIALOG_ACOBJ_LIST_OPTIONS) {
        if(!response) {
            pACOBJEditId[playerid] = -1;
            return 1;
        }
        new slot = pACOBJEditId[playerid];
        if(listitem == 0) {
            if(ACOBJ[vid][slot][Show] == 0) {
                DIALOG_node(playerid, "该先装扮该装扮.....");
                return 1;
            }
            pACOBJEditVID[playerid] = vid;
            if(IsValidDynamicObject(ACOBJ[vid][slot][TagId]) == 1) {
                DestroyDynamicObject(ACOBJ[vid][slot][TagId]);
            }
            new Float:x, Float:y, Float:z;
            GetVehiclePos(vid, x, y, z);
            SetVehicleZAngle(vid, 0.0);
            SetPlayerPos(playerid, x+2, y+2, z+2);
            ACOBJ[vid][slot][TagId] = CreateDynamicObject(ACOBJ[vid][slot][ModelId], x+ACOBJ[vid][slot][X], y+ACOBJ[vid][slot][Y], z+ACOBJ[vid][slot][Z],
            ACOBJ[vid][slot][RX], ACOBJ[vid][slot][RY], ACOBJ[vid][slot][RZ]);
            SCM(playerid, COLOR_RED, "提示:已进入爱车装扮编辑模式!按住空格可移动视角,如无法移动,请按'T'发送一条消息后重试!");
            SetTimerEx("ACOBJDelayEditing", 1000, false, "i", playerid);
        } else if(listitem == 1) {
			ACOBJ[vid][slot][Show] = (strcmp(inputtext, "装扮") == 0 ? (1) : (0));
            ACOBJUpdate(vid, slot);
            ACOBJAttach(vid, slot);
			Streamer_Update(playerid);
        } else if(listitem == 2) {
			new msg[128], idx = -1;
			for(new i = 0; i < sizeof ACOBJBuy; i ++) {
				if(ACOBJBuy[i][ModelId] == ACOBJ[vid][slot][ModelId]) {
					idx = i;
					i = sizeof ACOBJBuy + 1;
				}
			}
			if(idx == -1) {
				DIALOG_node(playerid, "这个不能卖");
			} else {
				format(msg, sizeof msg, "你确定要已价格$%d出售掉这个装扮给系统吗?", ACOBJBuy[idx][Money]/2);
				ShowPlayerDialog(playerid, DIALOG_ACOBJ_SELL_CONFIRM, DIALOG_STYLE_MSGBOX, "装扮出售", msg, "确定", "关闭");
			}
		}
        return 1;
    }
	if(dialogid == DIALOG_ACOBJ_SELL_CONFIRM) {
		new slot = pACOBJEditId[playerid];
		pACOBJEditId[playerid] = -1;
		if(!response) return 1;
		new idx = -1;
		for(new i = 0; i < sizeof ACOBJBuy; i ++) {
			if(ACOBJBuy[i][ModelId] == ACOBJ[vid][slot][ModelId]) {
				idx = i;
				i = sizeof ACOBJBuy + 1;
			}
		}
		new msg[128];
		format(msg, sizeof msg, "出售装扮ID:%d", ACOBJ[vid][slot][Id]);
		GiveMoney(playerid, ACOBJBuy[idx][Money]/2);
		log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "acobj", "在线记录", 0, ACOBJBuy[idx][Money]/2, msg);
		ACOBJDelete(vid, slot);
		DIALOG_node(playerid, "出售成功");
		return 1;
	}
    if(dialogid == DIALOG_ACOBJ_LIST) {
        if(!response) {
            return 1;
        }
        pACOBJEditId[playerid] = strval(inputtext);
        new msg[128];
        format(msg, sizeof msg, "编辑\n%s\n出售", (ACOBJ[vid][pACOBJEditId[playerid]][Show] == 0 ? ("装扮") : ("取下")));
        ShowPlayerDialog(playerid, DIALOG_ACOBJ_LIST_OPTIONS, DIALOG_STYLE_LIST, "我的爱车装扮选项", msg, "确定", "关闭");
        return 1;
    }
    if(dialogid == DIALOG_ACOBJ_VIEW) {
        if(!response) {
            return 1;
        }
        ACOBJViewing(playerid, listitem);
        SCM(playerid, COLOR_ORANGE, "[爱车系统] 按'Q'可以预览上一个, 按'E'可以预览下一个, 再次输入'{FF00FF}/ac buyobj{FEA979}'可购买");
        return 1;
    }
    return 1;
}

stock ACOBJAttachInit(vid) {
    if(ACVeh[vid][Id] != 0) {
        for(new i = 0; i < MAX_ACOBJECTS; i ++) {
            if(ACOBJ[vid][i][ModelId] != 0) {
                if(IsValidDynamicObject(ACOBJ[vid][i][TagId]) == 1) {
                    DestroyDynamicObject(ACOBJ[vid][i][TagId]);
                }
            }
        }
    }
}

stock ACOBJAttach(vid, slot = -1) {
	if(slot == -1) {
		for(new i = 0; i < MAX_ACOBJECTS; i ++) {
			if(ACOBJ[vid][i][ModelId] != 0) {
				if(IsValidDynamicObject(ACOBJ[vid][i][TagId]) == 1) {
					DestroyDynamicObject(ACOBJ[vid][i][TagId]);
				}
				if(ACOBJ[vid][i][Show] == 1) {
					ACOBJ[vid][i][TagId] = CreateDynamicObject(ACOBJ[vid][i][ModelId], 0.0, 0.0, -100.0, 0.0, 0.0, 0.0);
					AttachDynamicObjectToVehicle(ACOBJ[vid][i][TagId], vid, ACOBJ[vid][i][X], ACOBJ[vid][i][Y], ACOBJ[vid][i][Z], ACOBJ[vid][i][RX], ACOBJ[vid][i][RY], ACOBJ[vid][i][RZ]);
				}
			}
		}
	} else {
		if(ACOBJ[vid][slot][ModelId] != 0) {
			if(IsValidDynamicObject(ACOBJ[vid][slot][TagId]) == 1) {
				DestroyDynamicObject(ACOBJ[vid][slot][TagId]);
			}
			if(ACOBJ[vid][slot][Show] == 1) {
				ACOBJ[vid][slot][TagId] = CreateDynamicObject(ACOBJ[vid][slot][ModelId], 0.0, 0.0, -100.0, 0.0, 0.0, 0.0);
				AttachDynamicObjectToVehicle(ACOBJ[vid][slot][TagId], vid, ACOBJ[vid][slot][X], ACOBJ[vid][slot][Y], ACOBJ[vid][slot][Z], ACOBJ[vid][slot][RX], ACOBJ[vid][slot][RY], ACOBJ[vid][slot][RZ]);
			}
		}
	}
}

stock ACOBJDelete(vid, slot) {
	new msg[128];
	format(msg, sizeof msg, "UPDATE acobj SET deleted_at='%d' WHERE id='%d'", gettime(), ACOBJ[vid][slot][Id]);
	mysql_query(mariadb, msg, false);
	ACOBJ[vid][slot][Id] = 0;
	ACOBJ[vid][slot][ModelId] = 0;
	if(IsValidDynamicObject(ACOBJ[vid][slot][TagId]) == 1) {
		DestroyDynamicObject(ACOBJ[vid][slot][TagId]);
	}
}

stock ACOBJInit(vid) {
    new query[128];
    mysql_format(mariadb, query, sizeof(query), "SELECT * FROM acobj WHERE acid='%d'", ACVeh[vid][Id]);
	mysql_tquery(mariadb, query, "ACOBJInit2", "d", vid);
}

forward ACOBJInit2(vid);
public ACOBJInit2(vid) {
	new rows = cache_num_rows();
    if(rows > 0) {
        for(new i = 0; i < rows; i ++) {
			new deleted_at;
			cache_get_value_name_int(i, "deleted_at", deleted_at);
			if(deleted_at == 0) {
                new slot = ACOBJEmptyId(vid);
				cache_get_value_name_int(i, "id", ACOBJ[vid][slot][Id]);
				cache_get_value_name_int(i, "acid", ACOBJ[vid][slot][ACId]);
				cache_get_value_name(i, "name", ACOBJ[vid][slot][Name], MAX_ACOBJ_NAME_LEN);
				cache_get_value_name_int(i, "modelid", ACOBJ[vid][slot][ModelId]);
				cache_get_value_name_float(i, "x", ACOBJ[vid][slot][X]);
				cache_get_value_name_float(i, "y", ACOBJ[vid][slot][Y]);
				cache_get_value_name_float(i, "z", ACOBJ[vid][slot][Z]);
				cache_get_value_name_float(i, "rx", ACOBJ[vid][slot][RX]);
				cache_get_value_name_float(i, "ry", ACOBJ[vid][slot][RY]);
				cache_get_value_name_float(i, "rz", ACOBJ[vid][slot][RZ]);
				cache_get_value_name_int(i, "show", ACOBJ[vid][slot][Show]);
			}
        }
        ACOBJAttach(vid);
    }
	return 1;
}

stock ACOBJEmptyId(vid) {
    for(new i = 0; i < MAX_ACOBJECTS; i ++) {
        if(ACOBJ[vid][i][Id] == 0) {
            return i;
        }
    }
    return 1;
}