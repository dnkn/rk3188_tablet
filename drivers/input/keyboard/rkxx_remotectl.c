/*$_rbox_$_ modify _$hzb,20120522*/
/*$_rbox_$_ modify _$add this file for rk29 remotectl*/

/* 
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/adc.h>
#include <asm/gpio.h>
#include <mach/remotectl.h>
#include <mach/iomux.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>

#define TAG "RKXX_REMOTECTL"
#if 1
#define D(fmt, arg...) printk("D>>> %s->%s(%d): " fmt, TAG, __FUNCTION__, __LINE__, ##arg)
#else
#define D(fmt, arg...)
#endif
#define E(fmt, arg...) printk("E>>> %s->%s(%d): " fmt, TAG, __FUNCTION__, __LINE__, ##arg)
#define I(fmt, arg...) printk("I>>> %s->%s(%d): " fmt, TAG, __FUNCTION__, __LINE__, ##arg)

#if 1
#define remotectl_dbg(bdata, format, arg...)		\
	dev_printk(KERN_INFO , &bdata->input->dev , format , ## arg)
#else
#define remotectl_dbg(bdata, format, arg...)	
#endif

#define EV_MENU					KEY_F1





#define SYSCONFIG_VIRTUAL_TOUCH_KEY



#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY

#define  M_LEFT  3
#define  M_RIGHT 4
#define  M_UP    2
#define  M_DOWN  1

#define  M_LEFT_UP 7
#define  M_LEFT_DOWN  5
#define  M_RIGHT_UP   8
#define  M_RIGHT_DOWN 6



void gp_vts_shoot(int type, int x, int y, int action);
void gp_vts_report(void);
void sk_vts_report(void);
#define M_CENTER  0


static int mode_vt_rk=M_CENTER;

void SKSetRK(int direct_skrock,int direct_skrock2);

void sk_vts_set(int type,int fg_start);
extern int SKVmt_enable(void);
extern int SKVmt_mode(void);



#endif




extern suspend_state_t get_suspend_state(void);

struct rkxx_remotectl_suspend_data{
    int suspend_flag;
    int cnt;
    long scanTime[50];
};

struct rkxx_remote_key_table{
    int scanCode;
	int keyCode;		
};

struct rkxx_remotectl_button {	
    int usercode;
    int nbuttons;
    struct rkxx_remote_key_table *key_table;
};

struct rkxx_remotectl_drvdata {
    int state;
	int nbuttons;
	int result;
    unsigned long pre_time;
    unsigned long cur_time;
    long period;
    int scanData;
    int count;
    int keybdNum;
    int keycode;
    int press;
    int pre_press;
    
    struct input_dev *input;
    struct timer_list timer;
    struct tasklet_struct remote_tasklet;
    struct wake_lock remotectl_wake_lock;
    struct rkxx_remotectl_suspend_data remotectl_suspend_data;
};



//特殊功能键值定义
    //193      //photo
    //194      //video
    //195      //music
    //196      //IE
    //197      //
    //198
    //199
    //200
    
    //183      //rorate_left
    //184      //rorate_right
    //185      //zoom out
    //186      //zoom in


#if 0
static struct rkxx_remote_key_table remote_key_table_meiyu_202[] = {
    {0xB0, KEY_REPLY},//ok = DPAD CENTER
    {0xA2, KEY_BACK}, 
    {0xD0, KEY_UP},
    {0x70, KEY_DOWN},
    {0x08, KEY_LEFT},
    {0x88, KEY_RIGHT},  ////////
    {0x42, KEY_HOME},     //home
    {0xA8, KEY_VOLUMEUP},
    {0x38, KEY_VOLUMEDOWN},
    {0xE2, KEY_SEARCH},     //search
    {0xB2, KEY_POWER},     //power off
    {0xC2, KEY_MUTE},       //mute
    {0xC8, KEY_MENU},

//media ctrl
    {0x78,   0x190},      //play pause
    {0xF8,   0x191},      //pre
    {0x02,   0x192},      //next

//pic
    {0xB8, 183},          //rorate left
    {0x58, 184},          //rorate right
    {0x68, 185},          //zoom out
    {0x98, 186},          //zoom in
//mouse switch
    {0xf0,388},
//display switch
    {0x82,   0x175},
};

static struct rkxx_remote_key_table remote_key_table_df[] = {
    {0xf8, KEY_REPLY},
    {0xc0, KEY_BACK}, 
    {0xf0, KEY_UP},
    {0xd8, KEY_DOWN},
    {0xd0, KEY_LEFT},
    {0xe8,KEY_RIGHT},  ////////
    {0x90, KEY_VOLUMEDOWN},
    {0x60, KEY_VOLUMEUP},
    {0x80, KEY_HOME},     //home
    {0xe0, 183},          //rorate left
    {0x10, 184},          //rorate right
    {0x20, 185},          //zoom out
    {0xa0, 186},          //zoom in
    {0x70, KEY_MUTE},       //mute
    {0x50, KEY_POWER},     //power off
    {0x40, KEY_SEARCH},     //search
};

static struct rkxx_remote_key_table remote_key_table_ff[] = {
    {0x78, KEY_BACK}, 
    
    {0xb0, KEY_UP},
    {0x30, KEY_DOWN},
    {0x48, KEY_LEFT},
    {0x08, KEY_RIGHT},  ////////
    
    {0xd8, KEY_HOME},     //home
    {0x3a, KEY_MUTE},       //mute
    {0xf8, KEY_POWER},     //power off
    {0x58, EV_MENU},     //search
    
    {0x20, KEY_VOLUMEDOWN},
    {0x10, KEY_VOLUMEUP},
    
    {0xe1, 183},          //rorate left
    {0x11, 184},          //rorate right
    {0x21, 185},          //zoom out
    {0xa1, 186},          //zoom in
};

#endif

 
#ifdef   CONFIG_MACH_RK3188_SKQ621

static struct rkxx_remote_key_table remote_key_table_ff[] = {
    {0x00, KEY_DOWN},
    {0x01, KEY_UP},//     
    {0x2,KEY_RIGHT},
    {0x3, KEY_LEFT},
    // {0x3, KEY_F1},
    {0x4, KEY_L},  //
    
    {0x5, KEY_R},     //null
    {6, KEY_Y},       //null
    {7, KEY_Y},     //power off
    {8, KEY_B},     //search  KEY_A
    
    {9,  KEY_A},
    {10, KEY_X},
    {11, KEY_J},    // null
    {12, KEY_K},
    {13, 0x00},
};
#else
static struct rkxx_remote_key_table remote_key_table_ff[] = {
    {0x00, KEY_DOWN},
    {0x01, KEY_UP},//     
    {0x2,KEY_RIGHT},
    {0x3, KEY_LEFT},
    // {0x3, KEY_F1},
    {0x4, KEY_L},  //
    
    {0x5, KEY_R},     //null
    {6, KEY_Y},       //null
    {7, KEY_Y},     //power off
    {8, KEY_B},     //search  KEY_A
    
    {9, KEY_A},
    {10, KEY_X},
    {11, KEY_L},    // null
    {12, KEY_R},
    {13, 0x00},
};
#endif
extern suspend_state_t get_suspend_state(void);


static struct rkxx_remotectl_button remotectl_button[] = 
{
  /*  {  
       .usercode = 0x202, 
       .nbuttons =  22, 
       .key_table = &remote_key_table_meiyu_202[0],
    },
    {  
       .usercode = 0xdf, 
       .nbuttons =  16, 
       .key_table = &remote_key_table_df[0],
    },
    {  
       .usercode = 0xff, 
       .nbuttons =  16, 
       .key_table = &remote_key_table_ff[0],
    },*/

    {  
       .usercode = 0xff, 
       .nbuttons =  14, 
       .key_table = &remote_key_table_ff[0],
    },
	   
};


