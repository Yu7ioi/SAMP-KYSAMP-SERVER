
#define PRESSED(%0) \
	(((newkeys & (%0)) == (%0)) && ((oldkeys & (%0)) != (%0)))

#if !defined IsValidVehicle
    native IsValidVehicle(vehicleid);
#endif

#define PI 3.14159265358979323846
#define CHECK_DISTANCE 5.0 // 检测前方距离

#define TACKLE_TIME (10) // 飞扑冷却10秒

#define CHASE_INFO "{19E0E2}[战局] {CDD6CB}" // 战局信息前缀

#define CHASES_MAX_PLAYERS (4) // 最大参与玩家数
#define CHASES_MAX_POLICES (3) // 警察最大参与玩家数
#define CHASES_MAX_DEALERS (1) // 毒贩最大参与玩家数
#define CHASES_MAX_MINUTES (15) // 战局最大分钟数
#define CHASES_MAX_SECONDS (0) // 战局最大秒数
#define CHASES_BASE_POLICES_MONEY (CHASES_MAX_DEALERS * 2000) // 战局警察基本赏金
#define CHASES_BASE_DEALERS_MONEY (CHASES_MAX_POLICES * 2500) // 战局毒贩基本赃款

/* 战局状态 */
#define CHASE_STATUS_FREEING (0) // 空闲中
#define CHASE_STATUS_GATHERING (1) // 集结中
#define CHASE_STATUS_RUNNING (2) // 进行中
#define CHASE_STATUS_ENDING (3) // 结束

/* 战局状态宏 */
#define SetChaseStatus(%0)  (ChaseStatus = %0) // 设置战局状态存储方式 => 0:空闲中 1:集结中 2:进行中 3:结束
#define GetChaseStatus()    (ChaseStatus) // 获取战局状态

#define SetPlayerInChaseDeath(%0,%1) (isPlayerDeath{%0} = %1) // 设置战局玩家死亡状态
#define GetPlayerInChaseDeath(%0) (isPlayerDeath{%0}) // 获取战局玩家死亡状态


/* 战局占用对话框ID */
#define DIALOG_CHASE_CREATE (9988) // 创建战局对话框ID
#define DIALOG_CHASE_SELECT (9989) // 选择扮演角色对话框ID

/* 战局临时数据构造 */
enum ChaseDataStruct
{
    PoliceVehicle[3], // 警察车辆
    DealerVehicle[3], // 毒贩车辆

    PlayerText:td_PoliceMoney[MAX_PLAYERS], // 警察赏金
    PlayerText:td_DealerMoney[MAX_PLAYERS], // 毒贩赏金

    PoliceTackleTimer[MAX_PLAYERS], // 警察飞扑计时器
    PoliceSendTackle[MAX_PLAYERS], // 警察发动飞扑

    PoliceSpectateID[MAX_PLAYERS],
    DealerSpectateID[MAX_PLAYERS],
    
    PoliceCountKills, // 警察击杀数
    DealerCountKills, // 毒贩击杀数
    PoliceCountDeath, // 警察死亡数
    DealerCountDeath, // 毒贩死亡数

    PoliceBounty, // 警察赏金
    DealerBounty // 毒贩赏金
}

new 
gString[256], // 全局数组
ChaseStatus, // 战局状态
CheckChaseGatherTimer, // 检查战局状态定时器
ChaseRunTimer, // 战局运行计时器
ChaseEndTimer, // 战局结束计时器
Text:td_ChaseInfo[2], // 战局时间
ceVar[ChaseDataStruct], // 战局临时数据

isPlayerDeath[MAX_PLAYERS char]; // 玩家死亡状态


static 
Chase_Minutes = CHASES_MAX_MINUTES,
Chase_Seconds = CHASES_MAX_SECONDS;

forward OnCheckChaseGatherStatus();
forward OnChaseRun();
forward OnChaseEnd();
forward OnPoliceTackle(playerid);


new 
List_PlayPlayer[CHASES_MAX_PLAYERS+1],
List_PlayPolice[CHASES_MAX_POLICES+1],
List_PlayDealer[CHASES_MAX_DEALERS+1];


#define List_GetSize(%0) %0[0]

#define PlayerID List_PlayPlayer[i]
#define List_Add_Player(%0) List_Add(List_PlayPlayer, CHASES_MAX_PLAYERS, %0)
#define List_Remove_Player(%0) List_Remove(List_PlayPlayer, CHASES_MAX_PLAYERS, %0)
#define foreachPlayer() for(new i = 1; i <= List_GetSize(List_PlayPlayer); ++i)
#define IsPlayerInChasePlayer(%0) (List_FindValue(List_PlayPlayer, %0) != -1) // 玩家是否是战局的玩家

#define PoliceID List_PlayPolice[i]
#define List_Add_Police(%0) List_Add(List_PlayPolice, CHASES_MAX_POLICES, %0)
#define List_Remove_Police(%0) List_Remove(List_PlayPolice, CHASES_MAX_POLICES, %0)
#define foreachPolice() for(new i = 1; i <= List_GetSize(List_PlayPolice); ++i)
#define IsPlayerPolice(%0) (List_FindValue(List_PlayPolice, %0) != -1) // 玩家是否是警察

#define DealerID List_PlayDealer[i]
#define List_Add_Dealer(%0) List_Add(List_PlayDealer, CHASES_MAX_DEALERS, %0)
#define List_Remove_Dealer(%0) List_Remove(List_PlayDealer, CHASES_MAX_DEALERS, %0)
#define foreachDealer() for(new i = 1; i <= List_GetSize(List_PlayDealer); ++i)
#define IsPlayerDealer(%0) (List_FindValue(List_PlayDealer, %0) != -1) // 玩家是否是毒贩

stock BinarySearch(const array[], count, value, &index) 
{
    new low = 0, high = count - 1;
    index = -1;
    while (low <= high) 
    {
        new mid = (low + high) >> 1;
        if (array[mid] < value) 
        {
            low = mid + 1;
        }else if (array[mid] > value){
            high = mid - 1;
        }else{
            index = mid;
            return 1;
        }
    }
    index = low;
    return 0;
}

stock List_Add(array[], array_size, value) 
{
    new count = array[0];
    if (count < array_size) 
    {
        new index;
        if (!BinarySearch(array[1], count, value, index))
        {
            for (new i = count; i > index; --i) 
            {
                array[i + 1] = array[i];
            }
            array[index + 1] = value;
            array[0]++;
        }
    }
}

stock List_Remove(array[], array_size, value) 
{
    new count = array[0];
    new index;
    if (BinarySearch(array[1], count, value, index))
    {
        for (new i = index + 1; i < count; ++i)
        {
            if (i < array_size) 
            {
                array[i] = array[i + 1];
            }
        }
        array[0]--;
        array[count] = 0;
    }
}

stock List_FindValue(const array[], value) 
{
    new index;
    if (BinarySearch(array[1], List_GetSize(array), value, index))
    {
        return value;
    }else{
        return -1;
    }
}

/* 切换到下一个观战 */
stock SwitchSpectatingNext(playerid, isPolice)
{
    new nextPlayerID = INVALID_PLAYER_ID;
    new currentViewing = (isPolice ? ceVar[PoliceSpectateID][playerid] : ceVar[DealerSpectateID][playerid]);
    new foundCurrent;

    if (isPolice)
    {
        foreachPolice()
        {
            if (!GetPlayerInChaseDeath(PoliceID) && PoliceID != playerid)
            {
                if (foundCurrent)
                {
                    nextPlayerID = PoliceID;
                    break;
                }
                if (currentViewing == PoliceID)
                {
                    foundCurrent = 1;
                }
            }
        }
        if (nextPlayerID == INVALID_PLAYER_ID)
        {
            foreachPolice()
            {
                if (!GetPlayerInChaseDeath(PoliceID) && PoliceID != playerid)
                {
                    nextPlayerID = PoliceID;
                    break;
                }
            }
        }
    }else{
        foreachDealer()
        {
            if (!GetPlayerInChaseDeath(DealerID) && DealerID != playerid)
            {
                if (foundCurrent)
                {
                    nextPlayerID = DealerID;
                    break;
                }
                if (currentViewing == DealerID)
                {
                    foundCurrent = 1;
                }
            }
        }
        if (nextPlayerID == INVALID_PLAYER_ID)
        {
            foreachDealer()
            {
                if (!GetPlayerInChaseDeath(DealerID) && DealerID != playerid)
                {
                    nextPlayerID = DealerID;
                    break;
                }
            }
        }
    }

    if (nextPlayerID != INVALID_PLAYER_ID && nextPlayerID != currentViewing)
    {
        PlayerSpectatePlayer(playerid, nextPlayerID);
        if (isPolice) {
            ceVar[PoliceSpectateID][playerid] = nextPlayerID;
        } else {
            ceVar[DealerSpectateID][playerid] = nextPlayerID;
        }
        format(gString, sizeof gString, CHASE_INFO"现在你正在观战 %s[%i]。", Chase_GetName(nextPlayerID), nextPlayerID);
        SendClientMessage(playerid, -1, gString);
    }
    else if (nextPlayerID == currentViewing || nextPlayerID == INVALID_PLAYER_ID)
    {
        SendClientMessage(playerid, -1, CHASE_INFO"没有其他存活的队友可以切换观战。");
    }
    return 1;
}

