#include <graphics.h>
#include <stdio.h>
#include "control.h"

/* 控件显示函数 */
void control_show(CONTROL_T ctrl){
	int center=0;
	char str[20]={0};
	int i=0;
	if(ctrl.state==1){
		setfillcolor(ctrl.bgColor1);
	}
	else{
		setfillcolor(ctrl.bgColor2);
	}
	settextcolor(ctrl.textColor);
	settextstyle(16,10,_T("宋体"));
	if(ctrl.type ==BUTTON||ctrl.type ==EDIT||ctrl.type==EDIT_PWD){
		fillrectangle(ctrl.x,ctrl.y,ctrl.x+ctrl.width,ctrl.y+ctrl.height );
	}
	if(ctrl.type==EDIT_PWD){
		if(ctrl.visible){
			outtextxy(ctrl.x+5,ctrl.y+15,ctrl.text);
		}else{
			for(i=0;i<(int)strlen(ctrl.text);i++) str[i]='*';
			outtextxy(ctrl.x+5,ctrl.y+15,str);
		}
	}
	else if(ctrl.type ==EDIT||ctrl.type ==LABEL){
		outtextxy(ctrl.x+5,ctrl.y+15,ctrl.text );
	}
	else if(ctrl.type==BUTTON){
		center=ctrl.x+(ctrl.width-strlen(ctrl.text)*10)/2;
		outtextxy(center,ctrl.y+15,ctrl.text);
	}
}

/* 窗口显示 */
WINDOW_T window_show(WINDOW_T win){
	int i=0;
	setfillcolor(win.bgColor);
	fillrectangle(win.x,win.y,win.x+win.width,win.y+win.height);
	for(i=0;i<win.count;i++){
		control_show(win.controls[i]);
	}
	return win;
}

/* 窗口运行驱动 — 使用 EasyX getmessage() 读取图形窗口键盘事件 */
WINDOW_T window_run(WINDOW_T win){
	int i=0;
	ExMessage msg;

	/* 找到第一个可停留的控件(不是LABEL) */
	while (win.controls[i].type==LABEL){
		i++;
		if(i==win.count) i=0;
	}

	while(1){
		msg = getmessage(EX_KEY | EX_CHAR);

		if(msg.message == EX_KEY){
			if(msg.vkcode == VK_RETURN){			/* 回车 */
				if(win.controls[i].type == BUTTON){
					win.current = i;
					return win;
				}
			}
			else if(msg.vkcode == VK_TAB){			/* Tab 切换密码明文/密文 */
				if(win.controls[i].type == EDIT_PWD){
					win.controls[i].visible = !win.controls[i].visible;
					control_show(win.controls[i]);
				}
			}
			else if(msg.vkcode == VK_BACK){			/* 退格 */
				if(win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD){
					int len = (int)strlen(win.controls[i].text);
					if(len > 0){
						win.controls[i].text[len - 1] = '\0';
						control_show(win.controls[i]);
					}
				}
			}
			else if(msg.vkcode == VK_UP){			/* 上键 */
				win.controls[i].state=0;
				control_show(win.controls[i]);
				do{
					i--;
					if(i==-1) i=win.count-1;
				} while(win.controls[i].type==LABEL);
				win.controls[i].state=1;
				control_show(win.controls[i]);
			}
			else if(msg.vkcode == VK_DOWN){			/* 下键 */
				win.controls[i].state=0;
				control_show(win.controls[i]);
				do{
					i++;
					if(i==win.count) i=0;
				} while(win.controls[i].type==LABEL);
				win.controls[i].state=1;
				control_show(win.controls[i]);
			}
		}
		else if(msg.message == EX_CHAR){			/* 字符输入 */
			char ch = (char)msg.ch;
			if((ch>='0'&&ch<='9') || (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z')){
				if(win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD){
					int len = (int)strlen(win.controls[i].text);
					win.controls[i].text[len] = ch;
					control_show(win.controls[i]);
				}
			}
		}
	}
}