static int remotectl_keybdNum_lookup(struct rkxx_remotectl_drvdata *ddata)
{	
    int i;	

    for (i = 0; i < sizeof(remotectl_button)/sizeof(struct rkxx_remotectl_button); i++){		
        if (remotectl_button[i].usercode == (ddata->scanData&0xFFFF)){			
            ddata->keybdNum = i;
            return 1;
        }
    }
    return 0;
}


static int remotectl_keycode_lookup(struct rkxx_remotectl_drvdata *ddata)
{	
    int i;	
    unsigned char keyData = ((ddata->scanData >> 8) & 0xff);

    for (i = 0; i < remotectl_button[ddata->keybdNum].nbuttons; i++){
        if (remotectl_button[ddata->keybdNum].key_table[i].scanCode == keyData){			
            ddata->keycode = remotectl_button[ddata->keybdNum].key_table[i].keyCode;
            return 1;
        }
    }
    return 0;
}


static unsigned int sk_ir_key_down=0;
//static int fg_0 = 0;


static int last_time=0;
static int last_left_rk=0;
static int back_left_rk=0;
static int last_left_cnt=0;




static void remotectl_do_something(unsigned long  data)
{
int   i,sk_temp;
    struct rkxx_remotectl_drvdata *ddata = (struct rkxx_remotectl_drvdata *)data;
	unsigned int skkeyvalue = 0;

      unsigned int keydata=0;
	int change_vtk = 0;
		#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY
	//	mode_vt_rk=0;
		#endif

#if 1//sk_ir
         if ((1500 < ddata->period) && (ddata->period < 1700)){
                
                ddata->scanData = 0;
                ddata->count = 0;
                ddata->state = 10;  //head ok
            }else{
           
                    switch (ddata->state)
                	{
                	      case 10:
				           ddata->scanData <<= 1;
                                       ddata->count ++;
                                    // if ((1000 < ddata->period) && (ddata->period < 1300))
                                    if ((1180 < ddata->period) && (ddata->period < 1320))
					{
                                        ddata->scanData |= 0x01;
                                     }
						


                                 if(ddata->count ==13)
                                 	{
                                    	//ddata->state =11;
						keydata=ddata->scanData;		
										
						
                                    //	printk("datt=%x \n",keydata);	

						#if 0 //def SYSCONFIG_VIRTUAL_TOUCH_KEY

						if(SKVmt_enable())
						 {

						 	sk_vts_set(1,1);
						
						}
						#endif
//=================================
                                          
						if(SKVmt_enable())
						{

								 unsigned int rkkeydata=(keydata&0x0f);

								// printk("up  rkkeydata =%0x  \n",rkkeydata);
                                            if(rkkeydata&0x0f)//0-3 key
                                            {
                                                       if((rkkeydata&0x01)==rkkeydata) //down
                                                  	{
                                                  		mode_vt_rk = M_DOWN;
                                                  	}else  if((rkkeydata&0x02)==rkkeydata) //up
                                                  	{
                                                  		mode_vt_rk = M_UP;
                                                  	}else if((rkkeydata&0x04)==rkkeydata)  //righ
                                                  	{
                                                  		mode_vt_rk = M_RIGHT;
                                                  	}
                                                  	else if((rkkeydata&0x08)==rkkeydata)  //left
                                                  	{
                                                  		mode_vt_rk = M_LEFT;
                                                  	
                                                  	}else  if((rkkeydata&0x09)==rkkeydata) //up
                                                  	{
                                                  		mode_vt_rk = M_LEFT_DOWN;
                                                  	}else if((rkkeydata&0x0a)==rkkeydata)  //righ
                                                  	{
                                                  		mode_vt_rk = M_LEFT_UP;
                                                  	}
                                                  	else if((rkkeydata&0x06)==rkkeydata)  //left
                                                  	{
                                                  		mode_vt_rk = M_RIGHT_UP;
                                                  	
                                                  	}else if((rkkeydata&0x05)==rkkeydata)  //left
                                                  	{
                                                  		mode_vt_rk = M_RIGHT_DOWN;
                                                  	}

													
													
                                            }else{
                                                                              mode_vt_rk=0;
										
										//	last_left_cnt=0;
						}

					       SKSetRK(mode_vt_rk,0xff);	
										
#if 0

                                                    int mx=jiffies-last_time;

                                              //   if(last_time)
                                          //	{
                                       			//	printk("22  time  = %d \n",mx);
                                          //	}
							if(last_left_cnt==0)
							{
							     if(mode_vt_rk)
							     	{
							     	      last_left_rk=mode_vt_rk;
							            last_left_cnt=1;	
                                                        //   printk(" lost = %d \n",last_left_rk);										
							     	}
							}else if(last_left_cnt==1){
							             if(mx<8)
					                          	{
					                          	    last_left_rk=0;					                          	   
					                          	}else{
					                          	  int temp_rk=last_left_rk;
									   last_left_rk=0;
					                          	  SKSetRK(temp_rk,0xff);		
                                                              //   printk("add rk = %d \n",temp_rk);													  
					                          	}
								      SKSetRK(mode_vt_rk,0xff);	
                                                           //  printk("cur rk = %d \n",mode_vt_rk);											  
									  last_left_cnt=2;
									            	back_left_rk= 0;        
							}else{
                                                         if(mode_vt_rk)
                                                     	{
                                                     	      if(back_left_rk)
                                                     	      	{
                                                     	      	//       if(mx>8)
                                                     	      	    //	{
                                                     	      	    	       SKSetRK(back_left_rk,0xff);	
											//printk("3 addpu  rk = %d \n",mode_vt_rk);	
                                                     	      	    //	}
                                                     	      	}
						                    back_left_rk=mode_vt_rk;						                 
                                                     	    
                                                     	}else{
                                                     	      if((mx>8)&&back_left_rk)
                                                     	      	{
                                                     	      	   SKSetRK(back_left_rk,0xff);
					                               back_left_rk=0;
                                                     	      	}
                                                     	   SKSetRK(mode_vt_rk,0xff);
								  //  printk("4up  rk = %d \n",mode_vt_rk);								   
                                                     	}
														
                                                							   
							   if(mode_vt_rk==0) 
							   	{
							   	    last_left_cnt=0;
										last_time=0;	
										back_left_rk=0;
							   	}
							}
                                                
                                                                          											
							last_time=jiffies;
                                               
										       
                                  //  last_left_rk=mode_vt_rk;
		
				//	printk("22  mode_vt_rk  = %0x \n",mode_vt_rk);
								  #endif

						}
//=================================

							for(i=0;i<=12;i++)
							{

								//skkeyvalue |= ddata->scanData;
						               sk_temp=(0x01<<i);
						                if(keydata&sk_temp)
						                {
						                
									
								   	
									
										  
						                        if(sk_ir_key_down&sk_temp)
						                       {
						                       

						                       }else{
						                       
						                           sk_ir_key_down|=sk_temp;

									//printk("up  skkeyvalue =%0x  \n");

									#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY
												change_vtk=1;  
									#endif


									if(SKVmt_enable()==0)
									{
										//printk("000   ddata->keycode  =%d    \n",remotectl_button[0].key_table[i].keyCode);
						                           input_event(ddata->input, EV_KEY,  remotectl_button[0].key_table[i].keyCode, 1);
						                           input_sync(ddata->input);
									}else{
								

									     gp_vts_shoot(1, remotectl_button[0].key_table[i].keyCode, 0, 1);   

									}		  

									#endif

						                        
						                    
							               }
						                 }else{
						                       if(sk_ir_key_down&sk_temp)
						                       {
						                                 sk_ir_key_down&=(~sk_temp);


									#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY

											change_vtk=1;  
									#endif

											#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY
											//mode_vt_rk=0;
											#endif


										if(SKVmt_enable()==0)
										{
											//printk(" 1111  ddata->keycode  =%d    \n",remotectl_button[0].key_table[i].keyCode);
						                                     input_event(ddata->input, EV_KEY, remotectl_button[0].key_table[i].keyCode, 0);
						                           		input_sync(ddata->input);
										}else{												
									                    gp_vts_shoot(1, remotectl_button[0].key_table[i].keyCode, 0, 0);   
										}
						                                  
                                
								                            //  printk("Up=%x, \n",key_map[i]);
						                       }
						                 }




										 
							 }//for




									

									#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY

									
									if(SKVmt_enable())
									{
											if(change_vtk)
												{
													//sk_vts_set(1,0);
												}

									}
									#endif





					
                                 	}
					 

								 

                             
				     break;
					 
				default:
					ddata->state =0;
					
					break;
                	     
                	}
            }



#if 0
    switch (ddata->state)
    {
        case RMC_IDLE:
        {
            ;
        }
        break;
        
        case RMC_PRELOAD:
        {
            if ((TIME_PRE_MIN < ddata->period) && (ddata->period < TIME_PRE_MAX)){
                
                ddata->scanData = 0;
                ddata->count = 0;
                ddata->state = RMC_USERCODE;
            }else{
                ddata->state = RMC_PRELOAD;
            }
            ddata->pre_time = ddata->cur_time;
            //mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
        }
        break;
        
        case RMC_USERCODE:
        {
            ddata->scanData <<= 1;
            ddata->count ++;

            if ((TIME_BIT1_MIN < ddata->period) && (ddata->period < TIME_BIT1_MAX)){
                ddata->scanData |= 0x01;
            }

            if (ddata->count == 0x10){//16 bit user code
               //printk("u=0x%x\n",((ddata->scanData)&0xFFFF));
                if (remotectl_keybdNum_lookup(ddata)){
                    ddata->state = RMC_GETDATA;
                    ddata->scanData = 0;
                    ddata->count = 0;
                }else{                //user code error
                    ddata->state = RMC_PRELOAD;
                }
            }
        }
        break;
        
        case RMC_GETDATA:
        {
            ddata->count ++;
            ddata->scanData <<= 1;

          
            if ((TIME_BIT1_MIN < ddata->period) && (ddata->period < TIME_BIT1_MAX)){
                ddata->scanData |= 0x01;
            }           
            if (ddata->count == 0x10){
               printk(KERN_ERR "d=%x\n", ((ddata->scanData >> 8) & 0xff));

                if ((ddata->scanData&0x0ff) == ((~ddata->scanData >> 8)&0x0ff)){
                    if (remotectl_keycode_lookup(ddata)){
                        ddata->press = 1;
                         if (get_suspend_state()==0){
                                input_event(ddata->input, EV_KEY, ddata->keycode, 1);
                                input_sync(ddata->input);
                            }else if ((get_suspend_state())&&(ddata->keycode==KEY_POWER)){
                                input_event(ddata->input, EV_KEY, KEY_WAKEUP, 1);
                                input_sync(ddata->input);
                            }
                        //input_event(ddata->input, EV_KEY, ddata->keycode, ddata->press);
		                //input_sync(ddata->input);
                        ddata->state = RMC_SEQUENCE;
                    }else{
                        ddata->state = RMC_PRELOAD;
                    }
                }else{
                    ddata->state = RMC_PRELOAD;
                }
            }
        }
        break;
             
        case RMC_SEQUENCE:{

            //printk( "S\n");
            
            if ((TIME_RPT_MIN < ddata->period) && (ddata->period < TIME_RPT_MAX)){
                ;
            }else if ((TIME_SEQ_MIN < ddata->period) && (ddata->period < TIME_SEQ_MAX)){
	            if (ddata->press == 1){
                    ddata->press = 3;
                }else if (ddata->press & 0x2){
                    ddata->press = 2;
                //input_event(ddata->input, EV_KEY, ddata->keycode, 2);
		            //input_sync(ddata->input);
                }
                //mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));
                //ddata->state = RMC_PRELOAD;
            }
        }
        break;
       
        default:
            break;
    } 
	#endif
	return;
}


