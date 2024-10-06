/*
**ushio_p
**Dylan
**修改请保留ID..
PRAGMA synchronous = FULL; (2)最慢
PRAGMA synchronous = NORMAL; (1)一般
PRAGMA synchronous = OFF; (0)最快
*/
#include <mkstr>

#define COLOR 0x98CDFEFF

new PlayerName[MAX_PLAYERS][32];//玩家名字
new pRaceUnixtimestamp[MAX_PLAYERS];

stock UnixToDate(msg[],unix)//返回unix戳,0表示天数,1表示小时,2表示分钟,3表示秒
{
	new d,h,m;
	while(unix>=60)
	    {
	        unix=unix-60;
	        m++;
	        if(m>=60)
	            {
					m=0;
					h++;
	            }
			if(h>=24)
			    {
			        h=0;
			        d++;
			    }
	    }
	msg[0]=d;
	msg[1]=h;
	msg[2]=m;
	msg[3]=unix;
	return msg;
}

stock MsToS(ms)//返回秒数,将毫秒转换为秒
{
	new s,time[32];
	while(ms>=1000)
	    {
	        ms=ms-1000;
	        s++;
	    }
	if(ms<10)
	    {
	        format(time,32,"%i.00%i",s,ms);
	        return time;
	    }
	if(ms<100)
	    {
	        format(time,32,"%i.0%i",s,ms);
	        return time;
	    }
	if(ms<1000)
	    {
	        format(time,32,"%i.%i",s,ms);
	        return time;
	    }
	return time;
}

stock GetPlayerFacingAngleEx(playerid,&Float:ang)//获取玩家面向，如果在车上则获得汽车
{
	if(GetPlayerVehicleID(playerid)==0)
	    {
	        GetPlayerFacingAngle(playerid,ang);
	    }
	else
	    {
	        GetVehicleZAngle(GetPlayerVehicleID(playerid),ang);
	    }
}

stock SetPlayerFacingAngleEx(playerid,&Float:ang)//设置玩家面向,如果在车上则设置汽车
{
	if(GetPlayerVehicleID(playerid)==0)
	    {
	        SetPlayerFacingAngle(playerid,ang);
	    }
	else
	    {
	        SetVehicleZAngle(GetPlayerVehicleID(playerid),ang);
	    }
}


stock GetPlayerPosEx(playerid,&Float:x,&Float:y,&Float:z)//获取玩家坐标,如果在车上则获取车辆坐标
{
	if(GetPlayerVehicleID(playerid)==0)
	    {
	        GetPlayerPos(playerid,x,y,z);
	    }
	else
	    {
	        GetVehiclePos(GetPlayerVehicleID(playerid),x,y,z);
	    }
}


stock F2S(Float:s)//将一个float转换为字符串
{
	new msg[64];
	format(msg,64,"%f",s);
	return msg;
}

/*赛道相关*/
#define RACE_MAX 520//赛道数量上线,系统提示数量不足时修改
#define RACE_PAGES 20//对话框每页的数量
#define DIALOG_PRACE	(600)
#define RACE_EDIT_MAIN_DIALOG DIALOG_PRACE//编辑赛道的主界面,显示赛道基本信息
#define RACE_EDIT_MIAOSHU_DIALOG RACE_EDIT_MAIN_DIALOG+1//编辑赛道的修改描述界面
#define RACE_EDIT_CP_DIALOG RACE_EDIT_MAIN_DIALOG+2//编辑赛道的CP点操作界面
#define RACE_EDIT_CP_SCRIPT_DIALOG RACE_EDIT_MAIN_DIALOG+3//编辑赛道CP点触发的界面
#define RACE_GAME_MAIN_DIALOG RACE_EDIT_MAIN_DIALOG+4//赛道信息界面
#define RACE_GAME_MAIN_DIALOG_SEL RACE_EDIT_MAIN_DIALOG+5//选择赛道界面
#define RACE_EDIT_CP_SCRIPT_HELP RACE_EDIT_MAIN_DIALOG+6//触发说明界面
#define RACE_EDIT_CP_SCRIPT_HELP_MSG RACE_EDIT_MAIN_DIALOG+7//触发说明界面
#define RACE_CP_MAX_FUNCTION 100//CP点触发每一行的长度
#define RACE_MAP_ICON_TYPE 56//赛车CP点的预先浏览图标
#define RACE_MAP_ICON_INDEX 1//赛车CP点的预先浏览图标索引
#define RACE_GAME_MAX 100 //比赛房间上限
#define CPSCRIPT:%0(%1,%2,%3,%4) \
forward Race_Cp_Script_Function_%0(%1,%2,%3,%4);\
public Race_Cp_Script_Function_%0(%1,%2,%3,%4)

enum racetype{
				rid,
				rname[32],
				rpassword[32],
				rcreatedate,
				rauthor[32],
				rcps,//cp总数
				rstate,//赛道状态 0 - 开启 , 1 - 编辑
				reditplayer,//编辑该赛道的玩家ID
				rmiaoshu[64],
				completeTimes//完成次数
};

enum racetoptype{
					rtop_name[32],
					rtop};

enum raceedittype{
					rraceid,//正在编辑的赛道
					rtestcp,//测试赛道的CP点
					rdialogpage,//编辑赛道dialog框的翻页
					rdialogcp,//选择的CP点
					rdialogcppage,//选择的cp点的触发页数
					rdialogcplist,//选择的cp点的第几条触发
					Float:rcpsize};//cp大小

enum racecptype{
					rcpid,//cp点在数据库中的id
					rrid,//cp点所属赛道的id
					rcp,//表示第几个cp点
					Float:rcpx,//cp x
					Float:rcpy,//cp y
					Float:rcpz,//cp z
					Float:rcpa,//cp a
					Float:rcppsize};//cp大小

enum racegametype{
					rraceid,//这个房间的赛道ID
					rtop,//比赛名次
					rps,//房间内人数
					rtimer,//倒计时的计时器
					rtimes,//倒计时的秒
					rplayerid,//房主ID
					rstate};//房间状态 0 - 准备中 1 - 倒计时 2 - 已经开始

enum raceplayergametype{
							rgameid,//比赛房间的ID
							rtime,//开始时间
							rgamecp};//cp点

enum cpscripttype{
					cpsname[64],//cp触发函数名
					cpsmiaoshu[512]};//cp触发函数的描述

enum cpscriptreturntype{
							cpsrname[64],//cp触发返回函数名
							cpsrmiaoshu[512]};//cp触发返回函数的描述

new cpscriptr[][cpscriptreturntype]={//cp触发返回函数列表
										{"#pname","#pname\n功能\n返回玩家姓名"},
										{"#rname","#rname\n功能\n返回赛道名字"},
										{"#aname","#aname\n功能\n返回赛道作者名字"},
										{"#vs","#vs\n功能\n返回车辆速度(KM/H)"},
										{"#va","#va\n功能\n返回车辆角度"},
										{"#vzs","#vzs\n功能\n返回车辆Z轴速度"},
										{"#ncpx","#ncpx\n功能\n返回下一个检查点的X坐标,如果当前检查点是最后一个,则返回0"},
										{"#ncpy","#ncpy\n功能\n返回下一个检查点的Y坐标,如果当前检查点是最后一个,则返回0"},
										{"#ncpz","#ncpz\n功能\n返回下一个检查点的Z坐标,如果当前检查点是最后一个,则返回0"},
										{"#ncpa","#ncpa\n功能\n返回当前检查点与下一个检查点的角度,如果当前检查点是最后一个检查点则返回0"}};
new cpscript[][cpscripttype]={//cp触发函数列表
								{"msg","msg\n功能\n向玩家发送一条消息\n用法\nmsg 内容1 内容2 内容3 内容4 内容5 ....\n例:msg 啊啊啊啊哦哦哦\n例:msg 两只老虎 两只老虎 跑得快 跑得快~ ~"},
								{"speed","speed\n功能\n设置车辆速度\n用法\nspeed 角度运算模式 角度 速度运算模式 速度(KM/H)\n参数不能为空,否则报错\n角度/速度运算模式的可选值为\n| > 设置角/速度\n+ > 与当前角/速度进行加运算\n- > 与当前角/速度进行减运算\n* > 与当前角/速度进行乘运算\n/ > 与当前角/速度进行除运算\n不符合+-*/的参数会被重置为|"},
								{"angle","angle\n功能\n设置车辆角度\n用法\nangle 角度运算模式 角度\n参数不能为空，否则报错\n角度运算模式的可选值为\n | > 设置角度\n + > 与当前车辆角度进行加计算\n - > 与当前车辆角度进行减计算\n * > 与当前车辆角度进行乘计算\n / > 与当前车辆角度进行除计算\n不符合+-*/的参数会被重置为|"},
								{"zspeed","zspeed\n功能\n设置车辆Z轴速度\n用法\nzspeed 速度运算模式 速度\n参数不能为空，否则报错\n速度运算模式的可选值为\n | > 设置Z轴速度\n + > 与当前车辆Z轴速度进行加计算\n - > 与当前车辆Z轴速度进行减计算\n * > 与当前车辆Z轴速度进行乘计算\n / > 与当前车辆Z轴速度进行除计算\n不符合+-*/的参数会被重置为|"},
								{"speedex","speedex\n功能\n设置车辆速度的同时设置车辆Z轴速度\n用法\nspeedex 角度运算模式 角度 速度运算模式 速度(KM/H) Z轴速度运算模式 Z轴速度\n参数不能为空,否则报错\n运算模式的可选值为\n| >设置角/速/Z轴度\n+ >与当前角/速/Z轴度进行加运算\n- >与当前角/速/Z轴度进行减运算\n* >与当前角/速/Z轴度进行乘运算\n/ >与当前角/速/Z轴度进行除运算\n不符合+-*/的参数会被重置为|"},
								{"vgoto","vgoto\n功能\n传送汽车到指定坐标\n用法\nvgoto 执行模式 x坐标 y坐标 z坐标\n参数不能为空,否则报错\n执行模式的可选值为\n s > 传送到指定坐标,但不保留速度\n v > 传送到指定坐标并保留传送前的速度\n不符合s的参数会被重置为v"},
								{"fix","fix\n功能\n修复汽车\n用法\nfix 执行模式\n参数不能为空，否则报错\n执行模式的可选值为\n f > 仅修复车辆HP,不修复外观(比如轮胎)\n r > 修复车辆外观与HP(比如轮胎)\n不符合r的参数会重置为f"},
								{"damage","damage\n功能\n对汽车轮胎造成伤害\n用法\ndamage 破坏模式\n参数不能为空,否则报错\n破坏模式的可选值为\n0>无伤害\n1>右后轮胎破坏\n2>右前轮胎破坏\n3>右前/右后轮胎破坏\n4>左后轮胎破坏\n5>左后/右后轮胎破坏\n6>左后/右前轮胎破坏\n7>左后/右前/右后轮胎破坏\8>左前轮胎破坏\n9>左前/右后轮胎破坏\n10>左前/右前轮胎破坏\n11>左前/右前/右后轮胎破坏\n12>左前/左后轮胎破坏\n13>左前/左后/右后轮胎破坏\n14>左前/左后/右前轮胎破坏\n15>全部轮胎破坏\n其他值会被重置为0"}};
new Race[RACE_MAX][racetype];//赛道
new RaceTop[RACE_MAX][10][racetoptype];//赛道记录
new EditRace[MAX_PLAYERS][raceedittype];//编辑赛道
new GameRace[MAX_PLAYERS][raceplayergametype];//玩家比赛房间
new RaceHouse[RACE_GAME_MAX][racegametype];//比赛房间
new SelectRacePage[MAX_PLAYERS];//选择赛道页
new PlayerText:CpTextDraw[MAX_PLAYERS];//显示CP点的TEXTDRAW
new RaceEd;//脚本是否执行
new NowRaceId;//当前赛道ID索引

stock PRaceGetRandomRaceName() {
	new name[64];
	format(name, sizeof name, "%s", Race[random(NowRaceId)][rname]);
	return name;
}

