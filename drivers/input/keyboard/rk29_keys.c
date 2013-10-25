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
#include <linux/adc.h>

#include <asm/gpio.h>
#include <mach/board.h>
#include <plat/key.h>

#define EMPTY_ADVALUE					950
#define DRIFT_ADVALUE					35//70
#define INVALID_ADVALUE 				-1
#define EV_MENU					KEY_F1

void SKSetRK(int direct_skrock,int direct_skrock2);
#if 0
#define key_dbg(bdata, format, arg...)		\
	dev_printk(KERN_INFO , &bdata->input->dev , format , ## arg)
#else
#define key_dbg(bdata, format, arg...)	
#endif


#define  SK_ROCK1   RK30_PIN3_PB1
#define  SK_ROCK2   RK30_PIN0_PC0
#define  SK_ROCK3   RK30_PIN0_PC1
#define  SK_ROCK4   RK30_PIN0_PC2
#define SYSCONFIG_VIRTUAL_TOUCH_KEY



#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY
void gp_vts_shoot(int type, int x, int y, int action);
void gp_vts_report(void);
void sk_vts_report(void);





void sk_vts_set(int type,int fg_start);
extern int SKVmt_enable(void);
//extern void SKSetRK(int cur_mod);
//extern void SKSetRK2(int);
#endif

struct rk29_button_data {
	int state;
	int long_press_count;
	struct rk29_keys_button *button;
	struct input_dev *input;
	struct timer_list timer;
        struct rk29_keys_drvdata *ddata;
};

struct rk29_keys_drvdata {
	int nbuttons;
	int result;
	bool in_suspend;	/* Flag to indicate if we're suspending/resuming */
	struct input_dev *input;
	struct adc_client *client;
	struct timer_list timer;
	struct rk29_button_data data[0];
};

static struct input_dev *input_dev;
struct rk29_keys_Arrary {
	char keyArrary[20];
};

static ssize_t rk29key_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	int i,j,start,end;
	char rk29keyArrary[400];
	struct rk29_keys_Arrary Arrary[]={
                {
					     .keyArrary = {"home"},
                      
                },
                {
                     .keyArrary = {"menu"},
                        //.keyArrary = {"f1"},
                },
                {
                        .keyArrary = {"esc"},
                },
                {
                      //  .keyArrary = {"sensor"},
                       .keyArrary = {"f1"},
                },
                {
                        .keyArrary = {"play"},
                },
                {
                        .keyArrary = {"vol+"},
                },
                {
                        .keyArrary = {"vol-"},
                },
                {
                        .keyArrary = {"select"},
                },
                {
                        .keyArrary = {"start"},
                },
                {
                        .keyArrary = {"a"},
                },
                {
                        .keyArrary = {"b"},
                },
                {
                        .keyArrary = {"x"},
                },
                {
                        .keyArrary = {"y"},
                },


        }; 
	char *p;
	  
	for(i=0;i<9+4;i++)
	{
		
		p = strstr(buf,Arrary[i].keyArrary);
		if(p==0)
              {
                   dev_dbg(dev," rk29key_set p == 0 error ...............\n");
                   continue;
              }
		start = strcspn(p,":");
		
		if(i<8+4)
			end = strcspn(p,",");
		else
			end = strcspn(p,"}");
	
		memset(rk29keyArrary,0,sizeof(rk29keyArrary));
		
		strncpy(rk29keyArrary,p+start+1,end-start-1);
							 		
		for(j=0;j<9+4;j++)
		{		
			if(strcmp(pdata->buttons[j].desc,Arrary[i].keyArrary)==0)
			{

			if(strcmp(rk29keyArrary,"HOME")==0)
					pdata->buttons[j].code = KEY_HOME;
				else if(strcmp(rk29keyArrary,"MENU")==0)
					pdata->buttons[j].code = EV_MENU;
			
				else if(strcmp(rk29keyArrary,"ESC")==0)
					pdata->buttons[j].code = KEY_BACK;
				//else if(strcmp(rk29keyArrary,"sensor")==0)
					//pdata->buttons[j].code = KEY_CAMERA;
					else if(strcmp(rk29keyArrary,"F1")==0)
					pdata->buttons[j].code = KEY_F1;
				else if(strcmp(rk29keyArrary,"PLAY")==0)
					pdata->buttons[j].code = KEY_POWER;
				else if(strcmp(rk29keyArrary,"VOLUP")==0)
					pdata->buttons[j].code = KEY_VOLUMEUP;
				else if(strcmp(rk29keyArrary,"VOLDOWN")==0)
					pdata->buttons[j].code = KEY_VOLUMEDOWN;
				else if(strcmp(rk29keyArrary,"SELECT")==0)
					pdata->buttons[j].code = KEY_S;
				else if(strcmp(rk29keyArrary,"START")==0)
					pdata->buttons[j].code = KEY_ENTER;

				else if(strcmp(rk29keyArrary,"A")==0)
					pdata->buttons[j].code = KEY_A;
				else if(strcmp(rk29keyArrary,"B")==0)
					pdata->buttons[j].code = KEY_B;
				else if(strcmp(rk29keyArrary,"X")==0)
					pdata->buttons[j].code = KEY_X;
				else if(strcmp(rk29keyArrary,"Y")==0)
					pdata->buttons[j].code = KEY_Y;
				//else if(strcmp(rk29keyArrary,"HOME")==0)
					//pdata->buttons[j].code = KEY_HOME;
					//else if(strcmp(rk29keyArrary,"F1")==0)
					//pdata->buttons[j].code = KEY_F1;
				else
				     continue;
		 	}

		}
			
   	}

	for(i=0;i<9+4;i++)
		dev_dbg(dev, "desc=%s, code=%d\n",pdata->buttons[i].desc,pdata->buttons[i].code);
	return 0; 

}

