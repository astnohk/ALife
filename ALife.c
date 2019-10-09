#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#define WINDOW_X 600
#define WINDOW_Y 600
#define Task_Num 3
#define Sensor_Num 3
#define DNA_NUM 2


Display *d;
Window w;
Pixmap pix;
GC gc,gcbl,gcyl,gcrd,gcor,gclb[3],gcgr[3];
Colormap cmap;
XColor col,exact;
XEvent event,noev;

const int Init_Num=30,Sight_Dist=96;
const long Def_Energy=1600,Def_Sunlight_Intensity=6;
int not,key;
int map[WINDOW_X][WINDOW_Y];
long Num_Survivor,Sunlight_Intensity;
double SinArr[360],CosArr[360],sum_sens[Sensor_Num],sum_task[Task_Num],sum_sens_predation[Sensor_Num],sum_task_predation[Task_Num];
const double Pi=3.1415926535897932384626433832795028,Sqrt2=1.4142135623730950;
char off[2][4]={{"off\0"},{"on\0"}},met[128];

struct timeval waittime;
struct AL{
	int type;/*0=Vegetable, 1=herbivory, 2=carnivorous*/
	int kill;
	long Energy;
	long Angle;
	int answer_LR;
	int x;
	int y;
	double dx;
	double dy;
	double DNA[DNA_NUM];
	double coefficient[Task_Num][Sensor_Num];
	double predation_coefficient[Task_Num][Sensor_Num];
	long Prior_Exist_Num;
	long Prior_Exist_Feed_Num;
	struct AL *prior;
	struct AL *next;} ALStartPoint;
struct {double Vector[180];double Distance[180];} sensor;
struct {int bef_x;int bef_y;struct AL *drag;} xbutton;
struct {int Write;} Switch;


int mover(struct AL *pt,int x,int y);
int AI(struct AL *pt,struct AL **ppt);
int Sensor(struct AL *pt,int *ptExist_Num,int *ptExist_Feed_Num);
void god(struct AL *pt);
struct AL *komachi(struct AL *del);