#ifdef CONFIG_PM
void remotectl_wakeup(unsigned long _data)
{
    struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)_data;
    long *time;
    int i;
    
    time = ddata->remotectl_suspend_data.scanTime;

    if (get_suspend_state()){
        
        static int cnt;
       
        ddata->remotectl_suspend_data.suspend_flag = 0;
        ddata->count = 0;
        ddata->state = RMC_USERCODE;
        ddata->scanData = 0;
        
        for (i=0;i<ddata->remotectl_suspend_data.cnt;i++){
            if (((TIME_BIT1_MIN<time[i])&&(TIME_BIT1_MAX>time[i]))||((TIME_BIT0_MIN<time[i])&&(TIME_BIT0_MAX>time[i]))){
                cnt = i;
                break;;
            }
        }
        
        for (;i<cnt+32;i++){
            ddata->scanData <<= 1;
            ddata->count ++;

            if ((TIME_BIT1_MIN < time[i]) && (time[i] < TIME_BIT1_MAX)){
                ddata->scanData |= 0x01;
            }
            
            if (ddata->count == 0x10){//16 bit user code
                          
                if (ddata->state == RMC_USERCODE){
//                    printk(KERN_ERR "d=%x\n",(ddata->scanData&0xFFFF));  
                    if (remotectl_keybdNum_lookup(ddata)){
                        ddata->scanData = 0;
                        ddata->count = 0;
                        ddata->state = RMC_GETDATA;
                    }else{
                        ddata->state = RMC_PRELOAD;
                    }
                }else if (ddata->state == RMC_GETDATA){
                    if ((ddata->scanData&0x0ff) == ((~ddata->scanData >> 8)&0x0ff)){
//                        printk(KERN_ERR "d=%x\n",(ddata->scanData&0xFFFF));
                        if (remotectl_keycode_lookup(ddata)){
                             if (ddata->keycode==KEY_POWER){
                                input_event(ddata->input, EV_KEY, KEY_WAKEUP, 1);
                                input_sync(ddata->input);
                                input_event(ddata->input, EV_KEY, KEY_WAKEUP, 0);
                                input_sync(ddata->input);
                            }
                            ddata->state = RMC_PRELOAD;
                        }else{
                            ddata->state = RMC_PRELOAD;
                        }
                    }else{
                        ddata->state = RMC_PRELOAD;
                    }
                }else{
                    ddata->state = RMC_PRELOAD;
                }
            }
        }
    }
    memset(ddata->remotectl_suspend_data.scanTime,0,50*sizeof(long));
    ddata->remotectl_suspend_data.cnt= 0; 
    ddata->state = RMC_PRELOAD;
    
}

