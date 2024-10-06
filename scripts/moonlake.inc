/**
*   梦月潭 by DeepDC 灵感来源于新教父(自由国度)梦月潭
**/
#define MOOKLAKE_COLD_TIME          (2) // 梦月潭冷却时间(分钟)
#define MOOKLAKE_OPEN_TIME          (1) // 梦月潭持续时间(分钟)

#define COLOR_WHITE 0xFFFFFFFF//HEX颜色
#define COLOR_GRAD1 0xB4B5B7FF
#define COLOR_GRAD2 0xBFC0C2FF
#define COLOR_GRAD3 0xCBCCCEFF
#define COLOR_GRAD4 0xD8D8D8FF
#define COLOR_GRAD5 0xE3E3E3FF
#define COLOR_GRAD6 0xF0F0F0FF
#define COLOR_BLACK 0x000000FF
#define COLOR_DARKRED 0xAD0000FF
#define COLOR_RED 0xFC0000FF //1
#define COLOR_FADERED 0xFFA3A3FF
#define COLOR_VIOLET 0xFFADFFFF
#define COLOR_NURSEPINK 0xFFD1FFFF
#define COLOR_PINK 0xFF5585FF
#define COLOR_LIGHTRED 0xFF6060FF
#define COLOR_ORANGE 0xFF7700FF
#define COLOR_LIGHTORANGE 0xFFB75EFF
#define COLOR_YELLOW 0xFFDF00FF
#define COLOR_FADEYELLOW 0xFFF298FF
#define COLOR_LIGHTYELLOW 0xFFFF00FF
#define COLOR_YELLOWGREEN 0xECFF00FF
#define COLOR_DARKYELLOWGREEN 0x9AB900FF //2
#define COLOR_LIGHTGREEN 0x83FF00FF
#define COLOR_FADEGREEN 0xD1FF00FF
#define COLOR_GREEN 0x00CF00FF
#define COLOR_DARKGREEN 0x007F00FF
#define COLOR_BLUEGREEN 0x00F185FF
#define COLOR_LIGHTBLUE 0x00FFF2FF
#define COLOR_FADEBLUE 0x9DFFFFFF
#define COLOR_SKYBLUE 0x00DBFFFF
#define COLOR_WATERBLUE 0x00A2FFFF
#define COLOR_BLUE 0x003CFFFF
#define COLOR_PURPLE 0x5200FFFF
#define COLOR_LIGHTPURPLE 0xB100FFFF
#define COLOR_PINKRED 0xFF0095FF
#define COLOR_BLOODRED 0xC80000FF
#define COLOR_DARKBROWN 0x720000FF
#define COLOR_FADEBROWN 0x9B4A4AFF
#define COLOR_BROWN 0xBB7800FF
#define COLOR_SANDYELLOW 0xDBC841FF
#define COLOR_FADE1 0xE6E6E6E6
#define COLOR_FADE2 0xC8C8C8C8
#define COLOR_FADE3 0xAAAAAAAA
#define COLOR_FADE4 0x8C8C8C8C
#define COLOR_FADE5 0x6E6E6E6E

static Text3D:lakeinfo;
new fishingRodObj[MAX_PLAYERS];
new playerFishTimer[MAX_PLAYERS]; // 钓鱼等待时间

/**
*   创建临时鱼竿(狂野暂时没有相关系统可以获取鱼竿)
**/
stock createTempFishingRod(playerid)
{
    SetPlayerAttachedObject(playerid, 7, 18632, 5, 0.1, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0);
}

stock destroyTempFishingRod(playerid)
{
     Attire.Spawn(playerid);
}

stock startFishingAnim(playerid) {
    ApplyAnimation(playerid, "PAULNMAC", "WANK_LOOP", 3.0,1,0,0,1,0,1);
}

stock stopFishingAnim(playerid) {
    ClearAnimations(playerid);
}

stock SendRangeMessage(Float:range, playerid, const string[], color1, color2, color3, color4, color5)
{
	
    static Float:px, Float:py, Float:pz, Float:distance, pworld;
    GetPlayerPos(playerid, px, py, pz);
    pworld = GetPlayerVirtualWorld(playerid);
    ForEachPlayer(i)
    {
        if(GetPlayerVirtualWorld(i) == pworld)
        {
            distance = GetPlayerDistanceFromPoint(i, px, py, pz);//不用Ex,让Spectate的玩家看得到环境消息以及对话内容
            if(distance < range/16.0) SendClientMessage(i, color1, string);
            else if(distance < range/8.0) SendClientMessage(i, color2, string);
            else if(distance < range/4.0) SendClientMessage(i, color3, string);
            else if(distance < range/2.0) SendClientMessage(i, color4, string);
            else if(distance < range) SendClientMessage(i, color5, string);
        }
    }

	return 1;
}