public OnPoliceTackle(playerid)
{
    new 
    Float:x, 
    Float:y, 
    Float:z, 
    Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    // 计算前方检查点的位置
    new Float:checkX = x + (CHECK_DISTANCE * floatcos(angle * PI / 180.0));
    new Float:checkY = y + (CHECK_DISTANCE * floatsin(angle * PI / 180.0));

    // 检测前方是否有玩家
    foreachDealer()
    {
        new 
        Float:targetX,
        Float:targetY,
        Float:targetZ;

        GetPlayerPos(DealerID, targetX, targetY, targetZ);
        if (IsPointNearPoint(checkX, checkY, targetX, targetY, CHECK_DISTANCE)) 
        {
            ApplyAnimation(DealerID, "PARACHUTE", "FALL_skyDive_DIE", 4.0, 0, 1, 1, 1, -1);
            ApplyAnimation(i, "PED", "KO_SKID_BACK", 4.1, 0, 1, 1, 1, 0, 1);

            format(gString, sizeof gString, CHASE_INFO"毒贩 %s[%i] 被{1584DA} %s[%i]警察{FE0000}逮捕了！", Chase_GetName(DealerID), DealerID, Chase_GetName(playerid), playerid);
            SendClientMessageToAll(-1, gString);
            gString[0] = EOS; // 释放全局数组单元格
            SetPlayerInChaseDeath(DealerID, 1);
            break;
        }else{
            SendClientMessage(playerid, -1, CHASE_INFO"飞扑抓捕失败，没有抓到毒贩。");
            break;
        }
    }
    return 1;
}

/* 检查战局集结状态回调 */
public OnCheckChaseGatherStatus()
{
    if (GetChaseStatus() == CHASE_STATUS_GATHERING && List_GetSize(List_PlayPlayer) >= CHASES_MAX_PLAYERS)
    {
        SetChaseStatus(CHASE_STATUS_RUNNING); // 设置战局状态为进行中
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {50BD10}集结完成, 开始战局！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        SendClientMessageToAll(-1, gString);
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        InitChasePlayerData();
        ChaseRunTimer = SetTimer("OnChaseRun", 1000, 1); // 战局进行计时器
        KillTimer(CheckChaseGatherTimer); // 释放战局集结检查计时器
        return 1;
    }
    return 1;
}

/* 战局进行回调 */
public OnChaseRun()
{
    // 判断战局当中的毒贩是否全部死亡
    if (ceVar[DealerCountDeath] == CHASES_MAX_DEALERS)
    {
        Chase_HideInfoUI();
        SetChaseStatus(CHASE_STATUS_ENDING); // 设置战局状态为结束中
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {50BD10}警察阵营{53BA11}获胜！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {8EDC38}正在打扫战场...1分钟后即可创建战局！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        KillTimer(ChaseRunTimer); // 释放战局进行计时器

        foreachPlayer()
        {
            TogglePlayerSpectating(PlayerID, 0);
            SetPlayerInChaseDeath(PlayerID, 0);
            SetPlayerVirtualWorld(PlayerID, 0);
            SetPlayerInterior(PlayerID, 0);
            SetPlayerPos(PlayerID, 2582.2756, -1775.3367, 4.0523);
            SetPlayerFacingAngle(PlayerID, 295.7191);
        }

        foreachPolice()
        {
            GiveMoney(PoliceID, ceVar[PoliceBounty]);
        }

        ChaseEndTimer = SetTimer("OnChaseEnd", 60000, 0); // 创建战局结束计时器 60000 => 1分钟 只执行一次
        return 1;
    }

    // 判断战局当中的警察是否全部死亡
    if (ceVar[PoliceCountDeath] == CHASES_MAX_POLICES)
    {
        Chase_HideInfoUI();
        SetChaseStatus(CHASE_STATUS_ENDING); // 设置战局状态为结束中
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {50BD10}毒贩阵营{53BA11}获胜！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {8EDC38}正在打扫战场...1分钟后即可创建战局！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        KillTimer(ChaseRunTimer); // 释放战局进行计时器

        foreachPlayer()
        {
            TogglePlayerSpectating(PlayerID, 0);
            SetPlayerInChaseDeath(PlayerID, 0);
            SetPlayerVirtualWorld(PlayerID, 0);
            SetPlayerInterior(PlayerID, 0);
            SetPlayerPos(PlayerID, 2582.2756, -1775.3367, 4.0523);
            SetPlayerFacingAngle(PlayerID, 295.7191);
            
        }

        foreachDealer()
        {
            GiveMoney(DealerID, ceVar[DealerBounty]);
        }

        ChaseEndTimer = SetTimer("OnChaseEnd", 60000, 0); // 创建战局结束计时器 60000 => 1分钟 只执行一次
        return 1;
    }

    /* 更新战局时间 */
    TD_UpdateTimerUI();
    return 1;
}

/* 战局结束回调 */
public OnChaseEnd()
{
    /* 释放战局临时数据 */
    FreeChaseData();
    KillTimer(ChaseEndTimer); // 释放战局结束计时器
    return 1;
}

// 检查点是否靠近玩家
stock IsPointNearPoint(Float:x1, Float:y1, Float:x2, Float:y2, Float:distance) { return (floatsqroot((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) <= distance); }

stock Chase_GetName(playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof name);
    return name;
}

/* 初始化战局数据 */
stock Chase_InitData()
{
    print("[后台] 开始初始化追逐战局数据...");

    Chase_Map(); // 初始化战局地图
    SetChaseStatus(CHASE_STATUS_FREEING); // 初始化战局状态为空闲中
    print("[后台] 追逐战局数据初始化完成.");
    return 1;
}

/* 初始化战局信息UI */
stock Chase_InitInfoUI()
{
    td_ChaseInfo[0] = TextDrawCreate(493.000000, 123.000000, "ld_grav:timer");
	TextDrawFont(td_ChaseInfo[0], 4);
	TextDrawLetterSize(td_ChaseInfo[0], 0.600000, 2.000000);
	TextDrawTextSize(td_ChaseInfo[0], 22.500000, 24.000000);
	TextDrawSetOutline(td_ChaseInfo[0], 1);
	TextDrawSetShadow(td_ChaseInfo[0], 0);
	TextDrawAlignment(td_ChaseInfo[0], 2);
	TextDrawColor(td_ChaseInfo[0], -1);
	TextDrawBackgroundColor(td_ChaseInfo[0], 255);
	TextDrawBoxColor(td_ChaseInfo[0], 50);
	TextDrawUseBox(td_ChaseInfo[0], 1);
	TextDrawSetProportional(td_ChaseInfo[0], 1);
	TextDrawSetSelectable(td_ChaseInfo[0], 0);

	td_ChaseInfo[1] = TextDrawCreate(515.000000, 124.000000, "15:00");
	TextDrawFont(td_ChaseInfo[1], 3);
	TextDrawLetterSize(td_ChaseInfo[1], 0.395833, 2.249999);
	TextDrawTextSize(td_ChaseInfo[1], 340.500000, 16.500000);
	TextDrawSetOutline(td_ChaseInfo[1], 1);
	TextDrawSetShadow(td_ChaseInfo[1], 0);
	TextDrawAlignment(td_ChaseInfo[1], 1);
	TextDrawColor(td_ChaseInfo[1], -168436481);
	TextDrawBackgroundColor(td_ChaseInfo[1], 255);
	TextDrawBoxColor(td_ChaseInfo[1], 50);
	TextDrawUseBox(td_ChaseInfo[1], 0);
	TextDrawSetProportional(td_ChaseInfo[1], 1);
	TextDrawSetSelectable(td_ChaseInfo[1], 0);
}

stock Chase_InitPoliceMoneyUI(playerid)
{
    ceVar[td_PoliceMoney][playerid] = CreatePlayerTextDraw(playerid, 499.000000, 99.000000, "$00000000");
	PlayerTextDrawFont(playerid, ceVar[td_PoliceMoney][playerid], 3);
	PlayerTextDrawLetterSize(playerid, ceVar[td_PoliceMoney][playerid], 0.549999, 2.249999);
	PlayerTextDrawTextSize(playerid, ceVar[td_PoliceMoney][playerid], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, ceVar[td_PoliceMoney][playerid], 2);
	PlayerTextDrawSetShadow(playerid, ceVar[td_PoliceMoney][playerid], 0);
	PlayerTextDrawAlignment(playerid, ceVar[td_PoliceMoney][playerid], 1);
	PlayerTextDrawColor(playerid, ceVar[td_PoliceMoney][playerid], -2686721);
	PlayerTextDrawBackgroundColor(playerid, ceVar[td_PoliceMoney][playerid], 255);
	PlayerTextDrawBoxColor(playerid, ceVar[td_PoliceMoney][playerid], 50);
	PlayerTextDrawUseBox(playerid, ceVar[td_PoliceMoney][playerid], 0);
	PlayerTextDrawSetProportional(playerid, ceVar[td_PoliceMoney][playerid], 1);
	PlayerTextDrawSetSelectable(playerid, ceVar[td_PoliceMoney][playerid], 0);
    PlayerTextDrawShow(playerid, ceVar[td_PoliceMoney][playerid]);
    return 1;
}