#endif


static void remotectl_timer(unsigned long _data)
{
    struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)_data;
    
   // printk("to\n");



	
    if(ddata->press != ddata->pre_press) {
        ddata->pre_press = ddata->press = 0;

        if (get_suspend_state()==0){
            //input_event(ddata->input, EV_KEY, ddata->keycode, 1);
            //input_sync(ddata->input);
            input_event(ddata->input, EV_KEY, ddata->keycode, 0);
		    input_sync(ddata->input);
        }else if ((get_suspend_state())&&(ddata->keycode==KEY_POWER)){
            //input_event(ddata->input, EV_KEY, KEY_WAKEUP, 1);
            //input_sync(ddata->input);
            input_event(ddata->input, EV_KEY, KEY_WAKEUP, 0);
            input_sync(ddata->input);
        }
    }
#ifdef CONFIG_PM
    remotectl_wakeup(_data);
#endif
    ddata->state = RMC_PRELOAD;
}



static irqreturn_t remotectl_isr(int irq, void *dev_id)
{
    struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)dev_id;
    struct timeval  ts;

	
	
    ddata->pre_time = ddata->cur_time;
    do_gettimeofday(&ts);
    ddata->cur_time = ts.tv_usec;

	//printk("22  mode_vt_rk  = %0x \n",mode_vt_rk);
	 
    if (ddata->cur_time && ddata->pre_time)
    	{
    //	printk(" %d,    \n",ddata->cur_time - ddata->pre_time);
        ddata->period =  ddata->cur_time - ddata->pre_time;
    	}

	//printk("--------- ddata->period=%d    \n ",ddata->period);
	
    tasklet_hi_schedule(&ddata->remote_tasklet); 
   if ((ddata->state==RMC_PRELOAD)||(ddata->state==RMC_SEQUENCE))
    mod_timer(&ddata->timer,jiffies + msecs_to_jiffies(130));





