/************************************************************************** 
 *                                                                        *
 *         Copyright (c) 2012 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/** 
 * @file touch_virtual.c
 * @brief virtual touch screen driver
 * @author Anson Chuang
 */
/*#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/typedef.h>

*/
#include <linux/module.h>

#include <linux/delay.h>
#include <linux/input.h>
#include <mach/board.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/freezer.h>



#define LCD_WIDTH   1024
#define LCD_HEIGHT  600



#define MULTI_TP_POINTS 17 // 15key + 2 joystick
#define MULTI_VTP_POINTS 15

#define  JOY_LEFT_POS  (MULTI_TP_POINTS-2)
#define  JOY_RIGHT_POS  (MULTI_TP_POINTS-1)


#define GP_VPT_TYPE 'v'

#define GP_VPT_ENABLE        _IOW(GP_VPT_TYPE, 0x00, int)
#define GP_VPT_SET_MAP        _IOW(GP_VPT_TYPE, 0x01, int)
#define GP_VPT_GET_MAP        _IOR(GP_VPT_TYPE, 0x02, int)


#define     	direct_left          (0x01<<9)
#define		direct_down 	(0x01<<10)
#define     	direct_right         (0x01<<11)
#define		direct_up		(0x01<<12)

#define     	direct_leftdown          ((0x01<<9)|(0x01<<10))
#define		direct_leftup   	 ((0x01<<9)|(0x01<<12))
#define		direct_rightdown   	 ((0x01<<11)|(0x01<<10))
#define		direct_rightup   	 ((0x01<<11)|(0x01<<12))

//int   	skrock_x = 500;
//int  	skrock_y = 400;

//int 	skrock_linex;
//int 	skrock_liney;
/* For debug : print touch value */
//#define DEBUG_PRINT_TOUCH_VALUES

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
 
  /** @brief A structure of representing a point, the position */
typedef struct gp_point_s {
	int16_t x; /*!< @brief The x value of the point structure */
	int16_t y; /*!< @brief The y value of the point structure */
} gp_point_t;




typedef struct gp_tp_s {
	struct input_dev *dev;
	int prev_touched;
}gp_tp_t;

typedef struct multi_touch_data_s {
	int index;
	int pressed;
	gp_point_t points[MULTI_TP_POINTS];
	int id[MULTI_TP_POINTS];
	struct timer_list timer;
	
} multi_touch_data_t;

typedef struct virtual_key_touch_map_item_s {
	int keyCode;
	gp_point_t point;
	int id;
} virtual_key_mapping_t;

typedef struct virtual_tracecircle_location_s {
	int x;
	int y;
	int diameter;
	int id;
} virtual_joystick_mapping_t;

typedef struct virtual_touch_map_s {
	virtual_key_mapping_t key[MULTI_VTP_POINTS];
	virtual_joystick_mapping_t joystick;		
	virtual_joystick_mapping_t joystick_r;		
	int vk_sum;
} virtual_touch_map_t;

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 1
	#define DEBUG	printk
#else
	#define DEBUG(...)
#endif

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_tp_t ts;

static int vmt_enable;

static int vmt_mode;


static multi_touch_data_t multi_touch_data;


static int fg_rep_ing=0; 

static virtual_touch_map_t vts_mapping;
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
/**
 * @brief touch panel request fucntion
 * @return : handle of the requested touch panel(<0 invalid handle)
 */
int gp_tp_request(void)
{
	return (int)(&ts);
}

/**
 * @brief touch panel free fucntion
 * @param handle[in] : handle of touch panel to release
 * @return : SP_OK(0)/ERROR_ID
 */
int gp_tp_release(int handle)
{
	return 0;
}



void gp_vts_report(void);
static int last_pos=0;

#define M_CENTER  0







#define  DIR_S  0
#define  DIR_F  1


	//nX = vts_mapping.joystick.x + (x * vts_mapping.joystick.diameter) / 1024;

#define X_POS  (vts_mapping.joystick.x+(vts_mapping.joystick.diameter/2))
//#define Y_POS  (vts_mapping.joystick.y -(vts_mapping.joystick.diameter/2))
#define Y_POS  (vts_mapping.joystick.y - (vts_mapping.joystick.diameter/2) )
#define R_LEN  (vts_mapping.joystick.diameter/2)