stock Chase_UpdatePoliceMoneyUI(money)
{
    ceVar[PoliceBounty] += money;
    format(gString, sizeof gString, "$%i", ceVar[PoliceBounty]);
    foreachPolice()
    {
        PlayerTextDrawSetString(PoliceID, ceVar[td_PoliceMoney][PoliceID], gString);
    }
    gString[0] = EOS; // 释放全局数组单元格
    return 1;
}

stock Chase_InitDealerMoneyUI(playerid)
{
    ceVar[td_DealerMoney][playerid] = CreatePlayerTextDraw(playerid, 499.000000, 99.000000, "$00000000");
	PlayerTextDrawFont(playerid, ceVar[td_DealerMoney][playerid], 3);
	PlayerTextDrawLetterSize(playerid, ceVar[td_DealerMoney][playerid], 0.549999, 2.249999);
	PlayerTextDrawTextSize(playerid, ceVar[td_DealerMoney][playerid], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, ceVar[td_DealerMoney][playerid], 2);
	PlayerTextDrawSetShadow(playerid, ceVar[td_DealerMoney][playerid], 0);
	PlayerTextDrawAlignment(playerid, ceVar[td_DealerMoney][playerid], 1);
	PlayerTextDrawColor(playerid, ceVar[td_DealerMoney][playerid], -16776961);
	PlayerTextDrawBackgroundColor(playerid, ceVar[td_DealerMoney][playerid], 255);
	PlayerTextDrawBoxColor(playerid, ceVar[td_DealerMoney][playerid], 50);
	PlayerTextDrawUseBox(playerid, ceVar[td_DealerMoney][playerid], 0);
	PlayerTextDrawSetProportional(playerid, ceVar[td_DealerMoney][playerid], 1);
	PlayerTextDrawSetSelectable(playerid, ceVar[td_DealerMoney][playerid], 0);
    PlayerTextDrawShow(playerid, ceVar[td_DealerMoney][playerid]);
    return 1;
}

stock Chase_UpdateDealerMoneyUI(money)
{
    ceVar[DealerBounty] += money;
    format(gString, sizeof gString, "$%d", ceVar[DealerBounty]);

    foreachDealer()
    {
        PlayerTextDrawSetString(DealerID, ceVar[td_DealerMoney][DealerID], gString);
    }
    gString[0] = EOS; // 释放全局数组单元格
    return 1;
}

/* 显示战局信息UI */
stock Chase_ShowInfoUI()
{
    foreachPlayer()
    {
        TextDrawShowForPlayer(PlayerID, td_ChaseInfo[0]);
        TextDrawShowForPlayer(PlayerID, td_ChaseInfo[1]);
    }
    return 1;
}

/* 隐藏战局信息UI */
stock Chase_HideInfoUI()
{
    foreachPlayer()
    {
        TextDrawHideForPlayer(PlayerID, td_ChaseInfo[0]);
        TextDrawHideForPlayer(PlayerID, td_ChaseInfo[1]);
    }
    foreachPolice()
    {
        PlayerTextDrawDestroy(PoliceID, ceVar[td_PoliceMoney][PoliceID]);
    }
    foreachDealer()
    {
        PlayerTextDrawDestroy(DealerID, ceVar[td_DealerMoney][DealerID]);
    }
    return 1;
}

/* 释放战局信息UI */
stock Chase_FreeInfoUI()
{
    TextDrawDestroy(td_ChaseInfo[0]);
    TextDrawDestroy(td_ChaseInfo[1]);
    return 1;
}

/* 更新战局时间UI */
stock TD_UpdateTimerUI()
{
    static str[8];

    /* 更新战局时间 */
    if (Chase_Minutes > 0 || Chase_Seconds > 0) 
    {
        if (Chase_Seconds == 0) 
        {
            Chase_Minutes--;
            Chase_Seconds = 59;
        }else{
            Chase_Seconds--;
        }

        format(str, sizeof(str), "%02d:%02d", Chase_Minutes, Chase_Seconds);
        TextDrawSetString(td_ChaseInfo[1], str);
        foreachPlayer()
        {
            TextDrawShowForPlayer(PlayerID, td_ChaseInfo[1]);
        }
    }else{ /* 战局时间到 */
        Chase_HideInfoUI();
        SetChaseStatus(CHASE_STATUS_ENDING); // 设置战局状态为结束中
        format(gString, sizeof gString, "{53BA11}>>> 警察&毒贩-追逐战局 {50BD10}时间到了，判定毒贩获胜！！！{53BA11} <<<");
        SendClientMessageToAll(-1, gString);
        gString[0] = EOS; // 释放全局数组单元格
        KillTimer(ChaseRunTimer); // 释放战局进行计时器

        foreachPlayer()
        {
            SetPlayerInChaseDeath(PlayerID, 0);
            SetPlayerVirtualWorld(PlayerID, 0);
            SetPlayerInterior(PlayerID, 0);
            SetPlayerPos(PlayerID, 2582.2756, -1775.3367, 4.0523);
            SetPlayerFacingAngle(PlayerID, 295.7191);
        }

        foreachDealer()
        {
            GiveMoney(DealerID, ceVar[DealerBounty]);
        }

        ChaseEndTimer = SetTimer("OnChaseEnd", 60000, 0); // 创建战局结束计时器 60000 => 1分钟 只执行一次
    }
    return 1;
}

stock RandDealerSkin()
{
    new numbers[] = {102, 103, 104, 127, 116, 181, 242, 241, 264};
    new size = sizeof(numbers);
    new randomIndex = random(size);
    new randomNumber = numbers[randomIndex];
    return randomNumber;
}

stock InitChasePlayerData()
{
    ceVar[PoliceVehicle][0] = CreateVehicle(596, 1601.92, -1683.92, 5.95, 91.469680, 1, 1, 0);
    ceVar[PoliceVehicle][1] = CreateVehicle(596, 1602.35, -1692.11, 5.95, 91.469680, 1, 1, 0);
    ceVar[PoliceVehicle][2] = CreateVehicle(596, 1602.35, -1700.29, 5.95, 91.469680, 1, 1, 0);
    SetVehicleVirtualWorld(ceVar[PoliceVehicle][0], 889);
    SetVehicleVirtualWorld(ceVar[PoliceVehicle][1], 889);
    SetVehicleVirtualWorld(ceVar[PoliceVehicle][2], 889);

    ceVar[DealerVehicle][0] = CreateVehicle(482, 2161.79, -1703.29, 15.08, 270.233886, 1, 1, 0);
    ceVar[DealerVehicle][1] = CreateVehicle(567, 2165.03, -1697.40, 15.09, 235.880966, 1, 1, 0);
    ceVar[DealerVehicle][2] = CreateVehicle(567, 2167.32, -1693.43, 15.09, 218.711410, 1, 1, 0);
    SetVehicleVirtualWorld(ceVar[DealerVehicle][0], 889);
    SetVehicleVirtualWorld(ceVar[DealerVehicle][1], 889);
    SetVehicleVirtualWorld(ceVar[DealerVehicle][2], 889);

    Chase_ShowInfoUI();

    /* 初始化警察阵营人员数据 */
    new Float:PoffsetY = -1683.59;
    foreachPolice()
    {
        Chase_InitPoliceMoneyUI(PoliceID);
        PoffsetY--;
        new randWeapon = random(2) ? 29 : 28; // MP5 或 Uzi
        new randAmmo = random(60)+180;

        SetPlayerSkin(PoliceID, 280);
        GivePlayerWeapon(PoliceID, 3, 0);
        GivePlayerWeapon(PoliceID, 31, 240);
        GivePlayerWeapon(PoliceID, randWeapon, randAmmo);
        GivePlayerWeapon(PoliceID, 16, 3); // 手榴弹

        SetPlayerHealth(PoliceID, 100.0);
        SetPlayerArmour(PoliceID, 100.0);
        SetPlayerVirtualWorld(PoliceID, 889);
        SetPlayerInterior(PoliceID, 0);
        SetPlayerPos(PoliceID, 1592.29, PoffsetY, 5.89); // 警察出生点  
        SetPlayerFacingAngle(PoliceID, 268.612762);
    }
    Chase_UpdatePoliceMoneyUI(CHASES_BASE_POLICES_MONEY);

    /* 初始化毒贩阵营人员数据 */
    new Float:DoffsetY = -1706.64;
    foreachDealer()
    {
        Chase_InitDealerMoneyUI(DealerID);
        DoffsetY++;
        new randWeapon = random(2) ? 32 : 28; // Tec-9 或 Uzi
        new randAmmo = random(60)+180;

        SetPlayerSkin(DealerID, RandDealerSkin());
        GivePlayerWeapon(DealerID, 5, 0); // 棒球棒
        GivePlayerWeapon(DealerID, 30, 240); // AK47
        GivePlayerWeapon(DealerID, randWeapon, randAmmo);
        GivePlayerWeapon(DealerID, 18, 2); // 燃烧瓶

        SetPlayerHealth(DealerID, 100.0);
        SetPlayerArmour(DealerID, 100.0);
        SetPlayerVirtualWorld(DealerID, 889);
        SetPlayerInterior(DealerID, 0);
        SetPlayerPos(DealerID, 2151.44, DoffsetY, 15.09);
        SetPlayerFacingAngle(DealerID, 229.151062);
    }
    Chase_UpdateDealerMoneyUI(CHASES_BASE_DEALERS_MONEY);
    return 1;
}