#ifdef CONFIG_PM
    //wake_lock_timeout(&ddata->remotectl_wake_lock, HZ);
   if ((get_suspend_state())&&(ddata->remotectl_suspend_data.cnt<50))
       ddata->remotectl_suspend_data.scanTime[ddata->remotectl_suspend_data.cnt++] = ddata->period;
#endif

    return IRQ_HANDLED;
}





static void keys_sk_timer(unsigned long _data)
{
	//struct rk29_keys_drvdata *ddata = (struct rk29_keys_drvdata *)_data;
struct rkxx_remotectl_drvdata *ddata =  (struct rkxx_remotectl_drvdata*)_data;
	//if (!ddata->in_suspend)
		//adc_async_read(ddata->client);


//if(SKVmt_mode())
		//{

			//printk("1234    \n");
			//SKSetRK(mode_vt_rk);
	//	}
//	if(last_left_cnt

#if 0

	  
             if(last_left_cnt==1)
       	{

                       if(last_left_rk)
                       	{
                       	int mx=jiffies-last_time;
                       	printk("33  time  = %d \n",mx);
					         if(mx>10)
					         {
					         	         SKSetRK(last_left_rk,0xff);
								   last_left_rk=0;
								   if(mode_vt_rk==0)
								   	{
								   	 SKSetRK(mode_vt_rk,0xff);
								   	}
					         }
                       	}
			          
				         
       	}
			 #endif


			   //    if(last_time)
                       //   	{
                       		//printk("33  time  = %d \n",jiffies-last_time);
                     //     }
#if 0
                          if(last_left_rk)
                        {
                                   if(last_time)
                            	{
                                          if((jiffies-last_time)>200)
                                          	{
				                        SKSetRK(last_left_rk,0xff);
				                         last_left_rk=0;
							   if(mode_vt_rk==0)
							   	{
							   	   SKSetRK(mode_vt_rk,0xff);
							   	}
                                          	}
                            	}
                        }
#endif
				   
//	printk(" sk--- keys_sk_timer    -----=--------sk--------------\n");
	mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(30));
}