int main(){
	int i,k,m,itmp,itmp2;
	double dtmp,bef_Distance;
	struct AL *ALpt;

	srand(time(0));
	for(i=0;i<360;i++){
		SinArr[i]=sin(i/180.0*Pi);
		CosArr[i]=cos(i/180.0*Pi);
	}

	d=XOpenDisplay(0);
	w=XCreateSimpleWindow(d,RootWindow(d,0),0,0,WINDOW_X,WINDOW_Y,0,BlackPixel(d,0),WhitePixel(d,0));
	XSelectInput(d,w,ExposureMask|KeyPressMask|ButtonPressMask|ButtonMotionMask);
	XStoreName(d,w,"Artificial Life ver3");
	XMapWindow(d,w);
	cmap=DefaultColormap(d,0);
	XSetForeground(d,gc=XCreateGC(d,w,0,0),BlackPixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"lightskyblue1",&col,&exact))) exit(1);
	XSetBackground(d,gc,col.pixel);

	if(!(XAllocNamedColor(d,cmap,"dodgerblue4",&col,&exact))) exit(1);
	XSetForeground(d,gclb[0]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gclb[0],col.pixel);
	if(!(XAllocNamedColor(d,cmap,"steelblue2",&col,&exact))) exit(1);
	XSetForeground(d,gclb[1]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gclb[1],col.pixel);
	if(!(XAllocNamedColor(d,cmap,"lightskyblue2",&col,&exact))) exit(1);
	XSetForeground(d,gclb[2]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gclb[2],col.pixel);

	if(!(XAllocNamedColor(d,cmap,"green2",&col,&exact))) exit(1);
	XSetForeground(d,gcgr[0]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcgr[0],WhitePixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"darkgreen",&col,&exact))) exit(1);
	XSetForeground(d,gcgr[1]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcgr[1],WhitePixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"green4",&col,&exact))) exit(1);
	XSetForeground(d,gcgr[2]=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcgr[2],WhitePixel(d,0));

	if(!(XAllocNamedColor(d,cmap,"blue",&col,&exact))) exit(1);
	XSetForeground(d,gcbl=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcbl,WhitePixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"yellow",&col,&exact))) exit(1);
	XSetForeground(d,gcyl=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcyl,WhitePixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"orange",&col,&exact))) exit(1);
	XSetForeground(d,gcor=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcor,WhitePixel(d,0));
	if(!(XAllocNamedColor(d,cmap,"red",&col,&exact))) exit(1);
	XSetForeground(d,gcrd=XCreateGC(d,w,0,0),col.pixel);
	XSetBackground(d,gcrd,WhitePixel(d,0));

	pix=XCreatePixmap(d,w,WINDOW_X,WINDOW_Y,DefaultDepth(d,0));
	XMaskEvent(d,ExposureMask,&noev);


	All_Reset:
	Sunlight_Intensity=Def_Sunlight_Intensity;
	ALpt=&ALStartPoint;
	for(i=0;i<Init_Num;i++){
		god(ALpt);
		ALpt=ALpt->next;
		ALpt->type=i>Init_Num*6.0/10.0?i>Init_Num*8.5/10.0?2:1:0;
		while("おぜう様"){
			ALpt->dx=ALpt->x=rand()/(double)RAND_MAX*(WINDOW_X-2.0)+1.0;
			ALpt->dy=ALpt->y=rand()/(double)RAND_MAX*(WINDOW_Y-2.0)+1.0;
			if(!ALpt->type){
				itmp=1;
				for(k=-1;k<2;k++){
					for(m=-1;m<2;m++){
						if(map[ALpt->x+k][ALpt->y+m]){
							itmp=0;
							k=2;
							m=2;
						}
					}
				}
				for(k=-1;k<2 && itmp;k++){
					for(m=-1;m<2;m++){
						map[ALpt->x+k][ALpt->y+m]=1;
					}
				}
				break;
			}else if(ALpt->type){
				if(!map[ALpt->x][ALpt->y]){
					map[ALpt->x][ALpt->y]=ALpt->type+1;
					break;
				}
			}
		}
	}

	while("＼射命丸／"){
		while(XPending(d)){
			XNextEvent(d,&event);
			switch(event.type){
				case ButtonPress:
					if(event.xbutton.x<0 || event.xbutton.y>=WINDOW_X || event.xbutton.y<0 || event.xbutton.y>=WINDOW_Y) break;
					if(event.xbutton.button==Button1){
						if(Switch.Write){
							for(i=0;i<2;i++){
								for(k=0;k<2;k++){
									itmp=event.xbutton.x+i;
									itmp2=event.xbutton.y+k;
									if(itmp>0 && itmp<WINDOW_X && itmp2>0 && itmp2<WINDOW_Y)
										if(!map[itmp][itmp2])
											map[itmp][itmp2]=-1;
								}
							}
						}else if(ALStartPoint.next){
							xbutton.drag=ALpt=ALStartPoint.next;
							itmp=ALpt->x-event.xbutton.x;
							itmp2=ALpt->y-event.xbutton.y;
							bef_Distance=itmp*itmp+itmp2*itmp2;
							ALpt=ALpt->next;
							while(ALpt){
								itmp=ALpt->x-event.xbutton.x;
								itmp2=ALpt->y-event.xbutton.y;
								dtmp=itmp*itmp+itmp2*itmp2;
								if(dtmp<bef_Distance){
									bef_Distance=dtmp;
									xbutton.drag=ALpt;
								}
								ALpt=ALpt->next;
							}
							mover(xbutton.drag,event.xbutton.x,event.xbutton.y);
						}
					}else if(event.xbutton.button==Button3){
						if(!map[event.xbutton.x][event.xbutton.y]){
							god(&ALStartPoint);
							ALStartPoint.next->dx=ALStartPoint.next->x=event.xbutton.x;
							ALStartPoint.next->dy=ALStartPoint.next->y=event.xbutton.y;
							for(i=-1;i<2 && !ALStartPoint.next->type;i++){
								for(k=-1;k<2;k++){
									map[ALStartPoint.next->x+i][ALStartPoint.next->y+k]=1;
								}
							}
						}
					}
					break;
				case MotionNotify:
					if(event.xbutton.state&Button1Mask){
						if(Switch.Write){
							for(i=0;i<2;i++){
								for(k=0;k<2;k++){
									itmp=event.xbutton.x+i;
									itmp2=event.xbutton.y+k;
									if(itmp>0 && itmp<WINDOW_X && itmp2>0 && itmp2<WINDOW_Y)
										if(!map[itmp][itmp2])
											map[itmp][itmp2]=-1;
								}
							}
						}else
							mover(xbutton.drag,event.xbutton.x,event.xbutton.y);
					}
					break;
				case KeyPress:
					if((key=XLookupKeysym(&event.xkey,0))==XK_Escape) exit(0);
					switch(key){
						case XK_F2:
							for(i=0;i<WINDOW_X;i++)
								for(k=0;k<WINDOW_Y;k++)
									map[i][k]=0;
							while(ALStartPoint.next){
								komachi(ALStartPoint.next);
							}
							goto All_Reset;
						case XK_w: Switch.Write=!Switch.Write; break;
						case XK_Page_Up: Sunlight_Intensity+=Sunlight_Intensity<12; break;
						case XK_Page_Down: Sunlight_Intensity-=Sunlight_Intensity>1; break;
					}
			}
		}

		Num_Survivor=0;
		ALpt=ALStartPoint.next;
		XFillRectangle(d,pix,gclb[Sunlight_Intensity>4.0?Sunlight_Intensity>8.0?2:1:0],0,0,WINDOW_X,WINDOW_Y);
		while(ALpt){
			Num_Survivor++;
			switch(ALpt->type){
				case 1:
					AI(ALpt,&ALpt);
					XFillRectangle(d,pix,gcbl,ALpt->x-2,ALpt->y-2,5,5);
					itmp=12*SinArr[ALpt->Angle];
					itmp2=12*CosArr[ALpt->Angle];
					XDrawLine(d,pix,gcbl,ALpt->x,ALpt->y,ALpt->x+itmp,ALpt->y+itmp2);
					break;
				case 2:
					AI(ALpt,&ALpt);
					XFillRectangle(d,pix,gcrd,ALpt->x-2,ALpt->y-2,5,5);
					itmp=12*SinArr[ALpt->Angle];
					itmp2=12*CosArr[ALpt->Angle];
					XDrawLine(d,pix,gcbl,ALpt->x,ALpt->y,ALpt->x+itmp,ALpt->y+itmp2);
					break;
				default:
					AI(ALpt,&ALpt);
					XFillRectangle(d,pix,gcgr[Sunlight_Intensity>4.0?Sunlight_Intensity>8.0?2:1:0],ALpt->x-1,ALpt->y-1,3,3);
			}
			ALpt=ALpt->next;
		}
		for(i=0;i<WINDOW_X;i++)
			for(k=0;k<WINDOW_Y;k++)
				if(map[i][k]==-1) XDrawPoint(d,pix,gc,i,k);
		sprintf(met,"Survivor %ld   Sunlight_Intensity %ld   [w]rite %s",Num_Survivor,Sunlight_Intensity,off[Switch.Write]);
		XDrawString(d,pix,gc,2,597,met,strlen(met));
		XCopyArea(d,pix,w,gc,0,0,WINDOW_X,WINDOW_Y,0,0);
		waittime.tv_usec=20000;
		select(0,0,0,0,&waittime);
		XMaskEvent(d,ExposureMask,&noev);
	}
	return 0;
}