/* 释放战局数据 */
stock FreeChaseData()
{
    /* 释放警察车辆 */
    for (new i; i < 3; i++)
    {
        if (IsValidVehicle(ceVar[PoliceVehicle][i])) DestroyVehicle(ceVar[PoliceVehicle][i]);
    }
    /* 释放毒贩车辆 */
    for (new i; i < 3; i++)
    {
        if (IsValidVehicle(ceVar[DealerVehicle][i])) DestroyVehicle(ceVar[DealerVehicle][i]);
    }

    /* 重置战局临时数据 */
    static const ResetData[ChaseDataStruct]; 
	ceVar = ResetData;

    Chase_Minutes = CHASES_MAX_MINUTES;
    Chase_Seconds = CHASES_MAX_SECONDS;

    foreachPlayer()
        List_Remove_Player(PlayerID);
    foreachPolice()
        List_Remove_Police(PoliceID);
    foreachDealer()
        List_Remove_Dealer(DealerID);

    List_PlayPlayer[0] = EOS;
    List_PlayPolice[0] = EOS;
    List_PlayDealer[0] = EOS;

    SetChaseStatus(CHASE_STATUS_FREEING); // 设置战局状态为空闲中
    SendClientMessageToAll(-1, "{53BA11}>>> 警察&毒贩-追逐战局 {50BD10}打扫完成, 输入/ceadd 创建战局{53BA11} <<<");
    return 1;
}

/* 创建战局对话框 
 * @playerid 玩家ID
 * @response 玩家选择的按钮
*/
stock DLG_CreateChase(playerid, response)
{
    if (!response) return 1;
    /* Fix: 检查战局是否在集结中 */
    if (GetChaseStatus() == CHASE_STATUS_GATHERING) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局正在集结中, 无法创建。请输入: /cejoin 加入。");

    SetChaseStatus(CHASE_STATUS_GATHERING); // 设置战局状态为集结中

    format(gString, sizeof gString, "{E68911}>>> 警察&毒贩-追逐战局 正在集结玩家中！！！请输入/cejoin 加入战局。  [战局发起人:{1584DA}%s[%i]{E68911}] <<<", Chase_GetName(playerid), playerid);
    SendClientMessageToAll(-1, gString);
    SendClientMessageToAll(-1, gString);
    SendClientMessageToAll(-1, gString);
    gString[0] = EOS; // 释放全局数组单元格

    CheckChaseGatherTimer = SetTimer("OnCheckChaseGatherStatus", 1000, 1); // 检查战局集结状态
    return 1;
}

/* 选择扮演角色对话框 
 * @playerid 玩家ID
 * @response 玩家选择的按钮
 * @listitem 玩家选择的列表项
*/
stock DLG_SelectPlay(playerid, response, listitem)
{
    if (!response) return 1;

    switch (listitem)
    {
        case 0:
        {
            if (List_GetSize(List_PlayPolice) >= CHASES_MAX_POLICES) return SendClientMessage(playerid, -1, CHASE_INFO"警察阵营人员已满！");
            if (IsPlayerPolice(playerid)) return SendClientMessage(playerid, -1, CHASE_INFO"你目前为警察阵营，输入/cejoin 可切换其他阵营。");

            if (IsPlayerDealer(playerid))
            {
                List_Remove_Dealer(playerid);
                List_Add_Police(playerid);

                format(gString, sizeof gString, CHASE_INFO"玩家 {1584DA}%s[%i]{13ABDC} 更换到了警察阵营。", Chase_GetName(playerid), playerid);
                SendClientMessageToAll(-1, gString);
                gString[0] = EOS; // 释放全局数组单元格
            }else{
                List_Add_Player(playerid);
                List_Add_Police(playerid);

                format(gString, sizeof gString, CHASE_INFO"玩家 {1584DA}%s[%i]{13ABDC} 选择了警察阵营。", Chase_GetName(playerid), playerid);
                SendClientMessageToAll(-1, gString);
                gString[0] = EOS; // 释放全局数组单元格
            }
        }
        case 1:
        {
            if (List_GetSize(List_PlayDealer) >= CHASES_MAX_DEALERS) return SendClientMessage(playerid, -1, CHASE_INFO"毒贩阵容人员已满！");
            if (IsPlayerDealer(playerid)) return SendClientMessage(playerid, -1, CHASE_INFO"你目前为毒贩阵营，输入/cejoin 可切换其他阵营。");

            if (IsPlayerPolice(playerid))
            {
                List_Remove_Police(playerid);
                List_Add_Dealer(playerid);

                format(gString, sizeof gString, CHASE_INFO"玩家 {1584DA}%s[%i]{13ABDC} 更换到了毒贩阵营。", Chase_GetName(playerid), playerid);
                SendClientMessageToAll(-1, gString);
                gString[0] = EOS; // 释放全局数组单元格
            }else{
                List_Add_Player(playerid);
                List_Add_Dealer(playerid);

                format(gString, sizeof gString, CHASE_INFO"玩家 {1584DA}%s[%i]{13ABDC} 选择了毒贩阵营。", Chase_GetName(playerid), playerid);
                SendClientMessageToAll(-1, gString);
                gString[0] = EOS; // 释放全局数组单元格
            }
        }
        case 2:
        {
            if (IsPlayerInChasePlayer(playerid))
            {
                if (IsPlayerPolice(playerid))
                    List_Remove_Police(playerid);
                if (IsPlayerDealer(playerid))
                    List_Remove_Dealer(playerid);
                
                List_Remove_Player(playerid);
                format(gString, sizeof gString, CHASE_INFO"玩家 {1584DA}%s[%i]{E68911} 主动退出了战局。", Chase_GetName(playerid), playerid);
                SendClientMessageToAll(-1, gString);
                gString[0] = EOS; // 释放全局数组单元格
            }
        }

        default: return 1;
    }
    format(gString, sizeof gString, "{E68911}>>> 目前 玩家人数:%i/%i - 扮演警察人数:%i/%i - 扮演毒贩人数: %i/%i {E68911}<<<", 
    List_GetSize(List_PlayPlayer), CHASES_MAX_PLAYERS, List_GetSize(List_PlayPolice), CHASES_MAX_POLICES, List_GetSize(List_PlayDealer), CHASES_MAX_DEALERS);
    SendClientMessageToAll(-1, gString);
    gString[0] = EOS; // 释放全局数组单元格
    return 1;
}