static int __devinit remotectl_probe(struct platform_device *pdev)
{
    struct RKxx_remotectl_platform_data *pdata = pdev->dev.platform_data;
    struct rkxx_remotectl_drvdata *ddata;
    struct input_dev *input;
    int i, j;
    int irq;
    int error = 0;

    printk("++++++++remotectl_probe\n");

    if(!pdata) 
        return -EINVAL;

    ddata = kzalloc(sizeof(struct rkxx_remotectl_drvdata),GFP_KERNEL);
    memset(ddata,0,sizeof(struct rkxx_remotectl_drvdata));

    ddata->state = RMC_PRELOAD;
    input = input_allocate_device();
    
    if (!ddata || !input) {
        error = -ENOMEM;
        goto fail0;
    }

    platform_set_drvdata(pdev, ddata);

    input->name = pdev->name;
    input->phys = "gpio-keys/input0";
    input->dev.parent = &pdev->dev;

    input->id.bustype = BUS_HOST;
    input->id.vendor = 0x0001;
    input->id.product = 0x0001;
    input->id.version = 0x0100;

	/* Enable auto repeat feature of Linux input subsystem */
	if (pdata->rep)
		__set_bit(EV_REP, input->evbit);
    
	ddata->nbuttons = pdata->nbuttons;
	ddata->input = input;
  wake_lock_init(&ddata->remotectl_wake_lock, WAKE_LOCK_SUSPEND, "rk29_remote");
  if (pdata->set_iomux){
  	pdata->set_iomux();
  }





		
  error = gpio_request(pdata->gpio, "remotectl");
	if (error < 0) {
		printk("gpio-keys: failed to request GPIO %d,"
		" error %d\n", pdata->gpio, error);
		//goto fail1;
	}
	error = gpio_direction_input(pdata->gpio);
	if (error < 0) {
		pr_err("gpio-keys: failed to configure input"
			" direction for GPIO %d, error %d\n",
		pdata->gpio, error);
		gpio_free(pdata->gpio);
		//goto fail1;
	}
    irq = gpio_to_irq(pdata->gpio);
	if (irq < 0) {
		error = irq;
		pr_err("gpio-keys: Unable to get irq number for GPIO %d, error %d\n",
		pdata->gpio, error);
		gpio_free(pdata->gpio);
		goto fail1;
	}
	
	error = request_irq(irq, remotectl_isr,	IRQF_TRIGGER_FALLING , "remotectl", ddata);
	
	if (error) {
		pr_err("gpio-remotectl: Unable to claim irq %d; error %d\n", irq, error);
		gpio_free(pdata->gpio);
		goto fail1;
	}
    setup_timer(&ddata->timer,remotectl_timer, (unsigned long)ddata);
    
    tasklet_init(&ddata->remote_tasklet, remotectl_do_something, (unsigned long)ddata);
    
    for (j=0;j<sizeof(remotectl_button)/sizeof(struct rkxx_remotectl_button);j++){ 
    	printk("remotectl probe j=0x%x\n",j);
		for (i = 0; i < remotectl_button[j].nbuttons; i++) {
			unsigned int type = EV_KEY;
	        
			input_set_capability(input, type, remotectl_button[j].key_table[i].keyCode);
		}
  }
	error = input_register_device(input);
	if (error) {
		pr_err("gpio-keys: Unable to register input device, error: %d\n", error);
		goto fail2;
	}

	
  		setup_timer(&ddata->timer, keys_sk_timer, (unsigned long)ddata);
		
		mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(100));

    
    input_set_capability(input, EV_KEY, KEY_WAKEUP);

	device_init_wakeup(&pdev->dev, 1);

	return 0;