int mover(struct AL *pt,int x,int y){
	int i,k,itmp1,itmp2;
	if(x<0 || x>=WINDOW_X || y<0 || y>=WINDOW_Y) return 1;
	if(!pt->type){
		for(i=-1;i<2;i++){
			for(k=-1;k<2;k++){
				itmp1=pt->x+i;
				itmp2=pt->y+k;
				if(itmp1>=0 && itmp1<WINDOW_X && itmp2>=0 && itmp2<WINDOW_Y)
					map[itmp1][itmp2]=0;
			}
		}
		for(i=-1;i<2;i++){
			for(k=-1;k<2;k++){
				itmp1=x+i;
				itmp2=y+k;
				if(itmp1>=0 && itmp1<WINDOW_X && itmp2>=0 && itmp2<WINDOW_Y)
					if(!map[itmp1][itmp2])
						map[itmp1][itmp2]=1;
			}
		}
	}else{
		map[pt->x][pt->y]=0;
		map[x][y]=pt->type+1;
	}
	pt->dx=pt->x=x;
	pt->dy=pt->y=y;
	return 0;
}



int AI(struct AL *pt,struct AL **ppt){
	int i,itmp,itmp2,itmp3,itmp4,bef_x,bef_y,answer_Forward=0,Exist_Num=0,Exist_Feed_Num;
	double dtmp,dtmp2,bkup_sum_task,bkup_sum_sens[Sensor_Num];
	struct AL *target;

	if(pt->kill){
		*ppt=komachi(pt);
		return 1;
	}else if(pt->Energy<1){
		if(pt->type!=2 || rand()>RAND_MAX/2.0){
			*ppt=komachi(pt);
		}else{
			i=pt->type=0;
			map[pt->x][pt->y]=0;
			for(itmp=-1;itmp<2;itmp++){
				for(itmp2=-1;itmp2<2;itmp2++){
					itmp3=pt->x+itmp;
					itmp4=pt->y+itmp2;
					if(itmp3>=0 && itmp3<WINDOW_X && itmp4>=0 && itmp4<WINDOW_Y){
						if(!map[itmp3][itmp4]){
							i++;
							map[itmp3][itmp4]=1;
						}
					}
				}
			}
			if(!i) *ppt=komachi(pt);
			else pt->Energy=Def_Energy;
		}
		return 1;
	}else if(!pt->type){
		if(pt->Energy<Def_Energy && rand()<RAND_MAX/64.0) pt->Energy+=Sunlight_Intensity;
		if(rand()<RAND_MAX/640.0 && pt->Energy>Def_Energy*0.5){ /*Vegetable Propagation*/
			god(pt);
			pt->next->x=pt->x+(rand()>RAND_MAX/2.0?10:-10);
			pt->next->y=pt->y+(rand()>RAND_MAX/2.0?10:-10);
			i=0;
			for(itmp=-1;itmp<2 && !i;itmp++){
				for(itmp2=-1;itmp2<2 && !i;itmp2++){
					if(pt->next->x+itmp<0 || pt->next->x+itmp>=WINDOW_X || pt->next->y+itmp2<0 || pt->next->y+itmp2>=WINDOW_Y){
						i=1;
					}else if(map[pt->next->x+itmp][pt->next->y+itmp2]){
						i=1;
					}
				}
			}
			if(i) komachi(pt->next);
			else{
				pt->next->type=0;
				pt->next->Energy/=2;
				for(itmp=-1;itmp<2;itmp++)
					for(itmp2=-1;itmp2<2;itmp2++)
						map[pt->next->x+itmp][pt->next->y+itmp2]=1;
				pt->next->dx=pt->next->x;
				pt->next->dy=pt->next->y;
			}
		}
		return 0;
	}

	if(pt->Energy>pt->DNA[1]){ /*Propagation except Veget*/
		for(itmp=-1;itmp<2;itmp++){
			for(itmp2=-1;itmp2<2;itmp2++){
				itmp3=pt->x+itmp;
				itmp4=pt->y+itmp2;
				if(itmp3<0 || itmp3>=WINDOW_X || itmp4<0 || itmp4>=WINDOW_Y) continue;
				else if(!map[itmp3][itmp4]){
					itmp=itmp2=8;
				}
			}
		}
		if(itmp&8){
			god(pt);
			if(rand()>RAND_MAX/64.0) pt->next->type=pt->type;
			pt->Energy/=2;
			pt->next->Energy=pt->Energy;
			pt->next->dx=pt->next->x=itmp3;
			pt->next->dy=pt->next->y=itmp4;
			map[itmp3][itmp4]=pt->type+1;
			for(itmp=0;itmp<Task_Num;itmp++){
				for(itmp2=0;itmp2<Sensor_Num;itmp2++){
					for(i=0;i<DNA_NUM;i++)
						pt->next->DNA[i]=pt->DNA[i];
					pt->next->coefficient[itmp][itmp2]=pt->coefficient[itmp][itmp2];
					pt->next->predation_coefficient[itmp][itmp2]=pt->predation_coefficient[itmp][itmp2];
				}
			}
		}
	}


	Sensor(pt,&Exist_Num,&Exist_Feed_Num);

	if(Exist_Feed_Num && pt->Energy<pt->DNA[0]){
		answer_Forward=2;
	}else if(sum_task[1]<5.0){
		answer_Forward=1;
	}else if(rand()>RAND_MAX*0.7){
		answer_Forward=1;
	}

	if(Exist_Feed_Num && pt->Energy<pt->DNA[0]){
		if(sum_task_predation[0]>sum_task_predation[2]) pt->answer_LR=3;
		else if(sum_task_predation[0]<sum_task_predation[2]) pt->answer_LR=4;
	}else if(sum_task[0]<sum_task[2]){
		pt->answer_LR=1;
	}else if(sum_task[0]>sum_task[2]){
		pt->answer_LR=2;
	}
	if(rand()>RAND_MAX*0.85){
		pt->answer_LR=rand()/(1.0+RAND_MAX)*2.0+1.0;
		if(Exist_Feed_Num) pt->answer_LR+=2;
	}

	if(answer_Forward){
		bef_x=dtmp=pt->dx;
		bef_y=dtmp2=pt->dy;
		dtmp+=SinArr[pt->Angle];
		dtmp2+=CosArr[pt->Angle];
		itmp=dtmp;
		itmp2=dtmp2;
		if(itmp<0 || itmp>=WINDOW_X || itmp2<0 || itmp2>=WINDOW_Y){
			pt->coefficient[1][1]+=sum_sens[1]/1000.0;
		}else if(answer_Forward==2){
			for(itmp3=-2;itmp3<3;itmp3++){
				for(itmp4=-2;itmp4<3;itmp4++){
					if(itmp+itmp3<0 || itmp+itmp3>=WINDOW_X || itmp2+itmp4<0 || itmp2+itmp4>=WINDOW_Y) continue;
					if(map[itmp+itmp3][itmp2+itmp4]==pt->type) itmp4=itmp3=1000;
				}
			}
			if(itmp3>100){
				target=ALStartPoint.next;
				while(target){
					if(target!=pt && itmp-2<=target->x && target->x<itmp+2 && itmp2-2<target->y && target->y<itmp2+2){
						target->kill=1;
						pt->Energy+=target->Energy;
						break;
					}
					target=target->next;
				}
			}
			map[pt->x][pt->y]=0;
			pt->x=pt->dx=dtmp;
			pt->y=pt->dy=dtmp2;
			map[pt->x][pt->y]=pt->type+1;
		}else if(map[itmp][itmp2]){
			pt->coefficient[1][1]+=sum_sens[1]/1000.0;
		}else{
			map[pt->x][pt->y]=0;
			pt->x=pt->dx=dtmp;
			pt->y=pt->dy=dtmp2;
			map[pt->x][pt->y]=pt->type+1;
			if(Exist_Num){
				pt->coefficient[1][1]-=sum_sens[1]/500.0;
				pt->coefficient[1][0]+=sum_sens[0]/500.0;
				pt->coefficient[1][2]+=sum_sens[2]/500.0;
			}
		}
		if(bef_x!=pt->x || bef_y!=pt->y) pt->Energy--;
	}else if(Exist_Num<pt->Prior_Exist_Num){
		for(i=0;i<Sensor_Num;i++)
			pt->coefficient[1][1]-=sum_sens[1]/500.0;
	}
	pt->Prior_Exist_Num=Exist_Num;

	switch(pt->answer_LR){
		case 1:
			bkup_sum_task=sum_task[0];
			for(i=0;i<Sensor_Num;i++) bkup_sum_sens[i]=sum_sens[i];
			while(0){
		case 3:
			bkup_sum_task=sum_task_predation[0];
			for(i=0;i<Sensor_Num;i++) bkup_sum_sens[i]=sum_sens_predation[i];
			}
			pt->Angle+=5;
			if(pt->Angle>359) pt->Angle-=360;
			Sensor(pt,0,0);
			if(pt->answer_LR==1){
				if(sum_task[0]<bkup_sum_task){
					pt->coefficient[0][0]-=bkup_sum_sens[0]/100.0;
					pt->coefficient[0][2]-=bkup_sum_sens[2]/100.0;
				}else{
					pt->coefficient[0][0]+=bkup_sum_sens[0]/100.0;
					pt->coefficient[0][2]+=bkup_sum_sens[2]/100.0;
				}
			}else{
				if(sum_task_predation[0]<bkup_sum_task){
					pt->predation_coefficient[0][0]-=bkup_sum_sens[0]/100.0;
					pt->predation_coefficient[0][2]-=bkup_sum_sens[2]/100.0;
				}else{
					pt->predation_coefficient[0][0]+=bkup_sum_sens[0]/100.0;
					pt->predation_coefficient[0][2]+=bkup_sum_sens[2]/100.0;
				}
			}
			break;
		case 2:
			bkup_sum_task=sum_task[2];
			for(i=0;i<Sensor_Num;i++) bkup_sum_sens[i]=sum_sens[i];
			while(0){
		case 4:
			bkup_sum_task=sum_task_predation[2];
			for(i=0;i<Sensor_Num;i++) bkup_sum_sens[i]=sum_sens_predation[i];
			}
			pt->Angle-=5;
			if(pt->Angle<0) pt->Angle+=360;
			Sensor(pt,0,0);
			if(pt->answer_LR==2){
				if(sum_task[2]<bkup_sum_task){
					pt->coefficient[2][0]-=bkup_sum_sens[0]/100.0;
					pt->coefficient[2][2]-=bkup_sum_sens[2]/100.0;
				}else{
					pt->coefficient[2][0]+=bkup_sum_sens[0]/100.0;
					pt->coefficient[2][2]+=bkup_sum_sens[2]/100.0;
				}
			}else{
				if(sum_task_predation[2]<bkup_sum_task){
					pt->predation_coefficient[2][0]-=bkup_sum_sens[0]/100.0;
					pt->predation_coefficient[2][2]-=bkup_sum_sens[2]/100.0;
				}else{
					pt->predation_coefficient[2][0]+=bkup_sum_sens[0]/100.0;
					pt->predation_coefficient[2][2]+=bkup_sum_sens[2]/100.0;
				}
			}
			break;
	}
	return 0;
}