static int skrock_x=0;
static int skrock_y=0;


static int skrock_x2=0;
static int skrock_y2=0;


#define setp_skrock  1

#if 0
	{SKKEY_DOWN}, //1
	  {SKKEY_UP}, 	 	//2
    {SKKEY_LEFT}, 	//3
    {SKKEY_RIGHT}, 	//4
    {SKKEY_LEFT|SKKEY_DOWN}, 	 //5  
    {SKKEY_RIGHT|SKKEY_DOWN}, 	//6
    {SKKEY_LEFT|SKKEY_UP}, 	//7    
    {SKKEY_RIGHT|SKKEY_UP}     //8       
#endif


#define LEFT_AR    {-1,0}   	
#define RIGHT_AR    {1,0}    
#define UP_AR       {0,1}   
#define DOWN_AR     {0,-1}   
#define LEFT_UP_AR  {-1,1}   
#define LEFT_DOWN_AR  {-1,-1}   
#define RIGHT_UP_AR  {1,1}   
#define RIGHT_DOWN_AR  {1,-1}  

int leftjk_pos_arry[9][2]=
{
	  {0,0}, //null
	 DOWN_AR,  	  
	 UP_AR,
	 LEFT_AR,
	 RIGHT_AR,
	 LEFT_DOWN_AR,
	 RIGHT_DOWN_AR,
	 LEFT_UP_AR,
	 RIGHT_UP_AR    
};




int rightjk_pos_arry[9][2]=
{
	  {0,0}, //null
	  {-1,0}, //left 	   	  
	  {1,0}, //right
	  {0,-1}, //up	  
	  {0,1}, //down  
    {-1,-1}, //left_up	  
    {-1,1}, //left_down	 
    {1,-1}, //right_up	  
    {1,1} //right_down	            
};

int get_joy_view_mode()
{
	return vts_mapping.joystick_r.id;
}
EXPORT_SYMBOL(get_joy_view_mode);