static DEVICE_ATTR(rk29key,0660, NULL, rk29key_set);

void rk29_send_power_key(int state)
{
	if (!input_dev)
		return;
	if(state)
	{
		input_report_key(input_dev, KEY_POWER, 1);
		input_sync(input_dev);
	}
	else
	{
		input_report_key(input_dev, KEY_POWER, 0);
		input_sync(input_dev);
	}
}

void rk28_send_wakeup_key(void)
{
	if (!input_dev)
		return;

	input_report_key(input_dev, KEY_WAKEUP, 1);
	input_sync(input_dev);
	input_report_key(input_dev, KEY_WAKEUP, 0);
	input_sync(input_dev);
}

static void keys_long_press_timer(unsigned long _data)
{
	int state;
	struct rk29_button_data *bdata = (struct rk29_button_data *)_data;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	if(button->gpio != INVALID_GPIO )
		state = !!((gpio_get_value(button->gpio) ? 1 : 0) ^ button->active_low);
	else
		state = !!button->adc_state;

	//printk("wq  ------    state =%d       \n",state);
	if(state) {
		if(bdata->long_press_count != 0) {
			if(bdata->long_press_count % (LONG_PRESS_COUNT+ONE_SEC_COUNT) == 0){

				//printk("wq  ------0000000000000    \n");
				key_dbg(bdata, "%skey[%s]: report ev[%d] state[0]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
				input_event(input, type, button->code_long_press, 0);
				input_sync(input);
			}
			else if(bdata->long_press_count%LONG_PRESS_COUNT == 0) {

				//printk("wq  ------1111111111111111    \n");
				key_dbg(bdata, "%skey[%s]: report ev[%d] state[1]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
				input_event(input, type, button->code_long_press, 1);
				input_sync(input);
			}
		}
		bdata->long_press_count++;
		mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
	}
	else {
		if(bdata->long_press_count <= LONG_PRESS_COUNT) {
			bdata->long_press_count = 0;
			//printk("wq  ------22222222222222    \n");
			key_dbg(bdata, "%skey[%s]: report ev[%d] state[1], report ev[%d] state[0]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, button->code);
			input_event(input, type, button->code, 1);
			input_sync(input);
			input_event(input, type, button->code, 0);
			input_sync(input);
		}
		else if(bdata->state != state) {
			//printk("wq  ------333333333333333    \n");
			key_dbg(bdata, "%skey[%s]: report ev[%d] state[0]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
			input_event(input, type, button->code_long_press, 0);
			input_sync(input);
		}
	}
	bdata->state = state;
}

//int skrockr_fg = 0;
extern int get_joy_view_mode();

static void keys_timer(unsigned long _data)
{
	int state;
	struct rk29_button_data *bdata = (struct rk29_button_data *)_data;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	//printk("wq  ------keys_timer    \n");


	//	state = gpio_get_value(button->gpio);//!!((gpio_get_value(button->gpio) ? 1 : 0) ^ button->active_low);

	//	printk("ss       state  =%d    \n",state);

		
	if(button->gpio != INVALID_GPIO)
	{

	
		state = !!((gpio_get_value(button->gpio) ? 1 : 0) ^ button->active_low);
//
	//	printk("ss       state  =%d    \n",state);
	}
	
	else
		state = !!button->adc_state;

	
	if(bdata->state != state) {
		bdata->state = state;
	//printk("wq  --123124--state  ----   =%d      \n",state);

		//SKSetRK2(state);
		//skrockr_fg = state;
		key_dbg(bdata, "%skey[%s]: report ev[%d] state[%d]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, bdata->state);

		//printk("button->code[%d] \n", button->code);

			if(((button->code==30)||(button->code==48)||(button->code==45)||(button->code==21))&&get_joy_view_mode())
			{

			}else
				{

				input_event(input, type, button->code, bdata->state);
				input_sync(input);
				}
			
	}

	
	if(state)
		mod_timer(&bdata->timer,
			jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));

	
}


static irqreturn_t keys_isr(int irq, void *dev_id)
{
	struct rk29_button_data *bdata = dev_id;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	BUG_ON(irq != gpio_to_irq(button->gpio));


//	printk("wq   keys -isr  button->wakeup =%d, bdata->ddata->in_suspend =%s \n ",button->wakeup,bdata->ddata->in_suspend);

	#if 1

	
        if(button->wakeup == 1&& bdata->ddata->in_suspend == true){

		//printk("  ---------------------------------xxxx-----------------  \n");
		
		key_dbg(bdata, "wakeup: %skey[%s]: report ev[%d] state[%d]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, bdata->state);



			
			
					input_event(input, type, button->code, 1);
					input_sync(input);

					input_event(input, type, button->code, 0);
					input_sync(input);
			


			return IRQ_HANDLED;
        }


	
	bdata->long_press_count = 0;
	mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));

		#endif
	
	return IRQ_HANDLED;
}



static unsigned int  ADC_KEY_STATUS=0;
//static unsigned int  tmp_skrock = 0;
 static unsigned int direct_skrock=0;



#define     	direct_left          (0x01<<9)
#define		direct_down 	(0x01<<10)
#define     	direct_right         (0x01<<11)
#define		direct_up		(0x01<<12)

#define     	direct_leftup        ((0x01<<9)|(0x01<<10))
#define		 direct_leftdown    	 ((0x01<<9)|(0x01<<12))
#define		  direct_rightup   	 ((0x01<<11)|(0x01<<10))
#define		direct_rightdown 	 ((0x01<<11)|(0x01<<12))


static unsigned int tmp_skrock=0;

static void keys_adc_callback(struct adc_client *client, void *client_param, int result)
{



	//printk("wq  -------  keys_adc_callback       \n");
	struct rk29_keys_drvdata *ddata = (struct rk29_keys_drvdata *)client_param;
	int i;
	unsigned int type = EV_KEY;
	unsigned shift_bit=0;
	
	#if 0//def SYSCONFIG_VIRTUAL_TOUCH_KEY
	mode_vt_rk=0;
	
	#endif
				  
	 
	if(result > INVALID_ADVALUE && result < EMPTY_ADVALUE)
		ddata->result = result;

		struct rk29_button_data *bdata ;
		struct rk29_keys_button *button;
	
tmp_skrock=0;
		
	for (i = 0; i < ddata->nbuttons; i++) {
	        bdata = &ddata->data[i];
		 button = bdata->button;

				//printk("  wq     (button->gpio)  =  %d    \n ",gpio_get_value(button->gpio));
							
				
				//if(gpio_get_value(button->gpio)==1)

			
		 			//shift_bit=(0x01<<i); 
					
					  // printk("\n dn shift_bit  %d = %d \n",i,shift_bit);

					//	#ifdef SYSCONFIG_VIRTUAL_TOUCH_KEY   
								
					//	SKSetRK(mode_vt_rk);
							
								
					//	#endif        


				
#if		   (defined(CONFIG_MACH_RK3188_SKQ621)||defined(CONFIG_MACH_RK3188_SKQ028)||defined(CONFIG_MACH_RK3188_SKQ026))
			
				if(i>=8)

				{
			//printk("\n dn     i  =  %d = %d \n",i,gpio_get_value(button->gpio));
				                   		//shift_bit=(0x01<<i);
						               if(gpio_get_value(button->gpio)==0)
						               {								
									
							       	tmp_skrock |= (0x01<<i);
									//printk("\n dn     i  =  %d = %x \n",i,tmp_skrock);
								 						              
						               }

				}
//	printk("\n tmp_skrock11= %d \n",tmp_skrock);
			   
			


                	//printk("right direct_skrock  = %d      \n", direct_skrock);

					
              		
			
#endif
		
		if(!button->adc_value)
			continue;

		//printk("wq  --- button->adc_value  = %d      \n", button->adc_value);

		
		if(result < button->adc_value + DRIFT_ADVALUE &&
			result > button->adc_value - DRIFT_ADVALUE)
			{
			//printk("wq 00 --- button->adc    \n");
				button->adc_state = 1;
			}
		else
			{
				//printk("wq 11 --- button->adc     \n", button->gpio);
				button->adc_state = 0;
			}
				
		
			//xxx = gpio_get_value(button->gpio);

			//printk("wq 00 --- button   xxx   = %d      \n", xxx);
			
		/*	if(!gpio_get_value(button->gpio))
			{
			printk("wq 00 --- button->gpio  = %d      \n", button->gpio);
				button->adc_state = 1;
			}
				else
			{
			printk("wq  11--- button->gpio  = %d      \n", button->gpio);
				button->adc_state = 0;
			}*/

		//printk("wq  -------  button->adc_state  =%d          \n",button->adc_state);

		
			if(bdata->state != button->adc_state)
			mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
		

	}

				#ifndef  CONFIG_MACH_RK3188_SKQ627

			       	      if(direct_left == tmp_skrock)
			       		{
							direct_skrock = 1;
						}else if(direct_right== tmp_skrock)
			       		{
							direct_skrock = 2;
						}else if(direct_up == tmp_skrock)
			       		{
							direct_skrock = 3;
						}else if(direct_down== tmp_skrock)
			       		{
							direct_skrock = 4;
						}else if(direct_leftdown== tmp_skrock)
			       		{
							direct_skrock = 5;
						}else if(direct_leftup == tmp_skrock)
			       		{
							direct_skrock = 6;
						}else if(direct_rightdown== tmp_skrock)
			       		{
							direct_skrock = 7;
						}else if(direct_rightup== tmp_skrock)
			       		{
							direct_skrock = 8;
						}else
							{
							direct_skrock = 0;
						}
			    
		
	//printk("\n direct_skrock= %x \n",direct_skrock);

	         SKSetRK(0xff,direct_skrock);		

			#endif
	
	return;
}

static void keys_adc_timer(unsigned long _data)
{
	struct rk29_keys_drvdata *ddata = (struct rk29_keys_drvdata *)_data;

	if (!ddata->in_suspend)
		adc_async_read(ddata->client);

	//printk(" sk--- adc_timer    -----=--------ddata->result.result=%d--------------\n",ddata->result);
	mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(ADC_SAMPLE_TIME));
}

static ssize_t adc_value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);


	//printk(" #########33   sk--- adc_value_show      : %d\n", ddata->result);
	
	return sprintf(buf, "adc_value: %d\n", ddata->result);
}

