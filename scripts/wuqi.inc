/*
	===================================
    对话框列表选择武器
                            	By:Dawn
	===================================
										*/
#define DIALOG_WQSD_MAIN                 (800)
#define DIALOG_WQSD_MELEE                DIALOG_WQSD_MAIN + 1
#define DIALOG_WQSD_THROW                DIALOG_WQSD_MAIN + 2
#define DIALOG_WQSD_PISTOL               DIALOG_WQSD_MAIN + 3
#define DIALOG_WQSD_SHOTGUN              DIALOG_WQSD_MAIN + 4
#define DIALOG_WQSD_SUBMACHINEGUN        DIALOG_WQSD_MAIN + 5
#define DIALOG_WQSD_RIFLE                DIALOG_WQSD_MAIN + 6
#define DIALOG_WQSD_HEAVYGUN             DIALOG_WQSD_MAIN + 7
#define DIALOG_WQSD_OTHER                DIALOG_WQSD_MAIN + 8
#define DIALOG_WQSD_HBA                  DIALOG_WQSD_MAIN + 9
#define DIALOG_WQSD_MELEE_2              DIALOG_WQSD_MAIN + 10

CMD:wuqi(playerid, params[])
{
	new msg[256];
	if(userInfo[playerid][OnlineTime] < 80) {
		SendClientMessage(playerid,0xFF0000FF,"[狂野武器系统]你的时间分不够,需要80时间分才能刷武器");
		return 1;
	}
	format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
	ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
	return 1;
}

	