void SKSetRK(int direct_skrock,int direct_skrock2)
{
			int nX, nY;

			if(direct_skrock2!=0xff)
				{
         if(direct_skrock2==0)
         	{
         		   // skrock_x2=vts_mapping.joystick_r.x;
	       		 //   skrock_y2=vts_mapping.joystick_r.y;     		    
	       			//	multi_touch_data.points[JOY_RIGHT_POS].x = vts_mapping.joystick_r.x;
				      //  multi_touch_data.points[JOY_RIGHT_POS].y = vts_mapping.joystick_r.y;
					#ifdef  CONFIG_MACH_RK3188_SKQ627
				   	if((multi_touch_data.id[JOY_RIGHT_POS] ==0)&&(direct_skrock==0xff))
				   		{
						   return;
				   		}
				     #endif
						
				        multi_touch_data.id[JOY_RIGHT_POS] =0x00;  //vts_mapping.joystick.id;		

					
         	}else{
         	        
         			//	printk("\r\ndirect_skrock=%d \r\n",direct_skrock2);
         			if(multi_touch_data.id[JOY_RIGHT_POS] ==0x00)
         				{
         							    skrock_x2=vts_mapping.joystick_r.x;
	       		              skrock_y2=vts_mapping.joystick_r.y;  
         				}

			#ifdef CONFIG_MACH_RK3188_SKQ627
			skrock_x2 =skrock_x2+rightjk_pos_arry[direct_skrock2][0]*vts_mapping.joystick_r.diameter;
			skrock_y2 =skrock_y2+rightjk_pos_arry[direct_skrock2][1]*vts_mapping.joystick_r.diameter;
			#else
                	skrock_x2 =skrock_x2+rightjk_pos_arry[direct_skrock2][0]*vts_mapping.joystick_r.diameter*4;
			skrock_y2 =skrock_y2+rightjk_pos_arry[direct_skrock2][1]*vts_mapping.joystick_r.diameter*4;
			#endif
						    
						 //    		printk("11R_x=[%d ,y=%d],   ",skrock_x2,skrock_y2);		
						     if(skrock_x2<0)   skrock_x2=0;
						     if(skrock_x2>=LCD_WIDTH)   skrock_x2=LCD_WIDTH-1;		
						     	if(skrock_y2<0) skrock_y2=0;	
						     if(skrock_y2>=LCD_HEIGHT) skrock_y2=LCD_HEIGHT-1;	
						     	
						     	
						     	//	printk("22R_x=[%d ,y=%d],   \r\n",skrock_x2,skrock_y2);				
         				multi_touch_data.points[JOY_RIGHT_POS].x = skrock_x2;
						    multi_touch_data.points[JOY_RIGHT_POS].y = skrock_y2;
						    multi_touch_data.id[JOY_RIGHT_POS]=JOY_RIGHT_POS;  //vts_mapping.joystick.id;				
         		
         	}


			 
				}

			
        if(direct_skrock!=0xff)
        	{
	       if(direct_skrock==0)
	       	{
	       		    skrock_x=0;
	       		    skrock_y=0;     		    
	       				multi_touch_data.points[JOY_LEFT_POS].x = 0;
				        multi_touch_data.points[JOY_LEFT_POS].y = 0;
				        multi_touch_data.id[JOY_LEFT_POS] =0x00;  //vts_mapping.joystick.id;		
				       // 	printk("RK UP===\r\n");
	       	}else{	     
	       	#if 1
	       		if(multi_touch_data.id[JOY_LEFT_POS]==0x00)
	       		{	       		
	       	//	printk("RK DOWN===\r\n");
						            skrock_x=0;
					                   skrock_y=0;   
							      multi_touch_data.points[JOY_LEFT_POS].x = vts_mapping.joystick.x;
								multi_touch_data.points[JOY_LEFT_POS].y = vts_mapping.joystick.y;
								multi_touch_data.id[JOY_LEFT_POS] =JOY_LEFT_POS;  //vts_mapping.joystick.id;	
							
											gp_vts_report();	
								
							//	msleep(5);
}
				#endif
					  					  					  
							#ifdef  CONFIG_MACH_RK3188_SKQ627						
			                    if(multi_touch_data.id[JOY_LEFT_POS])
			                    	{
			                    		multi_touch_data.id[JOY_LEFT_POS] =0;
									  gp_vts_report();	
			                    	}
							#endif	
						if(direct_skrock>4)
							{
										nX = vts_mapping.joystick.x + (leftjk_pos_arry[direct_skrock][0]*setp_skrock* vts_mapping.joystick.diameter*4/3) / (setp_skrock*2);
					        	nY = vts_mapping.joystick.y - (leftjk_pos_arry[direct_skrock][1]*setp_skrock* vts_mapping.joystick.diameter*4/3) / (setp_skrock*2);							
							}else{
				       	   nX = vts_mapping.joystick.x + (leftjk_pos_arry[direct_skrock][0]*setp_skrock* vts_mapping.joystick.diameter*2) / (setp_skrock*2);
						       nY = vts_mapping.joystick.y - (leftjk_pos_arry[direct_skrock][1]*setp_skrock* vts_mapping.joystick.diameter*2) / (setp_skrock*2);
					         
					    }
						
	
						multi_touch_data.points[JOY_LEFT_POS].x = nX;
						multi_touch_data.points[JOY_LEFT_POS].y = nY;
						multi_touch_data.id[JOY_LEFT_POS] =JOY_LEFT_POS;  //vts_mapping.joystick.id;									
     }               
 //  printk("direct_skrock=%x x=%d,y=%d\n",direct_skrock,multi_touch_data.points[JOY_LEFT_POS].x,multi_touch_data.points[JOY_LEFT_POS].y);               									
        	}
	
                             gp_vts_report();		
		
		
}
EXPORT_SYMBOL(SKSetRK);




int SKVmt_enable(void)
{
	return vmt_enable;
}

EXPORT_SYMBOL(SKVmt_enable);



int SKVmt_mode(void)
{

	return vmt_mode;
}

EXPORT_SYMBOL(SKVmt_mode);