static DEVICE_ATTR(get_adc_value, S_IRUGO | S_IWUSR, adc_value_show, NULL);

static int __devinit keys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata;
	struct input_dev *input;
	int i, error = 0;
	int wakeup = 0;

	if(!pdata) 
		return -EINVAL;
	
	ddata = kzalloc(sizeof(struct rk29_keys_drvdata) +
			pdata->nbuttons * sizeof(struct rk29_button_data),
			GFP_KERNEL);
	input = input_allocate_device();
	if (!ddata || !input) {
		error = -ENOMEM;
		goto fail0;
	}

	platform_set_drvdata(pdev, ddata);

	input->name = pdev->name;
	input->phys = "gpio-keys/input0";
	input->dev.parent = dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	/* Enable auto repeat feature of Linux input subsystem */
	if (pdata->rep)
		__set_bit(EV_REP, input->evbit);
	ddata->nbuttons = pdata->nbuttons;
	ddata->input = input;

	for (i = 0; i < pdata->nbuttons; i++) {
		struct rk29_keys_button *button = &pdata->buttons[i];
		struct rk29_button_data *bdata = &ddata->data[i];

		bdata->input = input;
		bdata->button = button;
                bdata->ddata = ddata;

		if (button->code_long_press)
			setup_timer(&bdata->timer,
			    	keys_long_press_timer, (unsigned long)bdata);
		else if (button->code)
			setup_timer(&bdata->timer,
			    	keys_timer, (unsigned long)bdata);

		if (button->wakeup)
			wakeup = 1;

		input_set_capability(input, EV_KEY, button->code);
	};

	if (pdata->chn >= 0) {
		setup_timer(&ddata->timer, keys_adc_timer, (unsigned long)ddata);
		ddata->client = adc_register(pdata->chn, keys_adc_callback, (void *)ddata);
		if (!ddata->client) {
			error = -EINVAL;
			goto fail1;
		}
		mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(100));
	}


	for (i = 0; i < pdata->nbuttons; i++) {
		struct rk29_keys_button *button = &pdata->buttons[i];
		struct rk29_button_data *bdata = &ddata->data[i];
		int irq;

		if(button->gpio != INVALID_GPIO) {
			error = gpio_request(button->gpio, button->desc ?: "keys");
			if (error < 0) {
				pr_err("gpio-keys: failed to request GPIO %d,"
					" error %d\n", button->gpio, error);
				goto fail2;
			}
		
			error = gpio_direction_input(button->gpio);
			gpio_pull_updown(button->gpio,1);
			
			if (error < 0) {
				pr_err("gpio-keys: failed to configure input"
					" direction for GPIO %d, error %d\n",
					button->gpio, error);
				gpio_free(button->gpio);
				goto fail2;
			}
			
			irq = gpio_to_irq(button->gpio);
			if (irq < 0) {
				error = irq;
				pr_err("gpio-keys: Unable to get irq number"
					" for GPIO %d, error %d\n",
					button->gpio, error);
				gpio_free(button->gpio);
				goto fail2;
			}

			error = request_irq(irq, keys_isr,
					    (button->active_low)?IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING,
					    button->desc ? button->desc : "keys",
					    bdata);

			
			if (error) {
				pr_err("gpio-keys: Unable to claim irq %d; error %d\n",
					irq, error);
				gpio_free(button->gpio);
				goto fail2;
			}
		}
	}

	input_set_capability(input, EV_KEY, KEY_WAKEUP);

	error = input_register_device(input);
	if (error) {
		pr_err("gpio-keys: Unable to register input device, "
			"error: %d\n", error);
		goto fail2;
	}

	device_init_wakeup(dev, wakeup);
	error = device_create_file(dev, &dev_attr_get_adc_value);

	error = device_create_file(dev, &dev_attr_rk29key);
	if(error )
	{
		pr_err("failed to create key file error: %d\n", error);
	}


	input_dev = input;
	return error;

 fail2:
	while (--i >= 0) {
		free_irq(gpio_to_irq(pdata->buttons[i].gpio), &ddata->data[i]);
		del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}
	if(pdata->chn >= 0 && ddata->client);
		adc_unregister(ddata->client);
	if(pdata->chn >= 0)
	        del_timer_sync(&ddata->timer);
 fail1:
 	platform_set_drvdata(pdev, NULL);
 fail0:
	input_free_device(input);
	kfree(ddata);

	return error;
}