forward wuqiODR(playerid, dialogid, response, listitem, inputtext[]);
public wuqiODR(playerid, dialogid, response, listitem, inputtext[])
{
	new msg[256];
	if(dialogid==DIALOG_WQSD_MAIN)
	{
		if(response)
		{
			if(listitem==0)
			{
				format(msg,256,"拳套\n高尔夫球杆\n警棍\n小刀\n棒球棍\n铁铲\n台球杆\n武士刀\n电锯\n紫色假阳具\n假阳具\n{FF0000}下一页");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MELEE,DIALOG_STYLE_LIST,"武器菜单[近战类]",msg,"确定","返回");
			}
			if(listitem==1)
			{
				format(msg,256,"手雷\n催泪瓦斯\n燃烧瓶");
				ShowPlayerDialog(playerid,DIALOG_WQSD_THROW,DIALOG_STYLE_LIST,"武器菜单[投掷类]",msg,"确定","返回");
			}
			if(listitem==2)
			{
				format(msg,256,"普通手枪\n消音手枪\n沙鹰");
				ShowPlayerDialog(playerid,DIALOG_WQSD_PISTOL,DIALOG_STYLE_LIST,"武器菜单[手枪]",msg,"确定","返回");
			}
			if(listitem==3)
			{
				format(msg,256,"单发霰弹枪\n双持霰弹枪\n7连发霰弹枪");
				ShowPlayerDialog(playerid,DIALOG_WQSD_SHOTGUN,DIALOG_STYLE_LIST,"武器菜单[霰弹枪]",msg,"确定","返回");
			}
			if(listitem==4)
			{
				format(msg,256,"SMG\nMP5\nTEC-9");
				ShowPlayerDialog(playerid,DIALOG_WQSD_SUBMACHINEGUN,DIALOG_STYLE_LIST,"武器菜单[冲锋枪]",msg,"确定","返回");
			}
			if(listitem==5)
			{
				format(msg,256,"AK47\nM4\n单发步枪\n狙击步枪");
				ShowPlayerDialog(playerid,DIALOG_WQSD_RIFLE,DIALOG_STYLE_LIST,"武器菜单[步枪]",msg,"确定","返回");
			}
			if(listitem==6)
			{
				format(msg,256,"火箭发射器\n热感追踪弹\n喷火器\n加特林\n遥控炸药包\n");
				ShowPlayerDialog(playerid,DIALOG_WQSD_HEAVYGUN,DIALOG_STYLE_LIST,"武器菜单[重武器]",msg,"确定","返回");
			}
			if(listitem==7)
			{
				format(msg,256,"喷雾罐\n灭火器\n照相机\n降落伞\n飞行器");
				ShowPlayerDialog(playerid,DIALOG_WQSD_OTHER,DIALOG_STYLE_LIST,"武器菜单[其他]",msg,"确定","返回");
			}
			if(listitem==8)
			{
				format(msg,256,"血\n护甲\n血和护甲");
				ShowPlayerDialog(playerid,DIALOG_WQSD_HBA,DIALOG_STYLE_LIST,"武器菜单[血和护甲]",msg,"确定","返回");
			}
		}
	}

	switch(dialogid)
	{
		case DIALOG_WQSD_MELEE:
		{
			if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
				case 0:
				{
					GivePlayerWeapon(playerid,1,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,2,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,3,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 3:
				{
					GivePlayerWeapon(playerid,4,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 4:
				{
					GivePlayerWeapon(playerid,5,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 5:
				{
					GivePlayerWeapon(playerid,6,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 6:
				{
					GivePlayerWeapon(playerid,7,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 7:
				{
					GivePlayerWeapon(playerid,8,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 8:
				{
					GivePlayerWeapon(playerid,9,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 9:
				{
					GivePlayerWeapon(playerid,10,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 10:
				{
					GivePlayerWeapon(playerid,11,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 11:
				{
					format(msg,256,"{FF0000}上一页\n银色振动器\n振动器\n花束\n拐杖");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MELEE_2,DIALOG_STYLE_LIST,"武器菜单[近战类]",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_THROW:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
				case 0:
				{
					GivePlayerWeapon(playerid,16,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,17,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,18,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_PISTOL:
		{
			if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
				case 0:
				{
					GivePlayerWeapon(playerid,22,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,23,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,24,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
        case DIALOG_WQSD_SHOTGUN:
        {
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
                case 0:
				{
					GivePlayerWeapon(playerid,25,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,26,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,27,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_SUBMACHINEGUN:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
				case 0:
				{
					GivePlayerWeapon(playerid,28,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,29,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,32,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_RIFLE:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
                case 0:
				{
					GivePlayerWeapon(playerid,30,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,31,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,33,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 3:
				{
					GivePlayerWeapon(playerid,34,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_HEAVYGUN:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
                case 0:
				{
					GivePlayerWeapon(playerid,35,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,36,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,37,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 3:
				{
					GivePlayerWeapon(playerid,38,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 4:
				{
					GivePlayerWeapon(playerid,39,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_OTHER:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
                case 0:
				{
					GivePlayerWeapon(playerid,41,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					GivePlayerWeapon(playerid,42,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					GivePlayerWeapon(playerid,43,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 3:
				{
					GivePlayerWeapon(playerid,46,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 4:
				{
                    SetPlayerSpecialAction(playerid,2);
                    format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_HBA:
		{
            if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
                case 0:
				{
					SetPlayerHealth(playerid,100);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 1:
				{
					SetPlayerArmour(playerid,100);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
					SetPlayerHealth(playerid,100);
					SetPlayerArmour(playerid,100);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
		case DIALOG_WQSD_MELEE_2:
		{
			if(!response)
			{
				format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
				ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				return 1;
			}
			switch(listitem)
			{
				case 0:
				{
					format(msg,256,"拳套\n高尔夫球杆\n警棍\n小刀\n棒球棍\n铁铲\n台球杆\n武士刀\n电锯\n紫色假阳具\n假阳具\n{FF0000}下一页");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MELEE,DIALOG_STYLE_LIST,"武器菜单[近战类]",msg,"确定","返回");
				}
				case 1:
				{
                    GivePlayerWeapon(playerid,12,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 2:
				{
                    GivePlayerWeapon(playerid,13,5000);
        		    format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 3:
				{
                    GivePlayerWeapon(playerid,14,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
				case 4:
				{
                    GivePlayerWeapon(playerid,15,5000);
					format(msg,256,"近战类\n投掷类\n手枪\n霰弹枪\n冲锋枪\n步枪\n重武器\n其他\n血和防弹衣");
					ShowPlayerDialog(playerid,DIALOG_WQSD_MAIN,DIALOG_STYLE_LIST,"武器菜单",msg,"确定","返回");
				}
			}
		}
	}
	return 1;
}