void gp_vts_shoot(int type,int x,int y,int fg_down)
{
	
			int i;
	#if 0
//int  j;

    printk("vk_sum=%d \r\n",vts_mapping.vk_sum);
		for(i=0;i<MULTI_VTP_POINTS;i++)
		{

       printk("vt key[%d]=%d id=%d, x=%d,y=%d \r\n",i,vts_mapping.key[i].keyCode,vts_mapping.key[i].id,vts_mapping.key[i].point.x,vts_mapping.key[i].point.y);
    }


  printk("vt LJOY id=%d, d=%d, x=%d,y=%d \r\n",vts_mapping.joystick.id,vts_mapping.joystick.diameter,vts_mapping.joystick.x,vts_mapping.joystick.y);
  printk("vt LJOY  id=%d,d=%d, x=%d,y=%d \r\n",vts_mapping.joystick_r.id,vts_mapping.joystick_r.diameter ,vts_mapping.joystick_r.x, vts_mapping.joystick_r.y);
//  printk("vt LJOY id=%d d=%d, x=%d,y=%d \r\n",vts_mapping.joystick3.id,vts_mapping.joystick3.diameter ,vts_mapping.joystick3.x, vts_mapping.joystick3.y);     
return;
   
     #endif
   
  
 

  if (vmt_enable == 0) 
  {
		return;
	}
	if (type == 0) {  //_RK
	
	//	SKSetRK(x,y);
	}
	else if (type == 1) {
		/* Key */


		for (i=0; i<vts_mapping.vk_sum; i++) {			 
			   if (x == vts_mapping.key[i].keyCode)
			  {			
			      multi_touch_data.points[i].x=vts_mapping.key[i].point.x;
			      multi_touch_data.points[i].y=vts_mapping.key[i].point.y;
			      if(fg_down)
			      {
			         multi_touch_data.id[i]=vts_mapping.key[i].id;	
					// printk("key down===\r\n");
			      }else{
			      	 multi_touch_data.id[i]=0x00;		
					 //	 printk("key UP===\r\n");
			      }																
				    break;					
			}
		}
	}
}
EXPORT_SYMBOL(gp_vts_shoot);


void sk_vts_set(int type,int fg_start)
{
	  if(type==1)
	  {	  		
	  		  if(fg_start)
	  		  { 
	  		   		  		
	  		  }else{	  		
	  		
	  		  //	   printk("key tp report22\r\n");
	  		  	   gp_vts_report();	
	  		  		
	  		   	}
	  }
 
}
EXPORT_SYMBOL(sk_vts_set);



void gp_vts_report(void)
{

	int i;
	
	if(fg_rep_ing)
		{
//	printk("************start1********* ");			
		
		 return;
		}

    if (vmt_enable == 0) {
		return;
	}	
	
	


	fg_rep_ing=1;
	
//	printk("start2 ");
	
	multi_touch_data.pressed=0;
	for (i = 0; i <vts_mapping.vk_sum; i++) {
	  if(multi_touch_data.id[i])
	  	{
	  		
			//	printk("key:%d, ",multi_touch_data.id[i]);	  				
	  		  multi_touch_data.pressed++;	  	
	  			input_report_abs(ts.dev, ABS_MT_TRACKING_ID, multi_touch_data.id[i]);
	        input_report_abs(ts.dev, ABS_MT_POSITION_X, multi_touch_data.points[i].x);
	        input_report_abs(ts.dev,ABS_MT_POSITION_Y, multi_touch_data.points[i].y);
	        input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 7);
	        input_mt_sync(ts.dev);		  								
				//	multi_touch_data.id[i]=0xff;
			
		}
	}
	