stock Race_Cp_Script_Return(playerid,raceid,cpid,str[])//查询指定返回函数是否存在,如果存在则执行并返回结果
{
	new id=-1,msg[128];
	for(new i=0;i<sizeof(cpscriptr);i++)
	    {
	        if(mk_strcmp(str,cpscriptr[i][cpsrname])==0)
	            {
	                id=i;
	                i=sizeof(cpscriptr)+1;
	            }
	    }
	if(id==-1)
	    {
	        format(msg,128,"%s",str);
	        return msg;
	    }
	if(mk_strcmp(str,"#pname")==0)//返回姓名
	    {
	        format(msg,128,"%s",PlayerName[playerid]);
	        return msg;
	    }
	if(mk_strcmp(str,"#rname")==0)//返回赛道名
	    {
     		format(msg,128,"%s",Race[raceid][rname]);
	        return msg;
	    }
	if(mk_strcmp(str,"#aname")==0)//返回赛道名
	    {
     		format(msg,128,"%s",Race[raceid][rauthor]);
	        return msg;
	    }
	if(mk_strcmp(str,"#vs")==0)//返回车辆速度
	    {
			format(msg,128,"%f",GetVehicleSpeed(GetPlayerVehicleID(playerid)));
			return msg;
	    }
	if(mk_strcmp(str,"#va")==0)//返回车辆角度
	    {
	        new Float:a;
	        GetPlayerFacingAngleEx(playerid,a);
	        format(msg,128,"%f",a);
	        return msg;
	    }
	if(mk_strcmp(str,"#vzs")==0)//返回车辆角度
	    {
	        new Float:x,Float:y,Float:z;
	        GetVehicleVelocity(GetPlayerVehicleID(playerid),x,y,z);
	        format(msg,128,"%f",z);
	        return msg;
	    }
	if(mk_strcmp(str,"#ncpa")==0)//返回下一个检查点角度
	    {
	        if(cpid==Race[raceid][rcps])
	            {
	                format(msg,128,"0");
	                return msg;
				}
			new trcp[racecptype],nrcp[racecptype];
			Race_GetCp(raceid,cpid,trcp);
			Race_GetCp(raceid,cpid+1,nrcp);
			new Float:a = atan2(trcp[rcpy] - nrcp[rcpy],trcp[rcpx] - nrcp[rcpx]);
			if(a>360) a-=360;
			a=90+a;
			format(msg,128,"%f",a);
			return msg;
	    }
	if(mk_strcmp(str,"#ncpx")==0)//返回下一个检查点X
	    {
	        if(cpid==Race[raceid][rcps])
	            {
	                format(msg,128,"0");
	                return msg;
				}
			new trcp[racecptype];
			Race_GetCp(raceid,cpid+1,trcp);
			format(msg,128,"%f",trcp[rcpx]);
			return msg;
	    }
	if(mk_strcmp(str,"#ncpy")==0)//返回下一个检查点Y
	    {
	        if(cpid==Race[raceid][rcps])
	            {
	                format(msg,128,"0");
	                return msg;
				}
			new trcp[racecptype];
			Race_GetCp(raceid,cpid+1,trcp);
			format(msg,128,"%f",trcp[rcpy]);
			return msg;
	    }
	if(mk_strcmp(str,"#ncpz")==0)//返回下一个检查点z
	    {
	        if(cpid==Race[raceid][rcps])
	            {
	                format(msg,128,"0");
	                return msg;
				}
			new trcp[racecptype];
			Race_GetCp(raceid,cpid+1,trcp);
			format(msg,128,"%f",trcp[rcpz]);
			return msg;
	    }
	return msg;
}

CPSCRIPT:damage(playerid,raceid,cpid,script[])//cp触发 轮胎破坏
{
    if(GetPlayerVehicleID(playerid)==0) return 1;
    new tmp[128],idx;
    tmp=strtok(script,idx);
	tmp=strtok(script,idx);//破坏模式
	if(mk_strcmp(tmp,"")==0)
	    {
     		format(tmp,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:damage 破坏模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,tmp);
	        return 1;
	    }
    new panels, doors, lights, tires,vid=GetPlayerVehicleID(playerid);
	GetVehicleDamageStatus(vid, panels, doors, lights, tires);
	UpdateVehicleDamageStatus(vid, panels, doors, lights,strval(Race_Cp_Script_Return(playerid,raceid,cpid,tmp)));
	return 1;
}

CPSCRIPT:fix(playerid,raceid,cpid,script[])//cp触发 修复汽车
{
    if(GetPlayerVehicleID(playerid)==0) return 1;
    new tmp[128],idx;
    tmp=strtok(script,idx);
	tmp=strtok(script,idx);//执行模式
	if(mk_strcmp(tmp,"")==0)
	    {
     		format(tmp,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:fix 执行模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,tmp);
	        return 1;
	    }
	new b=-1;
	if(mk_strcmp(tmp,"r")==0) b=1;
	if(b==-1)
	    {
	        SetVehicleHealth(GetPlayerVehicleID(playerid),1000);
	    }
	else
	    {
	   		RepairVehicle(GetPlayerVehicleID(playerid));
	    }
	return 1;
}