stock DelBuild(playerid)
{
    RemoveBuildingForPlayer(playerid, 1266, 1538.5234, -1609.8047, 19.8438, 0.25);
    RemoveBuildingForPlayer(playerid, 1266, 1565.4141, -1722.3125, 25.0391, 0.25);
    RemoveBuildingForPlayer(playerid, 4229, 1597.9063, -1699.7500, 30.2109, 0.25);
    RemoveBuildingForPlayer(playerid, 4230, 1597.9063, -1699.7500, 30.2109, 0.25);
    RemoveBuildingForPlayer(playerid, 713, 1496.8672, -1707.8203, 13.4063, 0.25);
    RemoveBuildingForPlayer(playerid, 1231, 1479.6953, -1716.7031, 15.6250, 0.25);
    RemoveBuildingForPlayer(playerid, 1289, 1504.7500, -1711.8828, 13.5938, 0.25);
    RemoveBuildingForPlayer(playerid, 673, 1457.7266, -1710.0625, 12.3984, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1461.6563, -1707.6875, 11.8359, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1468.9844, -1704.6406, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 700, 1463.0625, -1701.5703, 13.7266, 0.25);
    RemoveBuildingForPlayer(playerid, 1231, 1479.6953, -1702.5313, 15.6250, 0.25);
    RemoveBuildingForPlayer(playerid, 673, 1457.5547, -1697.2891, 12.3984, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1468.9844, -1694.0469, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 1231, 1479.3828, -1692.3906, 15.6328, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1461.1250, -1687.5625, 11.8359, 0.25);
    RemoveBuildingForPlayer(playerid, 700, 1463.0625, -1690.6484, 13.7266, 0.25);
    RemoveBuildingForPlayer(playerid, 641, 1458.6172, -1684.1328, 11.1016, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1468.9844, -1682.7188, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 1231, 1479.3828, -1682.3125, 15.6328, 0.25);
    RemoveBuildingForPlayer(playerid, 1231, 1477.9375, -1652.7266, 15.6328, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1488.7656, -1704.5938, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 700, 1494.2109, -1694.4375, 13.7266, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1488.7656, -1693.7344, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1496.9766, -1686.8516, 11.8359, 0.25);
    RemoveBuildingForPlayer(playerid, 641, 1494.1406, -1689.2344, 11.1016, 0.25);
    RemoveBuildingForPlayer(playerid, 1280, 1488.7656, -1682.6719, 13.4531, 0.25);
    RemoveBuildingForPlayer(playerid, 1232, 1494.4141, -1629.9766, 15.5313, 0.25);
    RemoveBuildingForPlayer(playerid, 1288, 1504.7500, -1705.4063, 13.5938, 0.25);
    RemoveBuildingForPlayer(playerid, 1287, 1504.7500, -1704.4688, 13.5938, 0.25);
    RemoveBuildingForPlayer(playerid, 1286, 1504.7500, -1695.0547, 13.5938, 0.25);
    RemoveBuildingForPlayer(playerid, 673, 1498.9609, -1684.6094, 12.3984, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1503.1875, -1621.1250, 11.8359, 0.25);
    RemoveBuildingForPlayer(playerid, 673, 1501.2813, -1624.5781, 12.3984, 0.25);
    RemoveBuildingForPlayer(playerid, 673, 1498.3594, -1616.9688, 12.3984, 0.25);
    RemoveBuildingForPlayer(playerid, 712, 1508.4453, -1668.7422, 22.2578, 0.25);
    RemoveBuildingForPlayer(playerid, 1260, 1565.4141, -1722.3125, 25.0391, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1721.6328, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1541.4453, -1713.3047, 14.4297, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1541.4531, -1709.6406, 13.0469, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1705.2734, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1541.2969, -1702.6016, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 1229, 1524.2188, -1693.9688, 14.1094, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1546.6016, -1693.3906, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1547.5703, -1689.9844, 13.0469, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1688.0859, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1546.8672, -1687.1016, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 646, 1545.5234, -1678.8438, 14.0000, 0.25);
    RemoveBuildingForPlayer(playerid, 646, 1553.8672, -1677.7266, 16.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 1229, 1524.2188, -1673.7109, 14.1094, 0.25);
    RemoveBuildingForPlayer(playerid, 646, 1553.8672, -1673.4609, 16.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1668.0781, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 646, 1545.5625, -1672.2188, 14.0000, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1546.6016, -1664.6250, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1546.8672, -1658.3438, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1547.5703, -1661.0313, 13.0469, 0.25);
    RemoveBuildingForPlayer(playerid, 4192, 1591.6953, -1674.8516, 20.4922, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1541.4766, -1648.4531, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1647.6406, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 620, 1541.4531, -1642.0313, 13.0469, 0.25);
    RemoveBuildingForPlayer(playerid, 647, 1541.7422, -1638.9141, 14.4375, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1524.8281, -1621.9609, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 1226, 1525.3828, -1611.1563, 16.4219, 0.25);
    RemoveBuildingForPlayer(playerid, 1260, 1538.5234, -1609.8047, 19.8438, 0.25);
    return 1;
}

