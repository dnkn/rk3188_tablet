#ifndef __LINUX_GSLX680__
#define __LINUX_GSLX680__

#define CT36X_NAME	"gslX680"

struct gslX680_platform_data {
	int model ;
	int rest_pin;
	int irq_pin ;
	int (*get_pendown_state)(void);
	int (*init_platform_hw)(void);
	int (*platform_sleep)(void);
	int (*platform_wakeup)(void);
	void (*exit_platform_hw)(void);
};

#endif