fail2:
    pr_err("gpio-remotectl input_allocate_device fail\n");
	input_free_device(input);
	kfree(ddata);
fail1:
    pr_err("gpio-remotectl gpio irq request fail\n");
    free_irq(gpio_to_irq(pdata->gpio), ddata);
    del_timer_sync(&ddata->timer);
    tasklet_kill(&ddata->remote_tasklet); 
    gpio_free(pdata->gpio);
fail0: 
    pr_err("gpio-remotectl input_register_device fail\n");
    platform_set_drvdata(pdev, NULL);

	return error;
}

static int __devexit remotectl_remove(struct platform_device *pdev)
{
	struct RKxx_remotectl_platform_data *pdata = pdev->dev.platform_data;
	struct rkxx_remotectl_drvdata *ddata = platform_get_drvdata(pdev);
	struct input_dev *input = ddata->input;
    int irq;

	device_init_wakeup(&pdev->dev, 0);
    irq = gpio_to_irq(pdata->gpio);
    free_irq(irq, ddata);
    tasklet_kill(&ddata->remote_tasklet); 
    gpio_free(pdata->gpio);

	input_unregister_device(input);

	return 0;
}


#ifdef CONFIG_PM
static int remotectl_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct RKxx_remotectl_platform_data *pdata = pdev->dev.platform_data;
    struct rkxx_remotectl_drvdata *ddata = platform_get_drvdata(pdev);
    
    //ddata->remotectl_suspend_data.suspend_flag = 1;
    ddata->remotectl_suspend_data.cnt = 0;

	if (device_may_wakeup(&pdev->dev)) {
		if (pdata->wakeup) {
			int irq = gpio_to_irq(pdata->gpio);
			enable_irq_wake(irq);
		}
	}
    
	return 0;
}

static int remotectl_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct RKxx_remotectl_platform_data *pdata = pdev->dev.platform_data;
    
    if (device_may_wakeup(&pdev->dev)) {
        if (pdata->wakeup) {
            int irq = gpio_to_irq(pdata->gpio);
            disable_irq_wake(irq);
        }
    }

	return 0;
}

static const struct dev_pm_ops remotectl_pm_ops = {
	.suspend	= remotectl_suspend,
	.resume		= remotectl_resume,
};
#endif



static struct platform_driver remotectl_device_driver = {
	.probe		= remotectl_probe,
	.remove		= __devexit_p(remotectl_remove),
	.driver		= {
		.name	= "rkxx-remotectl",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
	    .pm	= &remotectl_pm_ops,
#endif
	},

};

static int  remotectl_init(void)
{
    printk(KERN_INFO "++++++++remotectl_init\n");
    return platform_driver_register(&remotectl_device_driver);
}


static void  remotectl_exit(void)
{
	platform_driver_unregister(&remotectl_device_driver);
    printk(KERN_INFO "++++++++remotectl_init\n");
}

module_init(remotectl_init);
module_exit(remotectl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rockchip");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys1");