stock Chase_Map()
{
    CreateObject(1423, 1543.60, -1634.40, 13.24,   0.00, 0.00, 88.98);
    CreateObject(1423, 1543.13, -1620.82, 13.27,   0.00, 0.00, -88.44);
    CreateObject(8841, 1574.70, -1617.88, 15.35,   0.00, 0.00, 0.00);
    CreateObject(2745, 1546.61, -1678.83, 15.15,   0.00, 0.00, -91.14);
    CreateObject(2745, 1546.65, -1672.36, 15.15,   0.00, 0.00, -91.14);
    CreateObject(19817, 1569.71, -1612.76, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1563.73, -1612.84, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1559.07, -1612.70, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1554.24, -1612.67, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1575.30, -1612.60, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1581.12, -1612.73, 10.53,   0.00, 0.00, -180.00);
    CreateObject(19817, 1586.92, -1612.69, 10.53,   0.00, 0.00, -180.00);
    CreateObject(18646, 1544.08, -1632.43, 15.06,   10.92, -6.24, 0.00);
    CreateObject(300, 1545.64, -1632.46, 12.38,   0.00, 0.00, 0.00);
    CreateObject(3881, 1547.23, -1620.12, 14.35,   0.00, 0.00, -90.30);
    CreateObject(1215, 1544.53, -1623.89, 12.47,   0.00, 0.00, 0.00);
    CreateObject(1215, 1544.27, -1630.77, 12.69,   0.00, 0.00, 0.00);
    CreateObject(983, 1544.85, -1635.94, 13.11,   0.00, 0.00, 0.00);
    CreateObject(983, 1544.85, -1635.94, 14.40,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.25, -1638.53, 13.21,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.19, -1642.15, 13.21,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.21, -1645.89, 13.21,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.19, -1642.15, 13.21,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.18, -1649.21, 13.21,   0.00, 0.00, 0.00);
    CreateObject(649, 1541.27, -1640.31, 12.99,   0.00, 0.00, 0.00);
    CreateObject(649, 1541.32, -1644.21, 12.89,   0.00, 0.00, 0.00);
    CreateObject(649, 1541.39, -1647.69, 12.89,   0.00, 0.00, 0.00);
    CreateObject(870, 1546.05, -1658.04, 13.11,   0.00, 0.00, 0.00);
    CreateObject(870, 1546.53, -1661.59, 13.11,   0.00, 0.00, -0.48);
    CreateObject(870, 1546.00, -1664.88, 13.11,   0.00, 0.00, -0.48);
    CreateObject(870, 1545.92, -1687.28, 13.17,   0.00, 0.00, 0.00);
    CreateObject(870, 1546.09, -1691.68, 13.15,   0.00, 0.00, 0.00);
    CreateObject(649, 1546.27, -1689.60, 12.88,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.35, -1702.74, 13.15,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.34, -1706.40, 13.15,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.43, -1710.73, 13.15,   0.00, 0.00, 0.00);
    CreateObject(870, 1541.36, -1713.86, 13.15,   0.00, 0.00, 0.00);
    CreateObject(620, 1542.17, -1708.56, 12.67,   356.86, 0.00, 2.24);
    CreateObject(1215, 1592.78, -1637.67, 12.81,   0.00, 0.00, 0.00);
    CreateObject(1215, 1584.35, -1637.68, 12.65,   0.00, 0.00, 0.00);
    CreateObject(1649, 1551.81, -1617.58, 14.17,   0.00, 0.00, -0.30);
    CreateObject(1649, 1556.21, -1617.60, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1560.64, -1617.59, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1565.06, -1617.60, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1569.36, -1617.59, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1573.76, -1617.58, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1578.17, -1617.57, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1582.58, -1617.56, 14.17,   0.00, 0.00, 0.12);
    CreateObject(1649, 1587.00, -1617.55, 14.17,   0.00, 0.00, 0.12);
    CreateObject(18646, 1581.98, -1633.00, 17.01,   0.00, 0.00, 0.00);
    CreateObject(978, 1594.17, -1617.61, 13.02,   0.00, 0.00, -178.26);
    CreateObject(895, 1547.88, -1663.97, 11.50,   0.00, 0.00, 0.00);
    CreateObject(895, 1547.72, -1658.63, 11.68,   0.00, 0.00, 0.00);
    CreateObject(632, 1555.07, -1673.16, 15.57,   0.00, 0.00, 0.00);
    CreateObject(632, 1554.98, -1677.97, 15.57,   0.00, 0.00, 0.00);
    CreateObject(970, 1539.55, -1669.80, 13.09,   0.00, 0.00, -90.48);
    CreateObject(19121, 1539.60, -1667.58, 12.77,   0.00, 0.00, 0.00);
    CreateObject(19121, 1539.54, -1672.05, 12.77,   0.00, 0.00, 0.00);
    CreateObject(970, 1539.58, -1660.16, 13.09,   0.00, 0.00, -90.48);
    CreateObject(19121, 1539.56, -1662.44, 12.77,   0.00, 0.00, 0.00);
    CreateObject(19121, 1539.59, -1657.96, 12.77,   0.00, 0.00, 0.00);
    CreateObject(970, 1539.53, -1680.30, 13.09,   0.00, 0.00, -90.48);
    CreateObject(19121, 1539.49, -1682.54, 12.77,   0.00, 0.00, 0.00);
    CreateObject(19121, 1539.51, -1678.08, 12.77,   0.00, 0.00, -22.68);
    CreateObject(970, 1539.54, -1689.47, 13.05,   0.00, 0.00, -450.24);
    CreateObject(19121, 1539.54, -1687.29, 12.77,   0.00, 0.00, 0.00);
    CreateObject(19121, 1539.56, -1691.69, 12.77,   0.00, 0.00, 0.00);
    CreateObject(970, 1537.09, -1699.48, 13.05,   0.00, 0.00, -450.24);
    CreateObject(970, 1537.14, -1708.20, 13.05,   0.00, 0.00, -450.24);
    CreateObject(970, 1537.02, -1648.61, 13.09,   0.00, 0.00, -90.48);
    CreateObject(970, 1537.12, -1640.93, 13.09,   0.00, 0.00, -89.94);
    CreateObject(1215, 1537.16, -1638.73, 12.75,   0.00, 0.00, 0.00);
    CreateObject(1215, 1537.12, -1643.16, 12.75,   0.00, 0.00, 0.00);
    CreateObject(1215, 1537.00, -1646.42, 12.75,   0.00, 0.00, 0.00);
    CreateObject(1215, 1537.03, -1650.84, 12.75,   0.00, 0.00, 0.00);
    CreateObject(19125, 1537.12, -1697.26, 12.81,   0.00, 0.00, 0.00);
    CreateObject(19125, 1537.11, -1701.70, 12.81,   0.00, 0.00, 0.00);
    CreateObject(19125, 1537.14, -1706.02, 12.81,   0.00, 0.00, -0.06);
    CreateObject(19125, 1537.16, -1710.44, 12.81,   0.00, 0.00, -0.06);
    CreateObject(3515, 1546.73, -1681.80, 11.82,   0.00, 0.00, 0.00);
    CreateObject(1215, 1545.27, -1681.89, 12.78,   0.00, 0.00, 0.00);
    CreateObject(19121, 1546.76, -1680.39, 12.76,   0.00, 0.00, 0.00);
    CreateObject(19122, 1546.61, -1683.24, 12.82,   0.00, 0.00, 0.00);
    CreateObject(19123, 1548.17, -1681.90, 12.76,   0.00, 0.00, 0.00);
    CreateObject(3515, 1547.26, -1669.25, 11.82,   0.00, 0.00, 0.00);
    CreateObject(1215, 1545.83, -1669.32, 12.78,   0.00, 0.00, 0.00);
    CreateObject(19122, 1547.32, -1670.73, 12.82,   0.00, 0.00, 0.00);
    CreateObject(19121, 1547.02, -1667.78, 12.76,   0.00, 0.00, 0.00);
    CreateObject(19123, 1548.64, -1669.02, 12.76,   0.00, 0.00, 0.00);
    CreateObject(970, 154146.00, -1632.78, 12.94,   0.00, 0.00, -179.16);
    CreateObject(970, 1544.48, -1621.68, 12.94,   0.00, 0.00, -269.94);
    CreateObject(970, 154233.00, -1622.92, 12.88,   0.00, 0.00, -360.78);
    CreateObject(970, 1598.91, -1615.59, 12.90,   0.00, 0.00, 88.62);
    CreateObject(983, 1539.77, -1605.65, 16.61,   0.00, 0.00, 0.00);
    CreateObject(983, 1539.78, -1612.11, 16.61,   0.00, 0.00, 0.00);
    CreateObject(983, 1539.74, -1614.55, 16.61,   0.00, 0.00, 0.00);
    CreateObject(983, 1542.93, -1617.74, 16.61,   0.00, 0.00, -89.70);
    CreateObject(983, 1546.19, -1617.69, 16.61,   0.00, 0.00, -89.70);
    CreateObject(983, 1543.01, -1602.47, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1549.46, -1602.46, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1555.89, -1602.46, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1562.34, -1602.46, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1568.77, -1602.46, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1575.20, -1602.46, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1581.65, -1602.47, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1588.07, -1602.50, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1594.51, -1602.53, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1600.93, -1602.54, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1604.77, -1602.55, 16.61,   0.00, 0.00, -90.24);
    CreateObject(983, 1607.96, -1605.80, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1607.99, -1612.20, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1608.00, -1618.60, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1608.03, -1625.00, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1608.06, -1631.45, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1608.07, -1634.66, 16.61,   0.00, 0.00, -179.76);
    CreateObject(983, 1604.86, -1637.95, 16.61,   0.00, 0.00, -269.40);
    CreateObject(1609, 1552.39, -1666.60, 12.82,   0.00, 0.00, 90.24);
    CreateObject(1609, 1552.40, -1684.51, 12.78,   0.00, 0.00, 89.94);
    CreateObject(3528, 1553.20, -1675.08, 25.21,   0.00, 0.00, -183.90);
    CreateObject(3267, 1543.97, -1638.77, 26.89,   0.00, 0.00, 75.30);
    CreateObject(3267, 1544.43, -1712.61, 26.89,   0.00, 0.00, 111.90);
    CreateObject(3267, 1543.94, -1649.19, 26.89,   0.00, 0.00, 106.20);
    CreateObject(3267, 1544.32, -1702.19, 26.89,   0.00, 0.00, 58.26);
    CreateObject(18646, 1581.97, -1637.58, 17.01,   0.00, 0.00, 0.00);
    CreateObject(12950, 1571.86, -1634.03, 13.60,   0.00, 0.00, -89.70);
    CreateObject(12987, 1578.50, -1659.01, 24.80,   0.00, 0.00, 0.00);
    CreateObject(14416, 1580.95, -1652.99, 18.09,   0.00, 0.00, 91.44);
    CreateObject(983, 1577.96, -1654.01, 21.96,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.95, -1657.22, 23.23,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.95, -1657.22, 24.48,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.95, -1657.22, 25.77,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.96, -1658.32, 28.04,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.94, -1651.88, 28.04,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.96, -1645.44, 28.04,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.94, -1651.88, 28.04,   0.00, 0.00, 0.00);
    CreateObject(983, 1577.94, -1640.40, 28.04,   0.00, 0.00, 0.00);
    CreateObject(983, 1574.69, -1637.17, 28.04,   0.00, 0.00, -90.66);
    CreateObject(983, 1568.27, -1637.20, 28.04,   0.00, 0.00, -88.92);
    CreateObject(983, 1561.84, -1637.22, 28.04,   0.00, 0.00, -90.54);
    CreateObject(983, 1555.43, -1637.16, 28.04,   0.00, 0.00, -90.54);
    CreateObject(1775, 1507.31, -1656.82, 13.89,   0.00, 0.00, 78.96);
    CreateObject(19122, 1513.65, -1685.62, 13.36,   0.00, 0.00, 0.00);
    CreateObject(19122, 1505.98, -1667.05, 13.36,   0.00, 0.00, 0.00);
    CreateObject(18655, 1517.13, -1688.05, 12.70,   0.00, 0.00, -115.68);
    CreateObject(18655, 1517.07, -1642.26, 12.59,   0.00, 0.00, -241.98);
    CreateObject(19872, 1602.33, -1683.93, 3.03,   0.00, 0.00, 89.58);
    CreateObject(19872, 1602.32, -1692.14, 3.03,   0.00, 0.00, 89.58);
    CreateObject(19872, 1602.13, -1700.29, 3.03,   0.00, 0.00, 89.58);
    CreateObject(19872, 1595.44, -1710.89, 3.03,   0.00, 0.00, -0.78);
    CreateObject(19872, 1587.34, -1710.81, 3.03,   0.00, 0.00, -0.78);
    CreateObject(19872, 1578.70, -1710.77, 3.03,   0.00, 0.00, -0.78);
    CreateObject(19872, 1570.24, -1710.92, 3.03,   0.00, 0.00, -0.54);
    CreateObject(970, 1603.58, -1682.00, 5.36,   0.00, 0.00, 0.00);
    CreateObject(970, 1603.53, -1685.87, 5.36,   0.00, 0.00, 0.00);
    CreateObject(970, 1603.56, -1690.03, 5.36,   0.00, 0.00, -0.06);
    CreateObject(970, 1603.59, -1694.00, 5.36,   0.00, 0.00, -0.06);
    CreateObject(970, 1603.51, -1698.17, 5.36,   0.00, 0.00, -0.06);
    CreateObject(970, 1603.57, -1702.12, 5.36,   0.00, 0.00, -0.06);
    CreateObject(970, 1603.64, -1706.37, 5.36,   0.00, 0.00, -0.06);
    CreateObject(970, 1597.31, -1712.45, 5.43,   0.00, 0.00, -89.88);
    CreateObject(970, 1593.49, -1712.58, 5.43,   0.00, 0.00, -89.88);
    CreateObject(970, 1589.39, -1712.60, 5.43,   0.00, 0.00, -89.88);
    CreateObject(970, 1585.54, -1712.50, 5.43,   0.00, 0.00, -89.88);
    CreateObject(970, 1580.67, -1712.67, 5.35,   0.00, 0.00, -89.88);
    CreateObject(970, 1576.45, -1712.63, 5.35,   0.00, 0.00, -89.88);
    CreateObject(970, 1572.45, -1712.56, 5.35,   0.00, 0.00, -89.88);
    CreateObject(1649, 1605.71, -1683.89, 6.51,   0.00, 0.00, -89.76);
    CreateObject(1649, 1605.68, -1691.94, 6.51,   0.00, 0.00, -89.94);
    CreateObject(1649, 1605.62, -1700.10, 6.51,   0.00, 0.00, -89.94);
    CreateObject(970, 1599.49, -1706.36, 5.36,   0.00, 0.00, -0.06);
    CreateObject(1649, 1595.42, -1714.75, 6.51,   0.00, 0.00, -180.06);
    CreateObject(1649, 1587.54, -1714.74, 6.51,   0.00, 0.00, -180.06);
    CreateObject(1649, 1578.62, -1714.77, 6.51,   0.00, 0.00, -180.06);
    CreateObject(970, 1568.24, -1712.57, 5.35,   0.00, 0.00, -89.88);
    CreateObject(1649, 1570.40, -1714.70, 6.51,   0.00, 0.00, -180.06);
    CreateObject(19872, 1559.07, -1711.38, 3.03,   0.00, 0.00, -0.54);
    CreateObject(1649, 1558.82, -1714.79, 6.51,   0.00, 0.00, -180.06);
    CreateObject(970, 1560.94, -1712.64, 5.35,   0.00, 0.00, -89.88);
    CreateObject(970, 1556.72, -1712.64, 5.35,   0.00, 0.00, -89.88);
    CreateObject(19126, 1601.36, -1682.04, 5.15,   0.00, 0.00, 0.00);
    CreateObject(19126, 1601.29, -1685.87, 5.15,   0.00, 0.00, 0.00);
    CreateObject(19126, 1601.29, -1690.02, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19126, 1601.37, -1693.98, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19126, 1601.30, -1698.16, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19126, 1601.33, -1702.11, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19126, 1597.30, -1710.25, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19126, 1593.49, -1710.31, 5.15,   0.00, 0.00, -0.18);
    CreateObject(19124, 1589.42, -1710.40, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1585.55, -1710.31, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1580.64, -1710.48, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1576.40, -1710.41, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1572.44, -1710.38, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1568.19, -1710.33, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1560.95, -1710.46, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19124, 1556.70, -1710.45, 5.10,   0.00, 0.00, 0.00);
    CreateObject(19872, 1545.37, -1684.33, 3.03,   0.00, 0.00, 90.66);
    CreateObject(19872, 1545.26, -1676.11, 3.03,   0.00, 0.00, 90.90);
    CreateObject(19872, 1545.47, -1667.82, 3.03,   0.00, 0.00, 90.78);
    CreateObject(19872, 1545.73, -1659.02, 3.03,   0.00, 0.00, 90.78);
    CreateObject(19872, 1545.65, -1651.04, 3.03,   0.00, 0.00, 90.30);
    CreateObject(19872, 1538.67, -1644.62, 3.03,   0.00, 0.00, 179.88);
    CreateObject(19872, 1530.47, -1644.65, 3.03,   0.00, 0.00, 179.58);
    CreateObject(19872, 1529.11, -1683.94, 3.03,   0.00, 0.00, 90.66);
    CreateObject(19872, 1528.85, -1688.05, 3.03,   0.00, 0.00, 90.66);
    CreateObject(970, 1528.75, -1685.98, 5.40,   0.00, 0.00, -181.02);
    CreateObject(970, 1528.75, -1685.98, 6.46,   0.00, 0.00, -181.02);
    CreateObject(19127, 1530.95, -1686.00, 5.21,   0.00, 0.00, 0.00);
    CreateObject(19127, 1526.51, -1685.96, 5.21,   0.00, 0.00, 0.00);
    CreateObject(979, 1589.52, -1669.86, 5.50,   0.00, 0.00, -90.18);
    CreateObject(978, 1605.36, -1669.83, 5.49,   0.00, 0.00, -270.36);
    CreateObject(978, 1605.33, -1677.17, 5.49,   0.00, 0.00, -270.36);
    CreateObject(979, 1589.51, -1679.00, 5.50,   0.00, 0.00, -90.18);
    CreateObject(19817, 1546.15, -1615.54, 10.53,   0.00, 0.00, -270.60);
    CreateObject(19817, 1546.27, -1611.21, 10.53,   0.00, 0.00, -270.60);
    CreateObject(19817, 1546.28, -1606.52, 10.53,   0.00, 0.00, -270.54);
    CreateObject(1649, 1541.35, -1615.27, 14.07,   0.00, 0.00, -270.54);
    CreateObject(1649, 1541.39, -1610.93, 14.07,   0.00, 0.00, -270.66);
    CreateObject(1649, 1541.46, -1606.54, 14.07,   0.00, 0.00, -270.96);
    CreateObject(18655, 1546.60, -1604.89, 12.32,   0.00, 0.00, 107.76);
    CreateObject(18655, 1546.23, -1617.06, 12.32,   0.00, 0.00, 252.00);
    CreateObject(19174, 1508.09, -1677.22, 15.48,   0.00, 0.00, -245.40);
    CreateObject(19172, 1506.51, -1671.31, 15.78,   0.00, 0.00, 98.34);
    CreateObject(19610, 1510.97, -1666.82, 14.70,   0.00, 0.00, -171.84);
    CreateObject(19149, 1509.45, -1680.16, 16.69,   0.00, 0.00, -61.20);
    CreateObject(19149, 1508.83, -1678.83, 16.69,   0.00, 0.00, -57.78);
    CreateObject(19146, 1507.20, -1675.27, 16.62,   0.00, 0.00, -62.70);
    CreateObject(19146, 1506.98, -1673.75, 16.62,   0.00, 0.00, -82.02);
    CreateObject(19153, 1506.19, -1669.32, 16.71,   0.00, 0.00, -82.68);
    CreateObject(19153, 1506.14, -1667.87, 16.71,   0.00, 0.00, -82.68);
    CreateObject(19122, 1509.63, -1680.20, 13.33,   0.00, 0.00, 0.00);
    CreateObject(19122, 1507.47, -1675.60, 13.33,   0.00, 0.00, 0.00);
    CreateObject(18885, 1506.71, -1661.28, 13.87,   0.00, 0.00, 82.32);
    CreateObject(18661, 1535.64, -1667.35, 12.40,   0.24, 89.94, 0.00);
    CreateObject(18660, 1535.52, -1677.86, 12.40,   -0.06, 89.70, -181.44);
    CreateObject(18761, 1509.12, -1677.05, 22.16,   0.00, 0.00, 112.44);
    CreateObject(1231, 1588.36, -1666.89, 7.58,   0.00, 0.00, -2.58);
    CreateObject(1231, 1588.29, -1669.67, 7.58,   0.00, 0.00, -2.58);
    CreateObject(1231, 1588.26, -1672.71, 7.58,   0.00, 0.00, -2.58);
    CreateObject(1231, 1588.16, -1675.59, 7.58,   0.00, 0.00, -2.58);
    CreateObject(970, 1583.17, -1681.73, 5.36,   0.00, 0.00, 0.00);
    CreateObject(970, 1587.35, -1681.74, 5.36,   0.00, 0.00, 0.00);
    CreateObject(1231, 1588.29, -1678.13, 7.58,   0.00, 0.00, -2.58);
    CreateObject(1231, 1588.45, -1680.76, 7.58,   0.00, 0.00, -2.58);
    CreateObject(970, 1610.60, -1678.72, 5.72,   0.00, 0.00, 0.00);
    CreateObject(1231, 1606.41, -1667.01, 7.28,   0.00, 0.00, -2.58);
    CreateObject(1231, 1606.66, -1669.71, 7.28,   0.00, 0.00, -2.58);
    CreateObject(1231, 1606.49, -1672.76, 7.28,   0.00, 0.00, -2.58);
    CreateObject(1231, 1606.66, -1669.71, 7.28,   0.00, 0.00, -2.58);
    CreateObject(1231, 1606.54, -1675.64, 7.28,   0.00, 0.00, -2.58);
    CreateObject(18850, 1566.11, -1702.89, 16.59,   0.00, 0.00, 0.30);
    CreateObject(870, 1495.03, -1685.92, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1494.96, -1690.37, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1499.49, -1685.72, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1499.44, -1690.13, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1499.23, -1694.95, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1494.82, -1695.48, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1499.13, -1700.03, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1494.84, -1700.28, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1498.81, -1705.38, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1494.53, -1705.34, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1498.87, -1710.59, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1494.44, -1710.41, 13.79,   0.00, 0.00, -0.66);
    CreateObject(649, 1493.06, -1688.20, 13.54,   0.00, 0.00, 0.00);
    CreateObject(649, 1493.17, -1693.60, 13.54,   0.00, 0.00, 0.00);
    CreateObject(649, 1493.04, -1698.27, 13.54,   0.00, 0.00, 0.00);
    CreateObject(649, 1493.02, -1703.13, 13.54,   0.00, 0.00, 0.00);
    CreateObject(649, 1493.03, -1708.09, 13.54,   0.00, 0.00, 0.00);
    CreateObject(649, 1493.19, -1712.81, 13.54,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.70, -1688.15, 12.93,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.74, -1693.21, 12.93,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.64, -1697.48, 12.93,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.74, -1702.45, 12.93,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.32, -1708.16, 12.93,   0.00, 0.00, 0.00);
    CreateObject(895, 1497.46, -1713.10, 12.93,   0.00, 0.00, 0.00);
    CreateObject(3578, 1487.54, -1689.84, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1487.45, -1694.02, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1487.45, -1698.38, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1487.48, -1702.66, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1487.55, -1707.03, 12.37,   0.00, 0.00, 0.00);
    CreateObject(970, 1487.52, -1687.42, 13.56,   0.00, 0.00, 0.00);
    CreateObject(970, 1483.37, -1687.41, 13.56,   0.00, 0.00, 0.00);
    CreateObject(970, 1473.59, -1687.39, 13.56,   0.00, 0.00, 0.00);
    CreateObject(3578, 1469.08, -1689.90, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1469.13, -1693.95, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1469.10, -1698.46, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1469.10, -1702.77, 12.37,   0.00, 0.00, 0.00);
    CreateObject(3578, 1468.93, -1707.42, 12.37,   0.00, 0.00, 0.00);
    CreateObject(2773, 1475.77, -1686.26, 13.55,   0.00, 0.00, 0.00);
    CreateObject(2773, 1481.13, -1686.32, 13.55,   0.00, 0.00, 0.00);
    CreateObject(970, 1469.46, -1687.38, 13.56,   0.00, 0.00, 0.00);
    CreateObject(870, 1457.52, -1710.52, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1462.23, -1710.79, 13.79,   0.00, 0.00, -0.60);
    CreateObject(870, 1457.25, -1705.73, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1462.32, -1705.88, 13.79,   0.00, 0.00, -0.54);
    CreateObject(673, 1455.61, -1683.30, 12.40,   356.86, 0.00, 3.14);
    CreateObject(870, 1457.35, -1701.02, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1462.13, -1700.96, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1462.30, -1695.56, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1457.17, -1695.01, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1457.36, -1689.92, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1462.66, -1690.15, 13.79,   0.00, 0.00, -0.54);
    CreateObject(870, 1457.41, -1684.51, 13.79,   0.00, 0.00, -1.38);
    CreateObject(870, 1462.92, -1684.91, 13.79,   0.00, 0.00, -0.54);
    CreateObject(641, 1461.20, -1685.03, 11.10,   356.86, 0.00, 3.14);
    CreateObject(641, 1461.07, -1690.26, 11.10,   356.86, 0.00, 3.14);
    CreateObject(673, 1455.94, -1712.32, 12.40,   356.86, 0.00, 3.26);
    CreateObject(641, 1460.70, -1695.74, 11.10,   356.86, 0.00, 3.14);
    CreateObject(641, 1460.39, -1701.14, 11.10,   356.86, 0.00, 3.02);
    CreateObject(641, 1460.28, -1706.09, 11.10,   356.86, 0.00, 2.90);
    CreateObject(641, 1460.48, -1710.95, 11.10,   356.86, 0.00, 3.08);
    CreateObject(19458, 1476.96, -1673.86, 13.03,   0.12, -89.88, 90.06);
    CreateObject(1247, 1474.89, -1672.22, 14.59,   0.00, 0.00, -3.90);
    CreateObject(19461, 1476.98, -1671.99, 14.71,   0.00, 0.00, -90.06);
    CreateObject(19458, 1476.97, -1677.34, 13.03,   0.12, -89.88, 90.06);
    CreateObject(2614, 1477.03, -1672.15, 15.88,   0.00, 0.00, 0.00);
    CreateObject(1247, 1476.47, -1672.20, 14.59,   0.00, 0.00, -3.90);
    CreateObject(1247, 1477.56, -1672.18, 14.59,   0.00, 0.00, -2.22);
    CreateObject(1247, 1478.81, -1672.22, 14.59,   0.00, 0.00, -3.90);
    CreateObject(3806, 1473.29, -1679.47, 13.09,   0.00, 0.00, -450.06);
    CreateObject(3806, 1475.81, -1679.47, 13.09,   0.00, 0.00, -450.24);
    CreateObject(3806, 1478.34, -1679.47, 13.09,   0.00, 0.00, -450.18);
    CreateObject(3806, 1480.88, -1679.46, 13.09,   0.00, 0.00, -449.52);
    CreateObject(1721, 1478.58, -1675.44, 13.11,   0.00, 0.00, -180.06);
    CreateObject(1721, 1480.07, -1675.43, 13.11,   0.00, 0.00, -180.06);
    CreateObject(1721, 1474.97, -1675.39, 13.11,   0.00, 0.00, -180.06);
    CreateObject(1721, 1473.41, -1675.42, 13.11,   0.00, 0.00, -180.06);
    CreateObject(1721, 1473.02, -1683.25, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1474.78, -1683.31, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1476.32, -1683.34, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1477.82, -1683.36, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1479.42, -1683.37, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1481.12, -1683.38, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1476.27, -1681.67, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1477.86, -1681.72, 13.12,   0.00, 0.00, -0.06);
    CreateObject(1721, 1481.15, -1681.86, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1479.40, -1681.86, 13.12,   0.00, 0.00, -0.06);
    CreateObject(3515, 1469.26, -1675.22, 12.36,   0.00, 0.00, 0.00);
    CreateObject(3515, 1484.31, -1675.08, 12.36,   0.00, 0.00, 0.00);
    CreateObject(1721, 1474.63, -1681.64, 13.12,   0.00, 0.00, 0.00);
    CreateObject(1721, 1472.96, -1681.54, 13.12,   0.00, 0.00, 0.00);
    CreateObject(19932, 1476.31, -1677.66, 13.12,   0.00, 0.00, -450.54);
    CreateObject(19610, 1476.26, -1677.50, 14.32,   30.48, 3.48, 0.00);
    CreateObject(19610, 1476.52, -1677.52, 14.32,   30.48, 3.48, 0.00);
    CreateObject(19932, 1477.04, -1677.67, 13.12,   0.00, 0.00, -450.54);
    CreateObject(19610, 1476.95, -1677.54, 14.32,   30.48, 3.48, 0.00);
    CreateObject(19610, 1477.16, -1677.54, 14.32,   30.48, 3.48, 0.42);
    CreateObject(1247, 1476.31, -1678.02, 13.85,   0.00, 0.00, -2.22);
    CreateObject(1247, 1477.05, -1678.03, 13.85,   0.00, 0.00, -2.58);
    CreateObject(970, 1481.81, -1674.06, 13.53,   0.00, 0.00, -90.48);
    CreateObject(970, 1472.11, -1674.10, 13.53,   0.00, 0.00, -90.36);
    CreateObject(649, 1546.37, -1694.55, 12.88,   0.00, 0.00, 0.00);
    CreateObject(649, 1546.54, -1685.43, 12.88,   0.00, 0.00, 0.00);
    CreateObject(895, 1540.22, -1710.94, 11.50,   0.00, 0.00, -37.80);
    CreateObject(895, 1540.18, -1704.23, 11.50,   0.00, 0.00, -37.80);
    CreateObject(646, 1472.89, -1672.81, 14.43,   0.00, 0.00, -46.86);
    CreateObject(646, 1481.27, -1672.91, 14.48,   0.00, 0.00, -45.00);
    CreateObject(968, 1544.61, -1630.78, 13.18,   0.00, 0.00, 0.00);
    CreateObject(980, 1594.84, -1638.02, 14.70,   0.00, 0.00, -0.42);
    CreateObject(1649, 1582.12, -1635.64, 14.63,   0.00, 0.00, -89.82);
    CreateObject(1649, 1579.66, -1632.88, 14.63,   0.00, 0.00, -0.30);
    CreateObject(1237, 1517.23, -1664.38, 12.77,   0.00, 0.00, 0.00);
    CreateObject(1237, 1515.04, -1664.42, 12.77,   0.00, 0.00, 0.00);
    CreateObject(1237, 1513.03, -1664.35, 12.77,   0.00, 0.00, 1.02);
    CreateObject(1237, 1523.26, -1664.43, 12.45,   0.00, 0.00, 0.00);
    CreateObject(1237, 1510.88, -1664.33, 12.77,   0.00, 0.00, 1.08);
    CreateObject(1237, 1521.03, -1664.45, 12.50,   0.00, 0.00, 0.00);
    CreateObject(1237, 1508.54, -1664.32, 12.77,   0.00, 0.00, 0.00);
    CreateObject(1237, 1519.07, -1664.43, 12.58,   0.00, 0.00, 0.00);
    CreateObject(1237, 1506.23, -1664.41, 12.77,   0.00, 0.00, 3.66);
    CreateObject(18885, 1509.09, -1652.55, 13.87,   0.00, 0.00, 64.32);
    return 1;
}