//==========left joystick
      if(multi_touch_data.id[JOY_LEFT_POS])
	  	{
	  				//printk(" LK****:%d, ",multi_touch_data.id[i]);
	  //	printk("LK***%d,x:%d,y=%d",multi_touch_data.id[JOY_LEFT_POS],multi_touch_data.points[JOY_LEFT_POS].x,multi_touch_data.points[JOY_LEFT_POS].y);
	  		  multi_touch_data.pressed++;	  	
	  			input_report_abs(ts.dev, ABS_MT_TRACKING_ID, multi_touch_data.id[JOY_LEFT_POS]);
	        input_report_abs(ts.dev, ABS_MT_POSITION_X, multi_touch_data.points[JOY_LEFT_POS].x);
	        input_report_abs(ts.dev,ABS_MT_POSITION_Y, multi_touch_data.points[JOY_LEFT_POS].y);
	        input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 7);
	        input_mt_sync(ts.dev);	
		
		//     	printk("LK***%d,x:%d,y=%d",multi_touch_data.id[JOY_LEFT_POS],multi_touch_data.points[JOY_LEFT_POS].x,multi_touch_data.points[JOY_LEFT_POS].y);
		  }
	//==========right joystick
      if(multi_touch_data.id[JOY_RIGHT_POS])
	  	{
	  	//	printk("RK:%d,  x:%d, y:%d \r\n ",multi_touch_data.id[JOY_RIGHT_POS],multi_touch_data.points[JOY_RIGHT_POS].x,multi_touch_data.points[JOY_RIGHT_POS].y);
	  		  multi_touch_data.pressed++;	  	
	  			input_report_abs(ts.dev, ABS_MT_TRACKING_ID, multi_touch_data.id[JOY_RIGHT_POS]);
	        input_report_abs(ts.dev, ABS_MT_POSITION_X, multi_touch_data.points[JOY_RIGHT_POS].x);
	        input_report_abs(ts.dev,ABS_MT_POSITION_Y, multi_touch_data.points[JOY_RIGHT_POS].y);
	        input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 7);
	        input_mt_sync(ts.dev);		  															
		  }
	


	if (ts.prev_touched && multi_touch_data.pressed == 0) // touch up
	{
		input_mt_sync(ts.dev);

		
	}
	if (ts.prev_touched == 0 && multi_touch_data.pressed == 0){ //no touch
	}
	else {//touch down.
		input_sync(ts.dev);
	
	}
		//printk("\r\ntp down==%d***\r\n ",multi_touch_data.pressed);
	ts.prev_touched = multi_touch_data.pressed;
  fg_rep_ing=0;
}
EXPORT_SYMBOL(gp_vts_report);



/** device driver probe*/
static int __init gp_vtp_probe(struct platform_device *pdev)
{
	int ret = 0;

//	DIAG_DEBUG("Entering %s\n", __FUNCTION__);

	memset(&ts, 0, sizeof(gp_tp_t));
	vmt_enable = 0;


	
	memset(&multi_touch_data, 0, sizeof(multi_touch_data_t));

	ts.dev = input_allocate_device();


  


    platform_set_drvdata(pdev, &multi_touch_data);




	if ( !ts.dev ){
	//	DIAG_ERROR("Unable to alloc input device\n");
		ret = -ENOMEM;
		goto __err_alloc;
	}

	__set_bit(EV_ABS, ts.dev->evbit);



#if (defined(CONFIG_LCD_RK3168_86V)||defined(CONFIG_MACH_RK3188_SKQ028))

	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 800 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 480 - 1, 0, 0);
#elif defined(CONFIG_LCD_SK616LVDS_1024X768)
//sdf
  input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 1024 - 1, 0, 0);
	  input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 768 - 1, 0, 0);
#else
	  input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 1024 - 1, 0, 0);
	  input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 600 - 1, 0, 0);
	
#endif

#if (defined SYSCONFIG_ANDROID_SDK_VERSION) && (SYSCONFIG_ANDROID_SDK_VERSION == 10)
	//input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_PRESSURE, 0, 1, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 0, 15, 0, 0);
#else
	input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 0, 15, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
#endif

	ts.dev->name = "gp_vts";
	ts.dev->phys = "gp_vts";
	ts.dev->id.bustype = BUS_VIRTUAL;

	/* All went ok, so register to the input system */
	if (input_register_device(ts.dev)) {
		ret = -EIO;
		goto __err_reg_input;
	}


	return 0;


__err_reg_input:
	input_free_device(ts.dev);
__err_alloc:
	return ret;
}

/** device driver remove*/
static int gp_vtp_remove(struct platform_device *pdev)
{
	input_unregister_device(ts.dev);
	input_free_device(ts.dev);
	return 0;
}

static int gp_vtp_suspend(struct platform_device *pdev, pm_message_t state)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

static int gp_vtp_resume(struct platform_device *pdev)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