stock startFishing(playerid) {
    new string[144];
    // if(PlayerHaveItem(playerid, ITEM_USE, 33) == 0){
    //      SendClientMessage(playerid, COLOR_WHITE, "错误: 你没有鱼饵可以钓鱼");  return; 
    //      }
    format(string, sizeof(string), "** %s 虔诚的献上供品向梦月潭开始祈祷 **", GetName(playerid));
    SendRangeMessage(20.0, playerid, string, COLOR_SANDYELLOW, COLOR_SANDYELLOW, COLOR_SANDYELLOW, COLOR_SANDYELLOW, COLOR_SANDYELLOW);
    SendClientMessage(playerid, COLOR_WHITE, "你献上了10个鱼饵开始向梦月潭祈祷,请耐心等待鱼上钩.按"#TEXT_YELLOW"回车键"#TEXT_WHITE"可以收起钓竿");
    new randomHookTime = random(5, 20); // 假设上钩时间在5到20秒之间
    playerFishTimer[playerid] = SetTimer("OnFishHooked", randomHookTime * 1000, false, "i", playerid);
    // GivePlayerItem(playerid, 1, 33, -1);
    // SetGVarInt("Fishing", 18, playerid);
    // SetGVarInt("FishEx", 0, playerid);
}

/**
*   上钩事件
**/ 
forward OnFishHooked(playerid);
public OnFishHooked(playerid)
{
    // 定时器到期，执行上钩代码

    // 在这里可以执行上钩后的逻辑，比如给玩家奖励、发送通知等
    SendClientMessage(playerid, COLOR_WHITE, "你上钩了一条鱼！");
    
    return 1;
}
/**
*   初始化梦月潭
**/

stock MoonLakeInit() {
    new time;
    time = MOOKLAKE_COLD_TIME * 1000 * 60;
    SetTimer("MoonLake", time, 1);
}

/**
*   玩家进入时初始化玩家状态
**/
stock MoonLakePlayerInit(playerid) {
    fishingRodObj[playerid] = -1;
}

/**
*   管理员开启梦月潭
**/ 
CMD:moonlake(playerid, params[])
{
    MoonLake();
    return 1;
}


CMD:fish(playerid, params[])
{
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_BLUE, "[钓鱼] 输入 /fish start 开始钓鱼");
        SCM(playerid, COLOR_BLUE, "[钓鱼] 输入 /fish stop 停止钓鱼");    
        SCM(playerid, COLOR_BLUE, "[钓鱼] 输入 /fish get 收网");    
        return 1;
    }
    if(strcmp(tmp, "start", true) == 0) {
        createTempFishingRod(playerid);
        startFishingAnim(playerid);
        return 1;
    }
    if(strcmp(tmp, "stop", true) == 0) {
        destroyTempFishingRod(playerid);
        stopFishingAnim(playerid);
        return 1;
    }
    if(strcmp(tmp, "get", true) == 0) {
        
        return 1;
    }
    return 1;
}

forward MoonLake();
public MoonLake()
{
    new time;
    time = MOOKLAKE_OPEN_TIME * 1000 * 60;
	SendClientMessageToAll(0x00A2FFFF, "** 传说中的梦月潭显灵了!赶紧前往湖畔献上10个鱼饵,将有机会钓到有价值的宝物!");
    printf("** 传说中的梦月潭显灵了!赶紧前往湖畔献上10个鱼饵,将有机会钓到有价值的宝物!");
    SetGVarInt("MoonLake", 1);
    DestroyDynamic3DTextLabel(lakeinfo);
    lakeinfo = CreateDynamic3DTextLabel("【梦月潭】\n使用钓竿奉献10个鱼饵\n你的愿望便有机会实现", 0x87CEFAFF, -71.1711, -564.9479, 3.1326, 40.0);
    SetTimer("CloseLake", time, 0);
}

forward CloseLake();
public CloseLake()
{
	if(GetGVarInt("MoonLake") == 1)
	{
	    SendClientMessageToAll(0x00A2FFFF, "** 梦月潭已经关闭!没有钓到宝物的玩家请关注下一次消息!");
        printf("** 梦月潭已经关闭!没有钓到宝物的玩家请关注下一次消息!");
	    DestroyDynamic3DTextLabel(lakeinfo);
	    lakeinfo = CreateDynamic3DTextLabel("【梦月潭】", 0x87CEFAFF, -71.1711, -564.9479, 3.1326, 40.0);
	    DeleteGVar("MoonLake");
	}
}