int Sensor(struct AL *pt,int *ptExist_Num,int *ptExist_Feed_Num){
	int i,k,Vector,itmp1,itmp2;
	double dx,dy,dest_x,dest_y,diff_x,diff_y,dist,dtmp;

	if(ptExist_Num) *ptExist_Num=0;
	if(ptExist_Feed_Num) *ptExist_Feed_Num=0;
	Vector=pt->Angle-45;
	if(Vector<0) Vector+=360;
	for(i=0;i<91;i++){
		sensor.Vector[i]=0;
		if(++Vector>359) Vector-=360;
		dest_x=Sight_Dist*SinArr[Vector];
		dest_y=Sight_Dist*CosArr[Vector];
		diff_x=dest_x/Sight_Dist;
		diff_y=dest_y/Sight_Dist;
		dx=pt->x;
		dy=pt->y;
		dist=dest_x*dest_x+dest_y*dest_y;
		k=dest_x=dest_y=0;
		while("本みりん"){
			dest_x+=diff_x;
			dest_y+=diff_y;
			dx+=diff_x;
			dy+=diff_y;
			if(dest_x*dest_x+dest_y*dest_y>dist){
				sensor.Vector[i]=0;
				sensor.Distance[i]=0;
				break;
			}
			k++;
			if(!(k%3)){
				if(i<30 || i>60) XDrawPoint(d,pix,gcyl,dx,dy);
				else if(i==45) XDrawPoint(d,pix,gcrd,dx,dy);
				else XDrawPoint(d,pix,gcor,dx,dy);
			}
			itmp1=dx;
			itmp2=dy;
			if(dx<0 || dx>=WINDOW_X || dy<0 || dy>=WINDOW_Y){
				sensor.Vector[i]=-1;
				sensor.Distance[i]=sqrt(dx*dx+dy*dy);
				break;
			}else if(map[itmp1][itmp2] && pt->x!=itmp1 && pt->y!=itmp2){
				sensor.Vector[i]=map[itmp1][itmp2];
				sensor.Distance[i]=sqrt(dx*dx+dy*dy);
				break;
			}
		}
	}

	for(i=0;i<Sensor_Num;i++) sum_sens_predation[i]=sum_sens[i]=0;
	for(i=0;i<45;i++){
		if(sensor.Vector[i]==pt->type){
			if(ptExist_Feed_Num) ++*ptExist_Feed_Num;
			sum_sens_predation[0]+=(i/45.0+0.5)/(1.0+sensor.Distance[i]);
		}else if(sensor.Vector[i]){
			if(ptExist_Num) ++*ptExist_Num;
			sum_sens[0]+=(i/45.0+0.5)/(1.0+sensor.Distance[i]);
		}
	}
	for(i=0;i<31;i++){
		if(sensor.Vector[30+i]==pt->type){
			if(ptExist_Feed_Num) ++*ptExist_Feed_Num;
			dtmp=(i-15.0)/15.0;
			sum_sens_predation[1]+=(2.5-dtmp*dtmp)/(1.0+sensor.Distance[30+i])*(!dtmp?2.0:1.0);
		}else if(sensor.Vector[30+i]){
			if(ptExist_Num) ++*ptExist_Num;
			dtmp=(i-15.0)/15.0;
			sum_sens[1]+=(2.5-dtmp*dtmp)/(1.0+sensor.Distance[30+i])*(!dtmp?2.0:1.0);
		}
	}
	for(i=0;i<45;i++){
		if(sensor.Vector[46+i]==pt->type){
			if(ptExist_Feed_Num) ++*ptExist_Feed_Num;
			sum_sens_predation[2]+=(1.5-i/45.0)/(1.0+sensor.Distance[46+i]);
		}else if(sensor.Vector[46+i]){
			if(ptExist_Num) ++*ptExist_Num;
			sum_sens[2]+=(1.5-i/45.0)/(1.0+sensor.Distance[46+i]);
		}
	}
	sum_task[1]=sum_sens[1]*pt->coefficient[1][1]-sum_sens[0]*pt->coefficient[1][0]-sum_sens[2]*pt->coefficient[1][2];
	sum_task[0]=sum_sens[0]*pt->coefficient[0][0]+sum_sens[2]*pt->coefficient[0][2];
	sum_task[2]=sum_sens[0]*pt->coefficient[2][0]+sum_sens[2]*pt->coefficient[2][2];
	sum_task_predation[0]=sum_sens_predation[0]*pt->predation_coefficient[0][0]+sum_sens_predation[2]*pt->predation_coefficient[0][2];
	sum_task_predation[2]=sum_sens_predation[0]*pt->predation_coefficient[2][0]+sum_sens_predation[2]*pt->predation_coefficient[2][2];
	return 0;
}