/**
 * @brief   touch pad device release
 */
static void gp_vtp_device_release(struct device *dev)
{
	//DIAG_INFO("remove virtual touch pad device ok\n");
}


static int
gp_vtp_open(
	struct inode *inode,
	struct file *file
)
{
	//DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

static int
gp_vtp_release(
	struct inode *inode,
	struct file *file
)
{
	//DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

static long
gp_vtp_ioctl(
	struct file *pfile,
	unsigned int cmd,
	unsigned long arg
)
{
	void __user *argp = (void __user *)arg;
	int i;
//	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	switch (cmd) {
	case GP_VPT_ENABLE:
		if (copy_from_user(&vmt_enable, argp, sizeof(vmt_enable)))
			return -EFAULT;
	//	DEBUG("[%s:%d]enable=%d\n", __FUNCTION__, __LINE__, vmt_enable);
		//memset(&mt_mapping_data, 0x0, sizeof(multi_touch_data_t));	
		break;		
	case GP_VPT_SET_MAP:
		if (copy_from_user(&vts_mapping, argp, sizeof(vts_mapping)))
			return -EFAULT;

		//	for (i=0; i<MULTI_VTP_POINTS; i++) {
			//if (1024 == vts_mapping.key[i].keyCode) {
				
			if(vts_mapping.joystick.diameter!=0)
			{
      
                            vmt_mode = 1;   //skrock

			}else
				{
						vmt_mode = 0;   //key
			}
			
//printk("---joystick x=%d---joystick.y=%d ---%d------\n",vts_mapping.joystick.x,vts_mapping.joystick.y,vts_mapping.joystick.diameter);
//printk("---skrock2 x=%d---skrock2.y=%d ---%d------\n",vts_mapping.skrock2.x,vts_mapping.skrock2.y,vts_mapping.skrock2.diameter);					
			//}
		//}

		
	  	//if(vts_mapping)
		//DEBUG("[%s:%d]setmap!\n", __FUNCTION__, __LINE__);
		//memset(&mt_mapping_data, 0x0, sizeof(multi_touch_data_t));
		break;
	case GP_VPT_GET_MAP:
		if (copy_to_user(argp, &vts_mapping, sizeof(vts_mapping)))
			return -EFAULT;

		//DEBUG("[%s:%d]getmap!\n", __FUNCTION__, __LINE__);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}

static const struct file_operations gp_vtp_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = gp_vtp_ioctl,
	.open = gp_vtp_open,
	.release = gp_vtp_release,
};

static struct miscdevice gp_vtp_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gp_vtp",
	.fops = &gp_vtp_fops,
};

static struct platform_device gp_vtp_device = {
	.name = "gp_vtp",
	.id   = -1,
	.dev	= {
		.release = gp_vtp_device_release,
	}
};

static struct platform_driver gp_vtp_driver = {
	.driver         = {
		.name   = "gp_vtp",
		.owner  = THIS_MODULE,
	},
	.probe          = gp_vtp_probe,
	.remove         = gp_vtp_remove,
	.suspend        = gp_vtp_suspend,
	.resume         = gp_vtp_resume,
};

static int __init gp_vtp_module_init(void)
{
	int rc;

	rc = misc_register(&gp_vtp_misc_device);
	if (rc) {
		printk("[%s:%d] gp_vtp_misc_device register failed\n", __FUNCTION__, __LINE__);
		goto  misc_register_fail;
	}

	platform_device_register(&gp_vtp_device);
	rc = platform_driver_register(&gp_vtp_driver);
	if (rc) {
		printk("[%s:%d] gp_vtp_driver register failed\n", __FUNCTION__, __LINE__);
		goto  device_register_fail;		
	}
	return rc;
	
device_register_fail:
	misc_deregister(&gp_vtp_misc_device);
misc_register_fail:	
	return rc;
}

static void __exit gp_vtp_module_exit(void)
{
	misc_deregister(&gp_vtp_misc_device);
	platform_device_unregister(&gp_vtp_device);
	platform_driver_unregister(&gp_vtp_driver);
}

module_init(gp_vtp_module_init);
module_exit(gp_vtp_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("skwq <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys");