CPSCRIPT:vgoto(playerid,raceid,cpid,script[])//cp触发 传送汽车
{
	if(GetPlayerVehicleID(playerid)==0) return 1;
	new tmp[128],msg[128],tmp1[128],idx;
	tmp=strtok(script,idx);
	tmp=strtok(script,idx);//执行模式
	if(mk_strcmp(tmp,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:vgoto 执行模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp1=strtok(script,idx);//x坐标
	if(mk_strcmp(tmp1,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:vgoto x坐标不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new tmp2[128];
	tmp2=strtok(script,idx);//y坐标
	if(mk_strcmp(tmp2,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:vgoto y坐标不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new tmp3[128];
	tmp3=strtok(script,idx);//z坐标
	if(mk_strcmp(tmp3,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:vgoto z坐标不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new b=-1,Float:nx,Float:ny,Float:nz;
	nx=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp1));
	ny=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp2));
	nz=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp3));
	if(mk_strcmp(tmp,"s")==0) b=1;
	if(b==1)
	    {
     		SetPlayerPosEx(playerid,nx,ny,nz);
     	}
	else
	    {
  	      	new Float:vx,Float:vz,Float:vy;
	        GetVehicleVelocity(GetPlayerVehicleID(playerid),vx,vy,vz);
	        SetPlayerPosEx(playerid,nx,ny,nz);
	        SetVehicleVelocity(GetPlayerVehicleID(playerid),vx,vy,vz);
	    }
	return 1;
}

CPSCRIPT:zspeed(playerid,raceid,cpid,script[])//cp触发 设置车辆Z轴速度
{
	if(GetPlayerVehicleID(playerid)==0) return 1;
	new tmp[128],msg[128],tmp1[128],idx;
	tmp=strtok(script,idx);
	tmp=strtok(script,idx);//速度运算模式
	if(mk_strcmp(tmp,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:zspeed 速度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp1=strtok(script,idx);//速度
	if(mk_strcmp(tmp1,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:zspeed 速度不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new Float:z,Float:nz,b=-1,Float:nx,Float:ny;
	z=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp1));
	GetVehicleVelocity(GetPlayerVehicleID(playerid),nx,ny,nz);
	if(mk_strcmp(tmp,"+")==0)	nz=nz+z,b=1;
	if(mk_strcmp(tmp,"-")==0)  nz=nz-z,b=1;
	if(mk_strcmp(tmp,"*")==0)  nz=nz*z,b=1;
	if(mk_strcmp(tmp,"/")==0)  nz=nz/z,b=1;
	if(b==-1) nz=z;
	SetVehicleVelocity(GetPlayerVehicleID(playerid),nx,ny,nz);
	return 1;
}

CPSCRIPT:angle(playerid,raceid,cpid,script[])//cp触发 设置车辆角度
{
	if(GetPlayerVehicleID(playerid)==0) return 1;
	new tmp[128],msg[128],tmp1[128],idx;
	tmp=strtok(script,idx);
	tmp=strtok(script,idx);//角度运算模式
	if(mk_strcmp(tmp,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:angle 角度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp1=strtok(script,idx);//角度
	if(mk_strcmp(tmp1,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:angle 角度不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new Float:a,Float:na,b=-1;
	a=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp1));
	GetPlayerFacingAngleEx(playerid,na);
	if(mk_strcmp(tmp,"+")==0)	na=na+a,b=1;
	if(mk_strcmp(tmp,"-")==0)  na=na-a,b=1;
	if(mk_strcmp(tmp,"*")==0)  na=na*a,b=1;
	if(mk_strcmp(tmp,"/")==0)  na=na/a,b=1;
	if(b==-1) na=a;
	SetPlayerFacingAngleEx(playerid,na);
	return 1;
}

CPSCRIPT:speedex(playerid,raceid,cpid,script[])//cp触发 设置车辆速度的同时设置Z轴速度
{
	if(GetPlayerVehicleID(playerid)==0) return 1;
	new msg[128],tmp[128],tmp1[128],tmp2[128],tmp3[128],idx,Float:a,Float:speed,vid=GetPlayerVehicleID(playerid);
	tmp=strtok(script,idx);
	tmp=strtok(script,idx);//角度运算模式
	if(mk_strcmp(tmp,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex 角度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp1=strtok(script,idx);//角度
	if(mk_strcmp(tmp1,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex 角度不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp2=strtok(script,idx);//速度运算模式
	if(mk_strcmp(tmp2,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex 速度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp3=strtok(script,idx);//速度
	if(mk_strcmp(tmp3,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex 速度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new tmp4[128];
	tmp4=strtok(script,idx);//Z轴运算模式
	if(mk_strcmp(tmp4,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex Z轴运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new tmp5[128];
	tmp5=strtok(script,idx);//Z轴速度
	if(mk_strcmp(tmp5,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speedex Z轴速度不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	new Float:z;
	a=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp1));
	speed=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp3));
	z=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp5));
	new Float:na,Float:nspeed,b=-1;
	new Float:x,Float:y,Float:nz;
	GetVehicleVelocity(vid,x,y,nz);
	GetPlayerFacingAngleEx(playerid,na);
	na=na+90;
	nspeed=GetVehicleSpeed(vid);
	if(mk_strcmp(tmp,"+")==0)	na=na+a,b=1;
	if(mk_strcmp(tmp,"-")==0)  na=na-a,b=1;
	if(mk_strcmp(tmp,"*")==0)  na=na*a,b=1;
	if(mk_strcmp(tmp,"/")==0)  na=na/a,b=1;
	if(b==-1) na=a+90;
	b=-1;
	if(mk_strcmp(tmp2,"+")==0)  nspeed=nspeed+speed,b=1;
	if(mk_strcmp(tmp2,"-")==0)  nspeed=nspeed-speed,b=1;
	if(mk_strcmp(tmp2,"*")==0)  nspeed=nspeed*speed,b=1;
	if(mk_strcmp(tmp2,"/")==0)  nspeed=nspeed/speed,b=1;
	if(b==-1) nspeed=speed;
	b=-1;
	if(mk_strcmp(tmp4,"+")==0)  nz=nz+z,b=1;
	if(mk_strcmp(tmp4,"-")==0)  nz=nz-z,b=1;
	if(mk_strcmp(tmp4,"*")==0)  nz=nz*z,b=1;
	if(mk_strcmp(tmp4,"/")==0)  nz=nz/z,b=1;
	if(b==-1) nz=z;
	nspeed=nspeed*0.005556;
	x=nspeed*floatcos(na,degrees);
	y=nspeed*floatsin(na,degrees);
	SetVehicleVelocity(vid,x,y,nz);
	return 1;
}

CPSCRIPT:speed(playerid,raceid,cpid,script[])//cp触发 设置车辆速度
{
	if(GetPlayerVehicleID(playerid)==0) return 1;
	new msg[128],tmp[128],tmp1[128],tmp2[128],tmp3[128],idx,Float:a,Float:speed,vid=GetPlayerVehicleID(playerid);
	tmp=strtok(script,idx);
	tmp=strtok(script,idx);//角度运算模式
	if(mk_strcmp(tmp,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speed 角度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp1=strtok(script,idx);//角度
	if(mk_strcmp(tmp1,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speed 角度不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp2=strtok(script,idx);//速度运算模式
	if(mk_strcmp(tmp2,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speed 速度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	tmp3=strtok(script,idx);//速度
	if(mk_strcmp(tmp3,"")==0)
	    {
	        format(msg,128,"[赛车]赛道[%s]第[%i]个检查点触发错误:speed 速度运算模式不能为空",Race[raceid][rname],cpid);
	        SendClientMessage(playerid,COLOR,msg);
	        return 1;
	    }
	a=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp1));
	speed=floatstr(Race_Cp_Script_Return(playerid,raceid,cpid,tmp3));
	new Float:na,Float:nspeed,b=-1;
	GetPlayerFacingAngleEx(playerid,na);
	na=na+90;
	nspeed=GetVehicleSpeed(vid);
	if(mk_strcmp(tmp,"+")==0)	na=na+a,b=1;
	if(mk_strcmp(tmp,"-")==0)  na=na-a,b=1;
	if(mk_strcmp(tmp,"*")==0)  na=na*a,b=1;
	if(mk_strcmp(tmp,"/")==0)  na=na/a,b=1;
	if(b==-1) na=a+90;
	b=-1;
	if(mk_strcmp(tmp2,"+")==0)  nspeed=nspeed+speed,b=1;
	if(mk_strcmp(tmp2,"-")==0)  nspeed=nspeed-speed,b=1;
	if(mk_strcmp(tmp2,"*")==0)  nspeed=nspeed*speed,b=1;
	if(mk_strcmp(tmp2,"/")==0)  nspeed=nspeed/speed,b=1;
	if(b==-1) nspeed=speed;
	new Float:x,Float:y,Float:z;
	GetVehicleVelocity(vid,x,y,z);
	nspeed=nspeed*0.005556;
	x=nspeed*floatcos(na,degrees);
	y=nspeed*floatsin(na,degrees);
	SetVehicleVelocity(vid,x,y,z);
	return 1;
}

CPSCRIPT:msg(playerid,raceid,cpid,script[])//cp触发 显示消息
{
	new msg[256],idx,tmp[128];
	tmp=strtok(script,idx);
	while(mk_strcmp(tmp,"")!=0)
		{
            tmp=strtok(script,idx);
			if(mk_strcmp(tmp,"")!=0)
		        {
		            format(msg,256,"%s%s",msg,Race_Cp_Script_Return(playerid,raceid,cpid,tmp));
		        }
		}
	SendClientMessage(playerid,COLOR,msg);
}

stock Race_Cp_Script_Check(funname[])//查询指定函数是否存在
{
	new s=-1;
	for(new i=0;i<sizeof(cpscript);i++)
	    {
	        if(mk_strcmp(funname,cpscript[i][cpsname])==0)
	            {
	                s=i;
	                i=sizeof(cpscript)+1;
	            }
	    }
	return s;
}

stock Race_Cp_Script_Start(playerid,raceid,cpid)//为指定玩家执行指定赛道指定cp点的触发
{

	new trcp[racecptype];
	Race_GetCp(raceid,cpid,trcp);
	new ss=Race_GetCp_Scripts(trcp[rcpid]),script[RACE_CP_MAX_FUNCTION],idx,tmp[128],msg[256];
	for(new i=0;i<ss;i++)
	    {
	        Race_GetCp_Script(trcp[rcpid],i,script);
	        format(script,RACE_CP_MAX_FUNCTION,"%s",script);
			tmp=strtok(script,idx);
			if(Race_Cp_Script_Check(tmp)!=-1)
			    {
			        format(msg,256,"Race_Cp_Script_Function_%s",tmp);
			        CallLocalFunction(msg,"iiis",playerid,raceid,cpid,script);
			    }
			else
			    {
					format(msg,256,"[赛车]赛道[%s]的第[%i]个检查点触发第[%i]条错误",Race[raceid][rname],cpid,i);
					SendClientMessage(playerid,COLOR,msg);
					format(msg,256,"[赛车]不存在函数[%s]",tmp);
					SendClientMessage(playerid,COLOR,msg);
			    }
			idx=0;
	    }
}

stock Race_Init()//初始化赛道变量
{
    NowRaceId=0;
	for(new i=0;i<RACE_MAX;i++)
	    {
	        Race[i][rid]=-1;
	        Race[i][reditplayer]=-1;
	        for(new t=0;t<10;t++)
	            {
	                format(RaceTop[i][t][rtop_name],32,"无");
	        		RaceTop[i][t][rtop]=-1;
				}
		}
}

stock Race_Game_Init()//初始化房间变量
{
	for(new i=0;i<RACE_GAME_MAX;i++)
	    {
	        RaceHouse[i][rraceid]=-1;
	        RaceHouse[i][rtop]=1;
	        RaceHouse[i][rstate]=0;
	        RaceHouse[i][rps]=0;
	        RaceHouse[i][rtimer]=-1;
	        RaceHouse[i][rtimes]=5;
	        RaceHouse[i][rplayerid]=-1;
	    }
}

stock Race_Game_End(playerid,raceid,time)//指定玩家完成指定赛道,刷新记录返回记录,没刷新返回-1
{
	new ttime,id=-1;//,ti=GetTickCount();
	for(new i=0;i<10;i++) {
		if(RaceTop[raceid][i][rtop]==-1) {
			ttime=999999999;
		}	else {
			ttime=RaceTop[raceid][i][rtop];
		}
		if(time<ttime) {
			id=i;
			i=11;
		}
	}
	if(id==-1) return id;
	new tpm[1024];
	cache_delete(mysql_query(mariadb, "begin"));
	for(new i=9;i>id;i--) {
		RaceTop[raceid][i][rtop]=RaceTop[raceid][i-1][rtop];
		format(RaceTop[raceid][i][rtop_name],32,"%s",RaceTop[raceid][i-1][rtop_name]);
		format(tpm,1024,"UPDATE race SET top%i_name = '%s' ,top%i='%i' WHERE rname = '%s'",i+1,RaceTop[raceid][i][rtop_name],i+1,RaceTop[raceid][i][rtop],Race[raceid][rname]);
		cache_delete(mysql_query(mariadb, tpm));
	}
	RaceTop[raceid][id][rtop]=time;
	format(RaceTop[raceid][id][rtop_name],32,"%s",PlayerName[playerid]);
	format(tpm,1024,"UPDATE race SET top%i_name = '%s' ,top%i='%i' WHERE rname = '%s'",id+1,RaceTop[raceid][id][rtop_name],id+1,RaceTop[raceid][id][rtop],Race[raceid][rname]);
	cache_delete(mysql_query(mariadb, tpm));
	cache_delete(mysql_query(mariadb, "commit"));
	//printf(tpm);
	//printf("  >P-Race 更新赛道[%s]的记录,耗时:%ims",Race[raceid][rname],GetTickCount()-ti);
	return id;
}

forward Race_Game_Start_s(houseid);//倒计时某个比赛房间
public Race_Game_Start_s(houseid)
{
	new msg[128];
	if(RaceHouse[houseid][rstate]==1)
	    {
			if(RaceHouse[houseid][rtimes]==0)
			    {
			        KillTimer(RaceHouse[houseid][rtimer]);
			        RaceHouse[houseid][rstate]=2;
			        //format(msg,128,"[赛车]比赛开始!gogogogo!");
			        for(new i=0;i<MAX_PLAYERS;i++)
			            {
			                if(IsPlayerConnected(i)==1)
			                    {
			                        if(GameRace[i][rgameid]==houseid)
			                            {
		                                    GameTextForPlayer(i,"~b~gogogo!",850,3);
		                            		Race_ShowCp(i,RaceHouse[houseid][rraceid],GameRace[i][rgamecp]);
			                                GameRace[i][rtime]=GetTickCount();
			                            }
			                    }
			            }
					//Race_Game_Msg(houseid,msg);
			    }
			else
			    {
			        format(msg,128,"~b~%i",RaceHouse[houseid][rtimes]);
					//Race_Game_Msg(houseid,msg);
					for(new i=0;i<MAX_PLAYERS;i++)
					    {
					        if(IsPlayerConnected(i)==1)
					            {
		           					if(GameRace[i][rgameid]==houseid)
			                            {
			                                GameTextForPlayer(i,msg,850,3);
			                            }
					            }
					    }
					RaceHouse[houseid][rtimes]--;
			    }
		}
}

stock Race_Game_Start(houseid)//使指定房间开始比赛
{
	RaceHouse[houseid][rstate]=1;
	RaceHouse[houseid][rtimes]=5;
	RaceHouse[houseid][rtimer]=SetTimerEx("Race_Game_Start_s",850,1,"i",houseid);
	Race_Game_Msg(houseid,"[赛车]倒计时开始...");
}

stock Race_Game_Msg(houseid,str[])//向指定房间的玩家发送消息
{
	for(new i=0;i<MAX_PLAYERS;i++)
	    {
	        if(IsPlayerConnected(i)==1)
	            {
					if(GameRace[i][rgameid]==houseid)
					    {
					        SendClientMessage(i,COLOR,str);
					    }
	            }
	    }
}

stock Race_Game_Quit(playerid)//使指定玩家退出房间
{
	new hid=GameRace[playerid][rgameid];
	GameRace[playerid][rgameid]=-1;
	GameRace[playerid][rtime]=0;
	GameRace[playerid][rgamecp]=1;
	PlayerTextDrawHide(playerid,CpTextDraw[playerid]);
	Race_HideCp(playerid);
	RaceHouse[hid][rps]--;
	if(RaceHouse[hid][rps]==0)
	    {
	        Race_Game_Delete(hid);
	        return 1;
	    }
	if(playerid==RaceHouse[hid][rplayerid]&&RaceHouse[hid][rstate]==0)
	    {
	        Race_Game_Msg(hid,"[赛车]赛车发起者离开了房间,因此房间将被关闭");
	        Race_Game_Delete(hid);
	        return 1;
	    }
	return 1;
}

stock Race_Game_Join(playerid,houseid)//使指定玩家加入指定房间.失败返回-1
{
	if(RaceHouse[houseid][rraceid]==-1) return -1;
	if(RaceHouse[houseid][rstate]!=0) return -1;
	new msg[128];
	format(msg,128,"[赛车] %s 加入了比赛",PlayerName[playerid]);
 	Race_Game_Msg(houseid,msg);
	RaceHouse[houseid][rps]++;
	GameRace[playerid][rgameid]=houseid;
	GameRace[playerid][rtime]=0;
	GameRace[playerid][rgamecp]=1;
	pRaceUnixtimestamp[playerid] = 0;
	new trcp[racecptype];
	Race_GetCp(RaceHouse[houseid][rraceid],GameRace[playerid][rgamecp],trcp);
	SetPlayerPosEx(playerid,trcp[rcpx],trcp[rcpy],trcp[rcpz]);
	SetPlayerFacingAngle(playerid,trcp[rcpa]);
	format(msg,128,"CP:0/%i",Race[RaceHouse[houseid][rraceid]][rcps]);
	PlayerTextDrawSetString(playerid,CpTextDraw[playerid],msg);
	PlayerTextDrawShow(playerid,CpTextDraw[playerid]);
	Race_ShowCp(playerid,RaceHouse[houseid][rraceid],GameRace[playerid][rgamecp]);
	return 1;
}

stock Race_Game_DeleteByRid(raceid)//关闭指定赛道的比赛房间
{
	for(new i=0;i<RACE_GAME_MAX;i++)
	    {
	        if(RaceHouse[i][rraceid]==raceid)
	            {
	            	Race_Game_Msg(i,"[赛车]赛道被编辑,因此房间将被关闭");
	                Race_Game_Delete(i);
	            }
	    }
}

stock Race_Game_Delete(houseid)//关闭比赛房间
{
//	printf("关闭房间:%i",houseid);
	for(new i=0;i<MAX_PLAYERS;i++)
	    {
	        if(IsPlayerConnected(i)==1)
	            {
	                if(GameRace[i][rgameid]==houseid)
	                    {
							Race_Game_Quit(i);
	                    }
	            }
	    }
	RaceHouse[houseid][rraceid]=-1;
	RaceHouse[houseid][rplayerid]=-1;
	if(RaceHouse[houseid][rtimer]!=-1)
	    {
	        //printf("Delete:Killtimer");
	        KillTimer(RaceHouse[houseid][rtimer]);
	        RaceHouse[houseid][rtimer]=-1;
	    }
}

stock Race_Game_Create(playerid,raceid)//创建比赛房间，赛道为raceid,失败返回-1,成功返回房间ID
{
	new id=-1;
	for(new i=0;i<RACE_GAME_MAX;i++)
	    {
	        if(RaceHouse[i][rraceid]==-1)
	            {
	                id=i;
	                i=RACE_GAME_MAX+1;
	            }
	    }
	if(id==-1) return id;
	RaceHouse[id][rraceid]=raceid;
	RaceHouse[id][rtop]=1;
	RaceHouse[id][rstate]=0;
	RaceHouse[id][rps]=0;
	RaceHouse[id][rtimer]=-1;
	RaceHouse[id][rtimes]=5;
	RaceHouse[id][rplayerid]=playerid;
//	printf("创建房间:%i",id);
	return id;
}

stock Race_MaxPage(s)//给定总数,求出页数
{
	new maxpage;
 	maxpage=s%RACE_PAGES;
  	if(maxpage==0)
   		{
     		maxpage=s/RACE_PAGES;
       	}
	else
 		{
   			maxpage=(s/RACE_PAGES)+1;
   		}
	return maxpage;
}

stock Race_ShowTrgFunDialog(playerid,fun[])//为指定玩家显示触发函数说明
{
	new id=-1;
	for(new i=0;i<sizeof(cpscript);i++)
	    {
	        if(mk_strcmp(cpscript[i][cpsname],fun)==0)
	            {
	                id=i;
	                i=sizeof(cpscript)+1;
	            }
	    }
	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP_MSG,DIALOG_STYLE_MSGBOX,"触发说明",cpscript[id][cpsmiaoshu],"关闭","");
}

stock Race_ShowTrgRetDialog(playerid,fun[])//为指定玩家显示触发转义符说明
{
	new id=-1;
	for(new i=0;i<sizeof(cpscriptr);i++)
	    {
	        if(mk_strcmp(cpscriptr[i][cpsrname],fun)==0)
	            {
	                id=i;
	                i=sizeof(cpscriptr)+1;
	            }
	    }
	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP_MSG,DIALOG_STYLE_MSGBOX,"触发说明",cpscriptr[id][cpsrmiaoshu],"关闭","");
}

stock Race_ShowTrgDialog(playerid,type[])//为指定玩家显示触发说明
{
	new msg[1024];
	if(mk_strcmp(type,"main")==0)
	    {
	    	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP,DIALOG_STYLE_LIST,"触发说明","查看语法\n查看函数\n查看转义符","选择","关闭");
	    }
	if(mk_strcmp(type,"语法")==0)
	    {
	        ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP,DIALOG_STYLE_MSGBOX,"触发说明","所有触发的用法都是\n函数 参数 参数1 参数2 参数3 参数4 参数......\n函数、参数之间利用空格分割\n参数可以是转义符或任意字符,根据函数不同执行效果不同\n{00FF00}注意:{FF0000}不{FFFFFF}支持数字计算,比如speed 2+5,不会设置为7!\n需要进行计算的函数会有参数决定运算方式,请查看对应函数的描述\n{00FF00}注意2:{FFFFFF}所有触发只有{00FF00}主驾驶{FFFFFF}的玩家可以触发","关闭","");
	    }
	if(mk_strcmp(type,"函数")==0)
	    {
	        for(new i=0;i<sizeof(cpscript);i++)
	            {
	                format(msg,1024,"%s\n函数> %s",msg,cpscript[i][cpsname]);
	            }
         	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP,DIALOG_STYLE_LIST,"触发说明",msg,"选择","关闭");
	    }
	if(mk_strcmp(type,"转义符")==0)
	    {
			for(new i=0;i<sizeof(cpscriptr);i++)
	            {
	                format(msg,1024,"%s\n转义符> %s",msg,cpscriptr[i][cpsrname]);
	            }
         	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_HELP,DIALOG_STYLE_LIST,"触发说明",msg,"选择","关闭");
	    }
}

//为指定玩家显示赛道选择菜单,page表示赛道页数 
stock Race_ShowGameSelDialog(playerid,page) {
	new title[128], msg[2048], maxpage;
	if(NowRaceId!=0) {
	    if(NowRaceId>RACE_PAGES) {
			maxpage=Race_MaxPage(NowRaceId);
		}
		if(page!=1) {
			format(msg, sizeof msg,"上一页");
		}
	    for(new i=(page-1)*RACE_PAGES;i<page*RACE_PAGES;i++) {
	        if(i<NowRaceId&&Race[i][rid]!=-1) {
	            format(msg, sizeof msg,"%s\n赛道 {98CDFE}%s {FFFFFF}作者:%s 完成次数:%d",msg,Race[i][rname],Race[i][rauthor], Race[i][completeTimes]);
			}
		}
		if(page!=maxpage) {
    		format(msg, sizeof msg,"%s\n下一页", msg);
		}
	}
	format(title, sizeof title, "选择您想玩的赛道吧... 当前第 %i 页 - 共 %i 页", page, maxpage);
	ShowPlayerDialog(playerid,RACE_GAME_MAIN_DIALOG_SEL,DIALOG_STYLE_LIST,title,msg,"选择","关闭");
}



stock Race_ShowGameDialog(playerid,id)//为指定玩家显示指定赛道的详细属性
{
	new title[128],msg[1024];
	format(title,128,"赛道 - %s",Race[id][rname]);
	format(msg,1024,"赛道作者:%s\n赛道描述\n%s\n赛道CP数量:%i\n \n{00FF00}开始 %s",Race[id][rauthor],Race[id][rmiaoshu],Race[id][rcps],Race[id][rname]);
	if(RaceTop[id][0][rtop]!=-1)
	    {
	        format(msg,1024,"%s\n \n-Top10:",msg);
		 	for(new i=0;i<10;i++)
		  		{
		    		if(RaceTop[id][i][rtop]!=-1)
		      			{
		       				format(msg,1024,"%s\n#%i:%s 耗时:%s秒",msg,i+1,RaceTop[id][i][rtop_name],MsToS(RaceTop[id][i][rtop]));
						}
			    }
		}
	ShowPlayerDialog(playerid,RACE_GAME_MAIN_DIALOG,DIALOG_STYLE_LIST,title,msg,"选择","关闭");
}

stock Race_ShowEditDialog(playerid,id,page)//为指定玩家显示指定赛道的详细属性,page表示检查点页数
{
	new title[128],msg[1024],maxpage,ps=-1;
	format(title,128,"赛道编辑 - %s",Race[id][rname]);
	format(msg,1024,"赛道作者:%s\n赛道描述 < 点击这行来修改描述\n%s\n赛道CP数量:%i\n测试该赛道\n清空赛道记录\n{00FF00}删除该赛道{FFFFFF}\n修改所有检查点尺寸为 %f",Race[id][rauthor],Race[id][rmiaoshu],Race[id][rcps],EditRace[playerid][rcpsize]);
	if(Race[id][rcps]!=0)
	    {
	        if(Race[id][rcps]>RACE_PAGES)
	            {
					maxpage=Race_MaxPage(Race[id][rcps]);
					if(page==maxpage)
					    {
    						format(msg,1024,"%s\n- 第 %i 页 - 共 %i 页 -",msg,page,maxpage);
					    }
					else
					    {
    						format(msg,1024,"%s\n- 第 %i 页 - 共 %i 页 -\n下一页",msg,page,maxpage);
					    }
					if(page!=1)
					    {
					        ps=1;
						}
				}
			else
			    {
					format(msg,1024,"%s\n-",msg);
			    }
	        for(new i=(page-1)*RACE_PAGES;i<page*RACE_PAGES;i++)
	            {
	                if(i<Race[id][rcps])
	                    {
	                		format(msg,1024,"%s\n检查点 %i",msg,i+1);
						}
				}
			if(ps==1)
			    {
			        format(msg,1024,"%s\n上一页",msg);
			    }
	    }
	ShowPlayerDialog(playerid,RACE_EDIT_MAIN_DIALOG,DIALOG_STYLE_LIST,title,msg,"选择","关闭");
}

stock Race_ShowEditMiaoshuDialog(playerid,id,tip[])//为指定玩家显示指定赛道的描述修改界面
{
	new msg[768],title[128];
	format(title,128,"赛道编辑 - %s - 修改描述",Race[id][rname]);
	format(msg,768,"当前赛道[%s]的描述\n{FF0000}%s{FFFFFF}\n请输入要修改的描述~最多64个字符(中文1个算2个)%s",Race[id][rname],Race[id][rmiaoshu],tip);
	ShowPlayerDialog(playerid,RACE_EDIT_MIAOSHU_DIALOG,DIALOG_STYLE_INPUT,title,msg,"修改","取消");
}

stock Race_ShowEditCpScriptDialog(playerid,id,cpid,tip[])//为指定玩家显示指定赛道的指定CP点的触发修改界面
{
	new msg[768],title[128],trigger[RACE_CP_MAX_FUNCTION],trcp[racecptype];
	format(title,128,"赛道编辑 - %s - 检查点 %i - 触发编辑",Race[id][rname],cpid);
	if(EditRace[playerid][rdialogcplist]==-1)
	    {
	    	format(msg,768,"----------------------------------------------------为了显示标题而存在的一行----------------------------------------------------\n为赛道[%s]的第[%i]个检查点新建触发\n请输入触发~最多%i个字符(中文1个算2个)%s",Race[id][rname],cpid,RACE_CP_MAX_FUNCTION,tip);
	    }
	else
	    {
			Race_GetCp(id,cpid,trcp);
			Race_GetCp_Script(trcp[rcpid],EditRace[playerid][rdialogcplist],trigger);
			format(msg,768,"----------------------------------------------------为了显示标题而存在的一行----------------------------------------------------\n当前赛道[%s]第[%i]个检查点的第[%i]条触发\n{FF0000}%s{FFFFFF}\n请输入要修改的触发~最多%i个字符(中文1个算2个)\n输入'插入'在这一行之上添加新触发\n输入'删除'来删除这一条触发%s",Race[id][rname],cpid,EditRace[playerid][rdialogcplist],trigger,RACE_CP_MAX_FUNCTION,tip);
		}
	ShowPlayerDialog(playerid,RACE_EDIT_CP_SCRIPT_DIALOG,DIALOG_STYLE_INPUT,title,msg,"确认","取消");
}

stock Race_ShowEditCpDialog(playerid,id,cpid,page)//为玩家显示指定赛道的指定cp点信息
{
	new title[128],msg[1024],trcp[racecptype],ts,maxpage,p=-1,fc[RACE_CP_MAX_FUNCTION];
	format(title,128,"赛道编辑 - %s - 检查点 %i",Race[id][rname],cpid);
	Race_GetCp(id,cpid,trcp);
	format(msg,1024,"x %f|y %f|z %f|a %f\nsize %f < 点击这行将尺寸修改为%f\n{00FF00}删除检查点{FFFFFF}\n插入检查点\n移动检查点\n传送到检查点\n从这个检查点开始测试赛道\n添加新触发\n删除所有触发",trcp[rcpx],trcp[rcpy],trcp[rcpz],trcp[rcpa],trcp[rcppsize],EditRace[playerid][rcpsize]);
	ts=Race_GetCp_Scripts(trcp[rcpid]);
	if(ts!=0)
	    {
	        if(ts>RACE_PAGES)
	            {
	                maxpage=Race_MaxPage(ts);
	                if(maxpage==page)
	                    {
							format(msg,1024,"%s\n- 第 %i 页 - 共 %i 页 -",msg,page,maxpage);
						}
					else
					    {
					        format(msg,1024,"%s\n- 第 %i 页 - 共 %i 页 -\n下一页",msg,page,maxpage);
					    }
					if(page!=1)
					    {
					        p=1;
					    }
	            }
			else
			    {
			        format(msg,1024,"%s\n-",msg);
			    }
			for(new i=RACE_PAGES*(page-1);i<RACE_PAGES*page;i++)
			    {
			        if(i<ts)
			            {
					        Race_GetCp_Script(trcp[rcpid],i,fc);
					        format(msg,1024,"%s\n触发 %i %s",msg,i,fc);
						}
			    }
			if(p==1)
			    {
			        format(msg,1024,"%s\n上一页",msg);
			    }
	    }
	ShowPlayerDialog(playerid,RACE_EDIT_CP_DIALOG,DIALOG_STYLE_LIST,title,msg,"选择","关闭");
}



stock Race_Test(playerid,raceid,cpid)//让指定玩家从指定cp点开始测试赛道
{
	if(Race[raceid][rcps]==0) return -1;
	new trcp[racecptype];
	Race_GetCp(raceid,cpid,trcp);
	SetPlayerPosEx(playerid,trcp[rcpx],trcp[rcpy],trcp[rcpz]);
	SetPlayerFacingAngleEx(playerid,trcp[rcpa]);
	EditRace[playerid][rtestcp]=cpid;
	Race_ShowCp(playerid,raceid,cpid);
	return 1;
}

stock Race_HideCp(playerid)//为指定玩家关闭cp点和小地图点
{
	DisablePlayerRaceCheckpoint(playerid);
    RemovePlayerMapIcon(playerid,RACE_MAP_ICON_INDEX);
}

stock Race_ShowCp(playerid,id,cpid)//为指定玩家显示指定赛道的指定cp点
{
	new trcp[racecptype],nrcp[racecptype];
	Race_GetCp(id,cpid,trcp);
 	if(cpid!=Race[id][rcps])
  		{
  			Race_GetCp(id,cpid+1,nrcp);
     		SetPlayerRaceCheckpoint(playerid,0,trcp[rcpx],trcp[rcpy],trcp[rcpz],nrcp[rcpx],nrcp[rcpy],nrcp[rcpz],trcp[rcppsize]);
            SetPlayerMapIcon(playerid,RACE_MAP_ICON_INDEX,nrcp[rcpx],nrcp[rcpy],nrcp[rcpz],RACE_MAP_ICON_TYPE,0,1);
		}
	else
		{
  			SetPlayerRaceCheckpoint(playerid,1,trcp[rcpx],trcp[rcpy],trcp[rcpz],0,0,0,trcp[rcppsize]);
	    }
}

stock Race_SetMiaoshu(id,miaoshu[])//设置指定赛道的描述
{
	new Cache:result,msg[512];//,ti=GetTickCount();
	format(Race[id][rmiaoshu],64,"%s",miaoshu);
	format(msg,512,"UPDATE race SET miaoshu = '%s' WHERE rname = '%s'",Race[id][rmiaoshu],Race[id][rname]);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
	//printf("  >P-Race 设置赛道[%s]的描述为[%s],耗时[%i]ms",Race[id][rname],miaoshu,GetTickCount()-ti);
}

stock Race_NewCp_Script(raceid,cpid,script[])//为指定cp点增加一条触发
{
	new Cache:result,msg[512],tr[racecptype];
	Race_GetCp(raceid,cpid,tr);
	format(msg,512,"INSERT INTO race_cp_script (rid,cpid,list,script) VALUES ('%i','%i','%i','%s')",Race[raceid][rid],tr[rcpid],Race_GetCp_Scripts(tr[rcpid]),script);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
	//printf("  >P-Race 新增一条触发于赛道[%s]的第[%i]个CP点,耗时[%i]ms",Race[raceid][rname],cpid,GetTickCount()-ti);
}

stock Race_InsCp_Script(raceid,cpid,list,script[])//为指定cp点插入一条触发
{
	new msg[512],tr[racecptype];
	cache_delete(mysql_query(mariadb, "begin"));
	Race_GetCp(raceid,cpid,tr);
	format(msg,512,"UPDATE race_cp_script SET list = list+1 WHERE cpid = '%i' and list>='%i'",tr[rcpid],list);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"INSERT INTO race_cp_script (rid,cpid,list,script) VALUES ('%i','%i','%i','%s')",Race[raceid][rid],tr[rcpid],list,script);
	cache_delete(mysql_query(mariadb, msg));
	cache_delete(mysql_query(mariadb, "commit"));
	//printf("  >P-Race 插入一条触发于赛道[%s]第[%i]个CP点的第[%i]条触发之上",Race[raceid][rname],cpid,list,GetTickCount()-ti);
}

stock Race_SetCp_Script(raceid,cpid,list,script[])// 修改指定CP点第list条触发
{
	new Cache:result, msg[512], tr[racecptype];
	Race_GetCp(raceid,cpid,tr);
	format(msg,512,"UPDATE race_cp_script SET script='%s' WHERE cpid = '%i' and list='%i' ",script,tr[rcpid],list);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
	//printf("  >P-Race 修改赛道[%s]第[%i]个CP点的第[%i]条触发,耗时[%i]ms",Race[raceid][rname],cpid,list,GetTickCount()-ti);
}

stock Race_GetCp_Script(cpdb,list,script[])//返回指定cp点第list条触发
{
	new Cache:result,msg[512];//,ti=GetTickCount();
	format(msg,512,"SELECT script FROM Race_cp_script WHERE cpid='%i' and list='%i'",cpdb,list);
	result = mysql_query(mariadb, msg);
	cache_get_value_index(0, 0, script, RACE_CP_MAX_FUNCTION);
	cache_delete(result);
	//printf("  >P-Race 读取CP点[%i]的第[%i]条触发,耗时[%i]ms",cpdb,list,GetTickCount()-ti);
	return script;
}

stock Race_DelCp_Script(raceid,cpid,list)//删除指定cp点的指定触发
{
	new msg[512],tr[racecptype];
	cache_delete(mysql_query(mariadb, "begin"));
	Race_GetCp(raceid,cpid,tr);
	format(msg,512,"DELETE FROM race_cp_script WHERE cpid = '%i' and list='%i'",tr[rcpid],list);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"UPDATE race_cp_script SET list = list-1 WHERE cpid = '%i' and list>'%i'",tr[rcpid],list);
	cache_delete(mysql_query(mariadb, msg));
	cache_delete(mysql_query(mariadb, "commit"));
	//printf("  >P-Race 删除赛道[%s]的第[%i]个CP点的第[%i]条触发,耗时[%i]ms",Race[raceid][rname],cpid,list,GetTickCount()-ti);
}

stock Race_DelCp_Scripts(raceid,cpid)//删除指定cp点触发
{
	new msg[512],tr[racecptype];
	Race_GetCp(raceid,cpid,tr);
	format(msg,512,"DELETE FROM race_cp_script WHERE cpid = '%i'",tr[rcpid]);
	cache_delete(mysql_query(mariadb, msg));
	//printf("  >P-Race 删除赛道[%s]第[%i]个CP点的触发,耗时[%i]ms",Race[raceid][rname],cpid,GetTickCount()-ti);
}

stock Race_GetCp_Scripts(cpdb)//返回指定CP点的触发数
{
	new Cache:result,s,msg[512];
	format(msg,512,"SELECT * FROM race_cp_script WHERE cpid='%i'",cpdb);
	result = mysql_query(mariadb, msg);
	cache_get_row_count(s);
	cache_delete(result);
//	printf("  >P-Race 读取赛道[%s]的CP数量,耗时[%i]ms",Race[id][rname],GetTickCount()-ti);
	return s;
}

stock Race_NewCp(id,playerid)//为指定赛道增加一个CP点，位置为playerid所在的位置
{
	new Cache:result,msg[512],Float:px,Float:py,Float:pz,Float:pa;//,ti=GetTickCount();
	GetPlayerPosEx(playerid,px,py,pz);
	GetPlayerFacingAngleEx(playerid,pa);
	format(msg,512,"INSERT INTO race_cp (rid,cp,x,y,z,a,size) VALUES ('%i','%i','%s','%s','%s','%s','%s')",Race[id][rid],Race[id][rcps],F2S(px),F2S(py),F2S(pz),F2S(pa),F2S(EditRace[playerid][rcpsize]));
	result = mysql_query(mariadb, msg);
	cache_delete(result);
	Race[id][rcps]++;
	//printf("  >P-Race 新增一个CP点于赛道[%s],耗时[%i]ms",Race[id][rname],GetTickCount()-ti);
}

stock Race_InsCp(id,playerid,cpid)//为指定赛道插入一个CP点，位置为playerid所在的位置
{
	cpid--;
	new msg[512],Float:px,Float:py,Float:pz,Float:pa;//,ti=GetTickCount();
	GetPlayerPosEx(playerid,px,py,pz);
	GetPlayerFacingAngleEx(playerid,pa);
	cache_delete(mysql_query(mariadb, "begin"));
	format(msg,512,"UPDATE race_cp SET cp = cp+1 WHERE rid = '%i' and cp>='%i'",Race[id][rid],cpid);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"INSERT INTO race_cp (rid,cp,x,y,z,a,size) VALUES ('%i','%i','%s','%s','%s','%s','%s')",Race[id][rid],cpid,F2S(px),F2S(py),F2S(pz),F2S(pa),F2S(EditRace[playerid][rcpsize]));
	cache_delete(mysql_query(mariadb, msg));
	cache_delete(mysql_query(mariadb, "commit"));
	Race[id][rcps]++;
//	printf("  >P-Race 在第[%i]个CP点插入一个CP点于赛道[%s],耗时[%i]ms",cpid+1,Race[id][rname],GetTickCount()-ti);
}

stock Race_SetCpPos(playerid,id,cpid)// 修改指定赛道的指定CP点位置
{
	cpid--;
	new Cache:result,msg[128],Float:x,Float:y,Float:z,Float:a;//,ti=GetTickCount();
	GetPlayerPosEx(playerid,x,y,z);
	GetPlayerFacingAngleEx(playerid,a);
	format(msg,128,"UPDATE race_cp SET x = '%f',y = '%f',z = '%f',a='%f' WHERE rid = '%i' and cp='%i' ",x,y,z,a,Race[id][rid],cpid);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
//	printf("  >P-Race 修改赛道[%s]的第[%i]个CP点位置,耗时[%i]ms",Race[id][rname],cpid+1,GetTickCount()-ti);
}

stock Race_SetAllCpSize(playerid,id)// 修改指定赛道的所有CP点尺寸
{
	new Cache:result,msg[128];//,ti=GetTickCount();
	format(msg,128,"UPDATE race_cp SET size='%f' WHERE rid = '%i'",EditRace[playerid][rcpsize],Race[id][rid]);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
//	printf("  >P-Race 修改赛道[%s]的所有CP点尺寸,耗时[%i]ms",Race[id][rname],GetTickCount()-ti);
}

stock Race_SetCpSize(playerid,id,cpid)// 修改指定赛道的指定CP点尺寸
{
	cpid--;
	new Cache:result,msg[128];//,ti=GetTickCount();
	format(msg,128,"UPDATE race_cp SET size='%f' WHERE rid = '%i' and cp='%i' ",EditRace[playerid][rcpsize],Race[id][rid],cpid);
	result = mysql_query(mariadb, msg);
	cache_delete(result);
//	printf("  >P-Race 修改赛道[%s]的第[%i]个CP点尺寸,耗时[%i]ms",Race[id][rname],cpid+1,GetTickCount()-ti);
}

stock Race_DelCp(id,cpid)//删除指定赛道的cp点
{
	new rf[racecptype],msg[512];//,ti=GetTickCount();
	cache_delete(mysql_query(mariadb, "begin"));
	Race_GetCp(id,cpid,rf);
	Race_DelCp_Scripts(id,cpid);
	cpid--;
	format(msg,512,"DELETE FROM race_cp WHERE rid = '%i' and cp='%i'",Race[id][rid],cpid);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"UPDATE race_cp SET cp = cp-1 WHERE rid = '%i' and cp>'%i'",Race[id][rid],cpid);
	cache_delete(mysql_query(mariadb, msg));
	cache_delete(mysql_query(mariadb, "commit"));
	Race[id][rcps]=Race_GetCps(id);
//	printf("  >P-Race 删除赛道[%s]的第[%i]个CP点,耗时[%i]ms",Race[id][rname],cpid+1,GetTickCount()-ti);

}

stock Race_GetCp(id,cpid,cps[])//返回指定赛道的cp点数据
{
	cpid--;
	new Cache:result,msg[512];//,ti=GetTickCount();
	format(msg,512,"SELECT * FROM race_cp WHERE rid='%i' and cp='%i'",Race[id][rid],cpid);
	result = mysql_query(mariadb, msg);
	cache_get_value_index_int(0, 0, cps[rcpid]);
	cache_get_value_index_int(0, 1, cps[rrid]);
	cache_get_value_index_int(0, 2, cps[rcp]);
	cache_get_value_index_float(0, 3, cps[rcpx]);
	cache_get_value_index_float(0, 4, cps[rcpy]);
	cache_get_value_index_float(0, 5, cps[rcpz]);
	cache_get_value_index_float(0, 6, cps[rcpa]);
	cache_get_value_index_float(0, 7, cps[rcppsize]);
	cache_delete(result);
//	printf("  >P-Race 读取赛道[%s]的第[%i]个CP点,耗时[%i]ms",Race[id][rname],cpid+1,GetTickCount()-ti);
	return cps;
}

stock Race_GetCps(id)//返回指定赛道的CP总数
{
	new Cache:result,s,msg[512];
	format(msg,512,"SELECT * FROM race_cp WHERE rid='%i'",Race[id][rid]);
	result = mysql_query(mariadb, msg);
	cache_get_row_count(s);
	cache_delete(result);
//	printf("  >P-Race 读取赛道[%s]的CP数量,耗时[%i]ms",Race[id][rname],GetTickCount()-ti);
	return s;
}

stock Race_CheckName(name[])//检查赛道是否存在,返回-1表示不存在,其他数字表示赛道的ID
{
	new s=-1;
	for(new i=0;i<RACE_MAX;i++)
	    {
	        if(Race[i][rid]!=-1)
	            {
	                if(mk_strcmp(Race[i][rname],name)==0)
	                    {
	                        s=i;
	                        i=RACE_MAX+1;
	                    }
	      	    }
	    }
	return s;
}

stock PRace_changeNtoN(oldname[], newname[]) {
	new msg[256];
	format(msg, sizeof msg, "UPDATE race SET rauthor='%s' WHERE rauthor='%s'", newname, oldname);
	cache_delete(mysql_query(mariadb, msg));
}

stock Race_ReTop(raceid)//清空指定赛道的top10
{
	new tpm[512];//,ti=GetTickCount();
	cache_delete(mysql_query(mariadb, "begin"));
	for(new i=0;i<10;i++) {
		format(RaceTop[raceid][i][rtop_name],32,"unknow");
		RaceTop[raceid][i][rtop]=-1;
		format(tpm,512,"UPDATE race SET top%i_name = 'unknow' ,top%i='-1' WHERE rname = '%s'",i+1,i+1,Race[raceid][rname]);
		cache_delete(mysql_query(mariadb, tpm));
	}
	cache_delete(mysql_query(mariadb, "commit"));
//  	printf("  >P-Race 初始化赛道[%s]的记录,耗时:%ims",Race[raceid][rname],GetTickCount()-ti);
}

stock Race_Delete(raceid)//删除赛道
{

	new msg[512],playerid=Race[raceid][reditplayer];
	pc_cmd_r(playerid, "edit q");
	Race_Game_DeleteByRid(raceid);
	cache_delete(mysql_query(mariadb, "begin"));
	format(msg,512,"DELETE FROM race_cp_script WHERE rid = '%i'",Race[raceid][rid]);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"DELETE FROM race_cp WHERE rid='%i'",Race[raceid][rid]);
	cache_delete(mysql_query(mariadb, msg));
	format(msg,512,"DELETE FROM race WHERE rname='%s'",Race[raceid][rname]);
	cache_delete(mysql_query(mariadb, msg));
	cache_delete(mysql_query(mariadb, "commit"));
	Race[raceid][rid]=-1;
//	printf("  >P-Race 删除赛道[%s],耗时[%i]ms",Race[raceid][rname],GetTickCount()-ti);

}

stock Race_Create(name[],password[],author[])//创建赛道,返回-1表示失败
{
	if(NowRaceId>=RACE_MAX) return -1;
	new id=NowRaceId;
	NowRaceId++;
//	new ti=GetTickCount();
	new tpm[1024],Cache:result;
//	printf(name);
	format(Race[id][rname],32,"%s",name);
	format(Race[id][rpassword],32,"%s",password);
	format(Race[id][rauthor],32,"%s",author);
	format(Race[id][rmiaoshu],64,"无");
	Race[id][rcreatedate]=gettime();
	Race[id][reditplayer]=-1;
	Race[id][rcps]=0;
	Race[id][completeTimes] = 0;
	cache_delete(mysql_query(mariadb, "begin"));
	format(tpm,1024,"INSERT INTO race (rname,rpassword,rcreatedate,rauthor,miaoshu) VALUES ('%s','%s','%i','%s','%s')",name,password,gettime(),author,"无");
	cache_delete(mysql_query(mariadb, tpm));
	format(tpm,1024,"SELECT rid FROM race WHERE rname='%s'",name);
	result = mysql_query(mariadb, tpm);
	cache_get_value_index(0, 0, tpm, sizeof tpm);
	cache_delete(result);
	Race[id][rid]=strval(tpm);
	for(new i=0;i<10;i++) {
		format(RaceTop[id][i][rtop_name],32,"unknow");
		RaceTop[id][i][rtop]=-1;
		format(tpm,1024,"UPDATE race SET top%i_name = 'unknow' ,top%i='-1' WHERE rname = '%s'",i+1,i+1,name);
		cache_delete(mysql_query(mariadb, tpm));
	}
	cache_delete(mysql_query(mariadb, "commit"));
//	printf("  >P-Race 创建赛道[%i]:%s(%i)|密码:%s|作者:%s|耗时:%ims",id,Race[id][rname],Race[id][rid],Race[id][rpassword],Race[id][rauthor],GetTickCount()-ti);
	return 0;
}

stock Race_Load()//载入赛道
{
	new Cache:result,msg[128],rs;
	format(msg,128,"SELECT * FROM race");
	result = mysql_query(mariadb, msg);
	cache_get_row_count(rs);
	printf("  >P-Race 赛道数量:%i|赛道数量上线:%i|可用赛道剩余:%i",rs,RACE_MAX,RACE_MAX-rs);
	if(rs>=RACE_MAX) {
		printf("  >P-Race 赛道数量超过赛道上线,请修改PRace.inc中的RACE_MAX,系统关闭");
    //SetTimer("FilterScriptExit",3000,0);
		RaceEd=-1;
	}	else {
		for(new i=0;i<rs;i++) {
			cache_get_value_index_int(i, 0, Race[i][rid]);
			cache_get_value_index(i, 1, Race[i][rname], 32);
			cache_get_value_index(i, 2, Race[i][rpassword], 32);
			cache_get_value_index_int(i, 3, Race[i][rcreatedate]);
			cache_get_value_index(i, 4, Race[i][rauthor], 32);
			cache_get_value_index(i, 5, Race[i][rmiaoshu], 64);
			for(new n=6;n<16;n++) {
				cache_get_value_index(i, n, RaceTop[i][n-6][rtop_name], 32);
				cache_get_value_index_int(i, n+10, RaceTop[i][n-6][rtop]);
			}
			cache_get_value_index_int(i, 26, Race[i][completeTimes]);
			Race[i][rstate]=0;
			Race[i][reditplayer]=-1;
		}
		for(new i=0;i<rs;i++) {
			Race[i][rcps]=Race_GetCps(i);
		}
		NowRaceId=rs;
		printf("  >P-Race 读取完成..");
	}
	cache_delete(result);
}
/*赛道相关*/

stock PRaceInit()
{
	//printf("  >P-Race 载入..");
	RaceEd=0;
	Race_Init();
	//printf("  >P-Race 初始化..");
	Race_Load();
	Race_Game_Init();
	return 1;
}

stock PRaceExit()
{
	//printf(" >P-Race 退出..");
	return 1;
}

stock PRaceConnect(playerid)
{
	EditRace[playerid][rraceid]=-1;
	EditRace[playerid][rtestcp]=-1;
	EditRace[playerid][rdialogcp]=1;
	EditRace[playerid][rcpsize]=9;
	EditRace[playerid][rdialogpage]=1;
	EditRace[playerid][rdialogcppage]=1;
	EditRace[playerid][rdialogcplist]=-1;
	GameRace[playerid][rgameid]=-1;
	GameRace[playerid][rtime]=0;
	GameRace[playerid][rgamecp]=0;
	SelectRacePage[playerid]=1;
	CpTextDraw[playerid]=CreatePlayerTextDraw(playerid,550,420,"~~");
	GetPlayerName(playerid,PlayerName[playerid],32);
	return 1;
}

stock PRaceDisconnect(playerid)
{
	if(EditRace[playerid][rraceid]!=-1)
	    {
			Race[EditRace[playerid][rraceid]][rstate]=0;
			Race[EditRace[playerid][rraceid]][reditplayer]=-1;
		}
	if(GameRace[playerid][rgameid]!=-1)
	    {
	        Race_Game_Quit(playerid);
	    }
	PlayerTextDrawDestroy(playerid,CpTextDraw[playerid]);
	return 1;
}

CMD:r(playerid, cmdtext[]) {
	new tmp[128],msg[128],idx;
	if(RaceEd==-1) {
		SendClientMessage(playerid,COLOR,"[赛车]系统关闭中…");
	    return 1;
	}
	tmp=strtok(cmdtext,idx);
	if(!strlen(tmp)) {
		SendClientMessage(playerid,COLOR,"[赛车]帮助:/r s 开启/开始比赛");
	    SendClientMessage(playerid,COLOR,"[赛车]帮助:/r join 加入比赛");
	    SendClientMessage(playerid,COLOR,"[赛车]帮助:/r leave 离开比赛");
	    SendClientMessage(playerid,COLOR,"[赛车]帮助:/r create 创建赛道");
	    SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit 编辑赛道帮助");
		return 1;
	} else if(mk_strcmp(tmp,"s")==0) {
		if(EditRace[playerid][rraceid]!=-1) {
			format(msg,128,"[赛车]你正在编辑赛道[%s],因此无法开启比赛",Race[EditRace[playerid][rraceid]][rname]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(GameRace[playerid][rgameid]!=-1) {
			if(playerid==RaceHouse[GameRace[playerid][rgameid]][rplayerid]) {
				if(RaceHouse[GameRace[playerid][rgameid]][rstate]==0) {
					Race_Game_Start(GameRace[playerid][rgameid]);
				}
			} else {
				format(msg,128,"[赛车]请耐心等待发起者'%s'开始比赛~",PlayerName[RaceHouse[GameRace[playerid][rgameid]][rplayerid]]);
				SendClientMessage(playerid,COLOR,msg);
			}
			return 1;
		}
		tmp=strtok(cmdtext,idx);
		if(!strlen(tmp)) {
			Race_ShowGameSelDialog(playerid,SelectRacePage[playerid]);
			return 1;
		}
        new id=Race_CheckName(tmp);
		if(id==-1) {
			format(msg,128,"[赛车]赛道[%s]不存在....",tmp);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(Race[id][rcps]==0) {
			format(msg,128,"[赛车]赛道[%s]无法开启.因为其检查点数量为0",tmp);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		new hid=Race_Game_Create(playerid,id);
		if(hid==-1) {
			format(msg,128,"[赛车]创建房间失败...当前赛车房间已达上限~请等待其他房间完成比赛..");
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		format(msg,128,"[赛车] %s 创建了比赛'%s',输入'/r join %i'来加入比赛!",PlayerName[playerid],Race[id][rname],playerid);
		SendClientMessageToAll(COLOR,msg);
		Race_Game_Join(playerid,hid);
		return 1;
	} else if(mk_strcmp(tmp,"join")==0) {
		if(EditRace[playerid][rraceid]!=-1) {
			format(msg,128,"[赛车]你正在编辑赛道[%s],因此无法加入比赛",Race[EditRace[playerid][rraceid]][rname]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(GameRace[playerid][rgameid]!=-1) {
			format(msg,128,"[赛车]你已经加入了一场比赛");
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		tmp=strtok(cmdtext,idx);
		if(!strlen(tmp)) {
			format(msg,128,"[赛车]用法:/r join ID[任何一个在比赛房间内的人ID]");
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		new id=strval(tmp);
		if(GameRace[id][rgameid]==-1) {
			format(msg,128,"[赛车] %s 不在房间内",PlayerName[id]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		new ri=Race_Game_Join(playerid,GameRace[id][rgameid]);
		if(ri==-1) {
			format(msg,128,"[赛车] 加入房间失败,这个房间已经开始比赛了..");
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		return 1;
	} else if(mk_strcmp(tmp,"leave")==0) {
		if(GameRace[playerid][rgameid]==-1) {
			format(msg,128,"[赛车]你没有参与任何比赛~请输入/r j或者/r s来加入/开启新的比赛~");
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
        SendClientMessage(playerid,COLOR,"[赛车]你退出了比赛");
        new hid=GameRace[playerid][rgameid];
		Race_Game_Quit(playerid);
		format(msg,128,"[赛车] %s 离开了比赛",PlayerName[playerid]);
		Race_Game_Msg(hid,msg);
		return 1;
	} else if(mk_strcmp(tmp,"edit")==0) {
		tmp=strtok(cmdtext,idx);
		if(!strlen(tmp)) {
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit [赛道名] [赛道编辑密码],编辑赛道");
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit q 退出编辑模式");
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit cpsize,设置/查看当前编辑cp的尺寸");
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit d,界面操作");
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit cp,在当前位置放置一个cp点");
			SendClientMessage(playerid,COLOR,"[赛车]帮助:/r edit trg,查看触发说明");
			return 1;
		}
		if(mk_strcmp(tmp,"trg")==0) {
			Race_ShowTrgDialog(playerid,"main");
			return 1;
		} else if(mk_strcmp(tmp,"cp")==0) {
			if(EditRace[playerid][rraceid]==-1) {
				SendClientMessage(playerid,COLOR,"[赛车]你没有正在编辑的赛道..");
				return 1;
			}
			Race_NewCp(EditRace[playerid][rraceid],playerid);
			format(msg,128,"[赛车]你为赛道[%s]在这个位置放置了第[%i]个CP点",Race[EditRace[playerid][rraceid]][rname],Race[EditRace[playerid][rraceid]][rcps]);
			SendClientMessage(playerid,COLOR,msg);
       		Race_ShowCp(playerid,EditRace[playerid][rraceid],Race[EditRace[playerid][rraceid]][rcps]);
			return 1;
		} else if(mk_strcmp(tmp,"q")==0) {
			if(EditRace[playerid][rraceid]==-1) {
				SendClientMessage(playerid,COLOR,"[赛车]你没有正在编辑的赛道..");
				return 1;
			}
			format(msg,128,"[赛车]你退出了赛道[%s]的编辑模式",Race[EditRace[playerid][rraceid]][rname]);
			SendClientMessage(playerid,COLOR,msg);
			Race[EditRace[playerid][rraceid]][rstate]=0;
			Race[EditRace[playerid][rraceid]][reditplayer]=-1;
			EditRace[playerid][rraceid]=-1;
			EditRace[playerid][rtestcp]=-1;
			EditRace[playerid][rdialogpage]=1;
			EditRace[playerid][rdialogcp]=1;
			EditRace[playerid][rdialogcppage]=1;
			EditRace[playerid][rdialogcplist]=-1;
			Race_HideCp(playerid);
			return 1;
		} else if(mk_strcmp(tmp,"cpsize")==0) {
			tmp=strtok(cmdtext,idx);
			if(!strlen(tmp)) {
				format(msg,128,"[赛车]你的CP点尺寸为:%f",EditRace[playerid][rcpsize]);
				SendClientMessage(playerid,COLOR,msg);
				return 1;
			}
			if(strlen(tmp)>12) {
				format(tmp,128,"%.7f",floatstr(tmp));
			}
			new Float:f=floatstr(tmp);
			if(f<0) f=0;
			if(f>1000) f=1000;
			EditRace[playerid][rcpsize]=f;
			format(msg,128,"[赛车]你将cp点尺寸设置为:%f",f);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		} else if(mk_strcmp(tmp,"d")==0) {
			if(EditRace[playerid][rraceid]==-1) return SendClientMessage(playerid,COLOR,"[赛车]你没有正在编辑的赛道..");
			Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
			return 1;
		}
		new id=Race_CheckName(tmp);
		if(id==-1) {
			format(msg,128,"[赛车]赛道[%s]不存在....",tmp);
		    SendClientMessage(playerid,COLOR,msg);
		    return 1;
		}
		tmp=strtok(cmdtext,idx);
		if(!strlen(tmp)) return SendClientMessage(playerid,COLOR,"[赛车]赛道维护密码不能为空..");
		if(mk_strcmp(tmp,Race[id][rpassword])!=0) {
			format(msg,128,"[赛车]赛道[%s]的维护密码错误...",Race[id][rname]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(EditRace[playerid][rraceid]!=-1) {
			format(msg,128,"[赛车]你正在编辑[%s]赛道,因此无法编辑[%s]",Race[EditRace[playerid][rraceid]][rname],Race[id][rname]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(Race[id][rstate]==1) {
			format(msg,128,"[赛车]赛道[%s]正在被玩家[-{00FF00}%s{9393AA}-]编辑,因此你无法编辑..",Race[id][rname],PlayerName[Race[id][reditplayer]]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		if(GameRace[playerid][rgameid]!=-1) {
			format(msg,128,"[赛车]你正处于赛车状态,因此无法编辑赛道[%s]~",Race[id][rname]);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		Race[id][rstate]=1;
		Race[id][reditplayer]=playerid;
		EditRace[playerid][rraceid]=id;
		EditRace[playerid][rtestcp]=-1;
		EditRace[playerid][rdialogpage]=1;
		EditRace[playerid][rdialogcp]=1;
		EditRace[playerid][rdialogcppage]=1;
		EditRace[playerid][rdialogcplist]=-1;
		Race_Game_DeleteByRid(id);
		format(msg,128,"[赛车]你已进入[%s]的编辑模式,该赛道将无法进行新的比赛,正在比赛的将被关闭",Race[id][rname]);
		SendClientMessage(playerid,COLOR,msg);
		SendClientMessage(playerid,COLOR,"[赛车]编辑模式下,你可以使用/r edit cpsize来查看/设置尺寸");
		SendClientMessage(playerid,COLOR,"[赛车]创建的新CP点将使用cpsize所设置的尺寸");
		SendClientMessage(playerid,COLOR,"[赛车]创建新CP点使用/r edit cp");
		SendClientMessage(playerid,COLOR,"[赛车]重新打开此界面使用/r edit d");
		pc_cmd_r(playerid, "edit d");
		return 1;
	} else if(mk_strcmp(tmp,"create")==0) {
	    new raname[128];
		raname=strtok(cmdtext,idx);
		if(mk_strcmp(raname,"")==0) return SendClientMessage(playerid,COLOR,"[赛车]帮助:/r create [赛道名称] [编辑密码]");
		if(strlen(raname)>=32) return SendClientMessage(playerid,COLOR,"[赛车]赛道名字过长……请缩短一点吧");
		if(Race_CheckName(raname)!=-1) {
			format(msg,128,"[赛车]赛道[%s]已存在,请换一个名字吧~",raname);
			SendClientMessage(playerid,COLOR,msg);
			return 1;
		}
		tmp=strtok(cmdtext,idx);
		if(mk_strcmp(tmp,"")==0) return SendClientMessage(playerid,COLOR,"[赛车]赛道编辑密码不能为空……");
		if(strlen(tmp)>=32) return SendClientMessage(playerid,COLOR,"[赛车]赛道编辑密码过长……请缩短一点吧");
		if(Race_Create(raname,tmp,PlayerName[playerid])==-1) {
			format(msg,128,"[赛车]赛道[%s]创建失败,没有可用的赛道ID", raname);
			SendClientMessage(playerid,0x000088FF,msg);
		} else {
			format(msg,128,"[赛车]赛道[%s]创建成功,编辑密码[%s],请牢记~",raname,tmp);
			SendClientMessage(playerid,COLOR,msg);
		}
		return 1;
	}
	return 1;
}

stock PRaceOPERC(playerid)
{
	if(GetPlayerVehicleID(playerid)!=0)
	    {
			if(EditRace[playerid][rraceid]!=-1&&EditRace[playerid][rtestcp]!=-1)
			    {
			        new msg[128],raid=EditRace[playerid][rraceid];
			        if(GetPlayerState(playerid)==2)
           				{
	        				Race_Cp_Script_Start(playerid,raid,EditRace[playerid][rtestcp]);
						}
			        format(msg,128,"[赛车]赛道[%s]检查点:%i/%i",Race[raid][rname],EditRace[playerid][rtestcp],Race[raid][rcps]);
			        SendClientMessage(playerid,COLOR,msg);
			        if(EditRace[playerid][rtestcp]==Race[raid][rcps])
			            {
			                format(msg,128,"[赛车]赛道[%s]已完成",Race[raid][rname]);
			                SendClientMessage(playerid,COLOR,msg);
			                EditRace[playerid][rtestcp]=-1;
			                Race_HideCp(playerid);
							return 1;
			            }
			        EditRace[playerid][rtestcp]++;
			        Race_ShowCp(playerid,raid,EditRace[playerid][rtestcp]);
			        return 1;
			    }
			if(GameRace[playerid][rgameid]!=-1)
			    {
			        if(RaceHouse[GameRace[playerid][rgameid]][rstate]==0)
			            {
							new msg[128];
							format(msg,128,"[赛车]请耐心等待比赛开始..");
							SendClientMessage(playerid,COLOR,msg);
							return 1;
			            }
			        if(RaceHouse[GameRace[playerid][rgameid]][rstate]==1)
			            {
							new msg[128];
							format(msg,128,"[赛车]比赛正在倒计时,请做好准备!");
							SendClientMessage(playerid,COLOR,msg);
							return 1;
			            }
					if(RaceHouse[GameRace[playerid][rgameid]][rstate]==2)
					    {
									new msg[128],raid=RaceHouse[GameRace[playerid][rgameid]][rraceid];
									if(GetPlayerState(playerid)==2) {
										Race_Cp_Script_Start(playerid,raid,GameRace[playerid][rgamecp]);
									}
									format(msg,128,"CP:%i/%i",GameRace[playerid][rgamecp],Race[raid][rcps]);
									PlayerTextDrawSetString(playerid,CpTextDraw[playerid],msg);
									if(GameRace[playerid][rgamecp]==Race[raid][rcps]-1) {
										pRaceUnixtimestamp[playerid] = GetTickCount();
									} else if(GameRace[playerid][rgamecp]==Race[raid][rcps]) {
										new tick = GetTickCount();
									
										AccountRaceMission(playerid, Race[raid][rname]);
										format(msg,128,"[赛车] %s 完成了赛道[%s],耗时:%s秒(小组第%i)",PlayerName[playerid],Race[raid][rname],MsToS(tick-GameRace[playerid][rtime]),RaceHouse[GameRace[playerid][rgameid]][rtop]);
										SendClientMessageToAll(COLOR,msg);
										Race_CompleteTimesPP(raid);
										RaceHouse[GameRace[playerid][rgameid]][rtop]++;
										new top=Race_Game_End(playerid,raid,tick-GameRace[playerid][rtime]);
										if(top!=-1) {
										  format(msg,128,"[赛车]恭喜 %s 登上了赛道 %s 的排行榜 No.%i!",PlayerName[playerid],Race[raid][rname],top+1);
										  SendClientMessageToAll(COLOR,msg);
										}
										Race_ShowGameDialog(playerid,raid);
										Race_Game_Quit(playerid);
										return 1;
					        }
									GameRace[playerid][rgamecp]++;
									Race_ShowCp(playerid,raid,GameRace[playerid][rgamecp]);
									return 1;
					    }
					return 1;
			    }
		}
	return 1;
}

stock Race_CompleteTimesPP(id) {
	new msg[128];
	format(msg, sizeof msg, "UPDATE race SET completeTimes=completeTimes+1 WHERE rid='%d'", Race[id][rid]);
	cache_delete(mysql_query(mariadb, msg));
	Race[id][completeTimes] ++;
}

forward PRaceODR(playerid, dialogid, response, listitem, inputtext[]);
public PRaceODR(playerid, dialogid, response, listitem, inputtext[])
{
	new msg[128],idx;
	if(dialogid==RACE_EDIT_MAIN_DIALOG)
	    {
	        if(response==1)
	            {
					msg=strtok(inputtext,idx);
					if(mk_strcmp(msg,"清空赛道记录")==0)
					    {
					        Race_ReTop(EditRace[playerid][rraceid]);
					        Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
					        return 1;
					    }
					if(mk_strcmp(msg,"删除该赛道")==0)
					    {
					        Race_Delete(EditRace[playerid][rraceid]);
					        return 1;
					    }
					if(mk_strcmp(msg,"测试该赛道")==0)
    					{
					        if(Race_Test(playerid,EditRace[playerid][rraceid],1)==-1)
					            {
					                SendClientMessage(playerid,COLOR,"[赛车]无法测试这个赛道,因为其检查点数量为0");
					            	Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
					            }
							else
							    {
				       				EditRace[playerid][rdialogpage]=1;
				            		EditRace[playerid][rdialogcp]=1;
									EditRace[playerid][rdialogcppage]=1;
									EditRace[playerid][rdialogcplist]=-1;
							    }
					        return 1;
					    }
					if(mk_strcmp(msg,"修改所有检查点尺寸为")==0)
					    {
					        Race_HideCp(playerid);
					        Race_SetAllCpSize(playerid,EditRace[playerid][rraceid]);
							Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
							return 1;
					    }
					if(mk_strcmp(msg,"检查点")==0)
					    {
					        msg=strtok(inputtext,idx);
					        EditRace[playerid][rdialogcp]=strval(msg);
					        Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					        return 1;
					    }
					if(mk_strcmp(msg,"赛道描述")==0)
					    {
					        Race_ShowEditMiaoshuDialog(playerid,EditRace[playerid][rraceid],"");
							return 1;
						}
	                if(mk_strcmp(msg,"上一页")==0)
	                    {
	                        EditRace[playerid][rdialogpage]--;
	                        Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
							return 1;
	                    }
					if(mk_strcmp(msg,"下一页")==0)
					    {
					        EditRace[playerid][rdialogpage]++;
							Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
					        return 1;
					    }
                    Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
					return 1;
				}
			else
			    {
            		EditRace[playerid][rdialogpage]=1;
            		EditRace[playerid][rdialogcp]=1;
					EditRace[playerid][rdialogcppage]=1;
					EditRace[playerid][rdialogcplist]=-1;
            		return 1;
				}

	    }
	if(dialogid==RACE_EDIT_MIAOSHU_DIALOG)
	    {
	        if(response==1)
	            {
	                if(strlen(inputtext)>=64)
	                    {
							Race_ShowEditMiaoshuDialog(playerid,EditRace[playerid][rraceid],"\n{00FF00}描述过长~请适当的缩短一点");
	                        return 1;
	                    }
					Race_SetMiaoshu(EditRace[playerid][rraceid],inputtext);
					Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
				 	return 1;
	            }
			else
			    {
			    	Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
			        return 1;
			    }
	    }
	if(dialogid==RACE_EDIT_CP_DIALOG)
	    {
	        if(response==1)
	            {
	                msg=strtok(inputtext,idx);
	                if(mk_strcmp(msg,"从这个检查点开始测试赛道")==0)
	                    {
					        if(Race_Test(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp])==-1)
					            {
					                SendClientMessage(playerid,COLOR,"[赛车]无法测试这个赛道,因为其检查点数量为0");
					                Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					            }
							else
							    {
				       				EditRace[playerid][rdialogpage]=1;
				            		EditRace[playerid][rdialogcp]=1;
									EditRace[playerid][rdialogcppage]=1;
									EditRace[playerid][rdialogcplist]=-1;
								}
	                        return 1;
	                    }
	                if(mk_strcmp(msg,"上一页")==0)
	                    {
	                        EditRace[playerid][rdialogcppage]--;
	                        Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
							return 1;
	                    }
					if(mk_strcmp(msg,"下一页")==0)
					    {
					        EditRace[playerid][rdialogcppage]++;
							Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					        return 1;
					    }
	                if(mk_strcmp(msg,"触发")==0)
	                    {
	                        msg=strtok(inputtext,idx);
	                        EditRace[playerid][rdialogcplist]=strval(msg);
	                		Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"");
							return 1;
	                    }
	                if(mk_strcmp(msg,"删除所有触发")==0)
						{
						    Race_DelCp_Scripts(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
						    EditRace[playerid][rdialogcplist]=-1;
						    EditRace[playerid][rdialogcppage]=1;
							Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
							return 1;
						}
	                if(mk_strcmp(msg,"添加新触发")==0)
						{
						    EditRace[playerid][rdialogcplist]=-1;
							Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"");
							return 1;
						}
	                if(mk_strcmp(msg,"插入检查点")==0)
	                    {
	                        Race_InsCp(EditRace[playerid][rraceid],playerid,EditRace[playerid][rdialogcp]);
							EditRace[playerid][rdialogpage]=1;
							EditRace[playerid][rdialogcp]=1;
							EditRace[playerid][rdialogcplist]=-1;
	                        Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcppage]);
	                        return 1;
	                    }
	                if(mk_strcmp(msg,"删除检查点")==0)
	                    {
	                        new s;
	                        Race_HideCp(playerid);
	                        Race_DelCp(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
							s=Race_MaxPage(Race[EditRace[playerid][rraceid]][rcps]);
							if(EditRace[playerid][rdialogpage]>s)
							    {
							        EditRace[playerid][rdialogpage]=s;
							    }
							EditRace[playerid][rdialogcp]=1;
        					EditRace[playerid][rdialogcppage]=1;
							EditRace[playerid][rdialogcplist]=-1;
					    	Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
					        return 1;
	                    }
	                if(mk_strcmp(msg,"传送到检查点")==0)
	                    {
	                        new trcp[racecptype];
	                        Race_ShowCp(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
	                        Race_GetCp(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],trcp);
							SetPlayerPosEx(playerid,trcp[rcpx],trcp[rcpy],trcp[rcpz]);
       						SetPlayerFacingAngleEx(playerid,trcp[rcpa]);
					    	Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					        return 1;
	                    }
	                if(mk_strcmp(msg,"移动检查点")==0)
	                    {
                            Race_SetCpPos(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
                            Race_ShowCp(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
						 	Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					        return 1;
	                    }
					if(mk_strcmp(msg,"size")==0)
					    {
                            Race_SetCpSize(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
                            Race_ShowCp(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp]);
							Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					        return 1;
					    }
					Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
					return 1;
	            }
			else
			    {
            		EditRace[playerid][rdialogcp]=1;
					EditRace[playerid][rdialogcppage]=1;
					EditRace[playerid][rdialogcplist]=-1;
			    	Race_ShowEditDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogpage]);
			        return 1;
			    }
	    }
	if(dialogid==RACE_EDIT_CP_SCRIPT_DIALOG)
 		{
			if(response==1)
			    {
			        if(EditRace[playerid][rdialogcplist]==-1)
			            {
			                if(mk_strcmp(inputtext,"删除")==0||mk_strcmp(inputtext,"插入")==0)
			                    {
									Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"\n建立新触发时'插入'与'删除'指令无法使用");
									return 1;
								}
							if(strlen(inputtext)>=RACE_CP_MAX_FUNCTION)
							    {
									Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"\n{00FF00}触发太长了,请缩短一点吧...");
									return 1;
								}
                            Race_NewCp_Script(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],inputtext);
                            Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
							return 1;
			            }
					else
					    {
					        if(mk_strcmp(inputtext,"删除")==0)
					            {
					                Race_DelCp_Script(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcplist]);
									EditRace[playerid][rdialogcplist]=-1;
									new tr[racecptype];
									Race_GetCp(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],tr);
									new s=Race_MaxPage(Race_GetCp_Scripts(tr[rcpid]));
									if(EditRace[playerid][rdialogcppage]>s)
									    {
									        EditRace[playerid][rdialogcppage]=s;
									    }
									Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
									return 1;
					            }
							if(mk_strcmp(inputtext,"插入")==0)
							    {
							        Race_InsCp_Script(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcplist],"新插入的触发,请修改触发内容~");
									Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"");
							        return 1;
							    }
							if(strlen(inputtext)>=RACE_CP_MAX_FUNCTION)
							    {
									Race_ShowEditCpScriptDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],"\n{00FF00}触发太长了,请缩短一点吧...");
									return 1;
								}
                            Race_SetCp_Script(EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcplist],inputtext);
                            Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
							return 1;
					    }
			    }
			EditRace[playerid][rdialogcplist]=-1;
			Race_ShowEditCpDialog(playerid,EditRace[playerid][rraceid],EditRace[playerid][rdialogcp],EditRace[playerid][rdialogcppage]);
			return 1;
   		}
	if(dialogid==RACE_GAME_MAIN_DIALOG)
	    {
	        if(response==1)
	            {
	                msg=strtok(inputtext,idx);
	                if(mk_strcmp(msg,"开始")==0)
	                    {
	                        msg=strtok(inputtext,idx);
	                        new id=Race_CheckName(msg);
	                        if(id!=-1)
	                            {
	                                format(msg,128,"s %s",Race[id][rname]);
	                                pc_cmd_r(playerid, msg);
	                                return 1;
	                            }
							SendClientMessage(playerid,COLOR,"[赛车]发生了一个错误:GAME_MAIN_DIALOG_RESPONSE");
							return 1;
	                    }
	            }
			return 1;
	    }
	if(dialogid==RACE_GAME_MAIN_DIALOG_SEL)
	    {
	        if(response==1)
	            {
					msg=strtok(inputtext,idx);
					if(mk_strcmp(msg,"上一页")==0)
						{
							SelectRacePage[playerid]--;
							Race_ShowGameSelDialog(playerid,SelectRacePage[playerid]);
							return 1;
						}
					if(mk_strcmp(msg,"下一页")==0)
					    {
					    	SelectRacePage[playerid]++;
							Race_ShowGameSelDialog(playerid,SelectRacePage[playerid]);
							return 1;
					    }
					if(mk_strcmp(msg,"赛道")==0)
					    {
							msg=strtok(inputtext,idx);
		     				new id=Race_CheckName(msg);
		         			if(id!=-1)
		            			{
									Race_ShowGameDialog(playerid,id);
		                      		return 1;
		                       	}
							SendClientMessage(playerid,COLOR,"[赛车]发生了一个错误:RACE_GAME_MAIN_DIALOG_SEL");
							return 1;
					    }
                    Race_ShowGameSelDialog(playerid,SelectRacePage[playerid]);
					return 1;
	            }
			else
			    {
			        SelectRacePage[playerid]=1;
			        return 1;
			    }
	    }
	if(dialogid==RACE_EDIT_CP_SCRIPT_HELP)
	    {
	        if(response==1)
	            {
	                msg=strtok(inputtext,idx);
	                if(mk_strcmp(msg,"查看语法")==0)
	                    {
	                        Race_ShowTrgDialog(playerid,"语法");
	                        return 1;
	                    }
					if(mk_strcmp(msg,"查看函数")==0)
					    {
	                        Race_ShowTrgDialog(playerid,"函数");
	                        return 1;
					    }
					if(mk_strcmp(msg,"查看转义符")==0)
					    {
         					Race_ShowTrgDialog(playerid,"转义符");
	                        return 1;
					    }
					if(mk_strcmp(msg,"函数>")==0)
					    {
					        msg=strtok(inputtext,idx);
					        Race_ShowTrgFunDialog(playerid,msg);
					        return 1;
					    }
					if(mk_strcmp(msg,"转义符>")==0)
					    {
					        msg=strtok(inputtext,idx);
					        Race_ShowTrgRetDialog(playerid,msg);
					        return 1;
					    }
	            }
			return 1;
	    }
	return 0;
}