void god(struct AL *pt){
	int i,k;
	struct AL *new,*bkup;
	bkup=pt->next;
	if(!(new=pt->next=(struct AL *)calloc(1,sizeof(struct AL)))){
		fprintf(stderr,"*** MEMORY ALLOCATE ERROR ***");
		exit(1);
	}
	new->prior=pt;
	new->next=bkup;
	if(new->next) new->next->prior=new;
	i=rand()/(RAND_MAX+1.0)*9.0;
	new->type=i>3?i>6?2:1:0;
	new->Energy=Def_Energy*(!new->type?0.5:1);
	new->Angle=rand()/(RAND_MAX+1.0)*360.0;
	new->DNA[0]=new->Energy*0.5*(rand()/(1.0+RAND_MAX)+1.0);
	new->DNA[1]=new->Energy*0.5*(rand()/(1.0+RAND_MAX)+1.0);
	for(i=0;i<Task_Num;i++){
		for(k=0;k<Sensor_Num;k++){
			new->coefficient[i][k]=2.0;
			new->predation_coefficient[i][k]=2.0;
		}
	}
}



struct AL *komachi(struct AL *del){
	int i,k,itmp1,itmp2;
	struct AL *pri;
	if(!del) return 0;
	if(!del->type){
		for(i=-1;i<2;i++){
			for(k=-1;k<2;k++){
				itmp1=del->x+i;
				itmp2=del->y+k;
				if(itmp1>=0 && itmp1<WINDOW_X && itmp2>=0 && itmp2<WINDOW_Y)
					map[itmp1][itmp2]=0;
			}
		}
	}else
		if(del->x>=0 && del->x<WINDOW_X && del->y>=0 && del->y<WINDOW_Y)
			map[del->x][del->y]=0;
	pri=del->prior;
	pri->next=del->next;
	if(del->next) del->next->prior=pri;
	free(del);
	return pri;
}