static int __devexit keys_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;
	int i;

	input_dev = NULL;
	device_init_wakeup(dev, 0);

	for (i = 0; i < pdata->nbuttons; i++) {
		int irq = gpio_to_irq(pdata->buttons[i].gpio);
		free_irq(irq, &ddata->data[i]);
		del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}
	if(pdata->chn >= 0 && ddata->client);
		adc_unregister(ddata->client);
	input_unregister_device(input);

	return 0;
}


#ifdef CONFIG_PM
static int keys_suspend(struct device *dev)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	int i;

	ddata->in_suspend = true;

	if (device_may_wakeup(dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct rk29_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = gpio_to_irq(button->gpio);
				enable_irq_wake(irq);
			}
		}
	}

	return 0;
}

static int keys_resume(struct device *dev)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	int i;

	if (device_may_wakeup(dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct rk29_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = gpio_to_irq(button->gpio);
				disable_irq_wake(irq);
			}
		}
		preempt_disable();
		if (local_softirq_pending())
			do_softirq(); // for call resend_irqs, which may call keys_isr
		preempt_enable_no_resched();
	}

	ddata->in_suspend = false;

	return 0;
}

static const struct dev_pm_ops keys_pm_ops = {
	.suspend	= keys_suspend,
	.resume		= keys_resume,
};
#endif

static struct platform_driver keys_device_driver = {
	.probe		= keys_probe,
	.remove		= __devexit_p(keys_remove),
	.driver		= {
		.name	= "rk29-keypad",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &keys_pm_ops,
#endif
	}
};

static int __init keys_init(void)
{
	return platform_driver_register(&keys_device_driver);
}

static void __exit keys_exit(void)
{
	platform_driver_unregister(&keys_device_driver);
}

module_init(keys_init);
module_exit(keys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys");

