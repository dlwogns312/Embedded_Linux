#include "stopwatch.h"

//usage counter
static int inter_usage=0;

//variables for stopwatch
static int pause_set=0;
static int inter_major=242;
static int inter_minor=0;
static int start_set=0;
static int exit_cnt=0;
static int exit_set=0;
static int result;

static unsigned char min=0;
static unsigned char sec=0;

static int cnt=0;

//Timer variable
struct timer_list timer;

//variable for work queue
static struct work_struct wq_home;
static struct work_struct wq_back;
static struct work_struct wq_up;
static struct work_struct wq_down;

// address of fpga fnd
static unsigned char* fnd_addr;

static dev_t inter_dev;
static struct cdev inter_cdev;

//variable for wait queue
wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

static struct file_operations inter_fops=
{
    .open = inter_open,
    .write = inter_write,
    .release = inter_release,
};

static void timer_handler(unsigned long param)
{
    if(exit_set)
    {
        if(exit_cnt==300)
        {
            printk("Turn off Stopwatch program!\n");
            __wake_up(&wq_write,1,1,NULL);
            return 0;
        }
        else
        {
            exit_cnt++;
        }
    }


    if(pause_set!=1)
    {
        //update stopwatch after 1second 
        cnt++;
        if(cnt==100)
        {
            cnt=0;
            sec++;
            if(sec==60)
            {
                sec=0;
                min=(min+1)%60;
            }
            unsigned short int _s_value;
            unsigned char value[4];
            value[0]=min/10;
            value[1]=min%10;
            value[2]=sec/10;
            value[3]=sec%10;

            //FND out
            _s_value = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
            outw(_s_value,(unsigned int)fnd_addr);
        } 
    }

    timer.expires=get_jiffies_64() + (HZ/100);
    timer.data=0;
    timer.function=timer_handler;
    add_timer(&timer);  

    return 0;
}

irqreturn_t inter_home(int irq, void* dev_id, struct pt_regs* reg)
{
    printk(KERN_ALERT"Start Stopwatch!\n");

    if(!start_set)
    {
        start_set=1;
        init_timer(&timer);
        timer.expires=get_jiffies_64() + (HZ/100);
        timer.data=0;
        timer.function=timer_handler;
        add_timer(&timer);
    }

    schedule_work(&wq_home);
    return IRQ_HANDLED;

}

irqreturn_t inter_back(int irq, void* dev_id, struct pt_regs* reg)
{
    printk(KERN_ALERT"Pause Stopwatch!\n");

    schedule_work(&wq_back);

    return IRQ_HANDLED;
}

irqreturn_t inter_volume_up(int irq, void* dev_id, struct pt_regs* reg)
{
    printk(KERN_ALERT"Reset Stopwatch!\n");

    schedule_work(&wq_up);

    return IRQ_HANDLED;
}

irqreturn_t inter_volume_down(int irq, void* dev_id, struct pt_regs* reg)
{
   printk(KERN_ALERT"Try to turn stopwatch off!\n");
   schedule_work(&wq_down);
   return IRQ_HANDLED;
} 

static int inter_open(struct inode *minode, struct file *mfile)
{
    int ret,irq;
    printk(KERN_ALERT"Open Module!\n");

    if(inter_usage!=0)
        return -EBUSY;
    
    inter_usage=1;

    min=0;sec=0;cnt=0;
    unsigned short int _s_value;
    unsigned char value[4];
    value[0]=min/10;
    value[1]=min%10;
    value[2]=sec/10;
    value[3]=sec%10;

    //FND out
    _s_value = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(_s_value,(unsigned int)fnd_addr);

    //interrupt home
    gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_home, IRQF_TRIGGER_FALLING, "home", 0);

	// interrrupt back
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_back, IRQF_TRIGGER_FALLING, "back", 0);

	// interrupt volume up
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_volume_up, IRQF_TRIGGER_FALLING, "volup", 0);

	// interrupt volume down
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_volume_down, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "voldown", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	
    inter_usage=0;
    start_set=0;
    min=0;sec=0;cnt=0;

    unsigned short int _s_value;
    unsigned char value[4];
    value[0]=min/10;
    value[1]=min%10;
    value[2]=sec/10;
    value[3]=sec%10;

    //FND out
    _s_value = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(_s_value,(unsigned int)fnd_addr);
    
    free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
        
    printk("sleep on\n");
    interruptible_sleep_on(&wq_write);
         
	printk("write\n");
	return 0;
}


static int inter_register_cdev(void)
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"inter");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"inter");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}

void do_home()
{
    pause_set=0;
    return;
}

void do_back()
{
    pause_set=1;
    return;
}

void do_up()
{
    cnt=0; min=0; sec=0;

    unsigned short int _s_value;
    unsigned char value[4];
    value[0]=min/10;
    value[1]=min%10;
    value[2]=sec/10;
    value[3]=sec%10;

    //FND out
     _s_value = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(_s_value,(unsigned int)fnd_addr);
    
    return;
}

void do_down()
{
    exit_set=1-exit_set;
    exit_cnt=0;

    return;
}

static int __init inter_init(void)
{
    int result;
	if((result = inter_register_cdev()) < 0 )
		return result;

    fnd_addr = ioremap (FND_ADDRESS, 0x4);

    if (fnd_addr == NULL) 
    {
        printk (KERN_WARNING "ioremap for fnd error!\n");
        return -EFAULT;
    }
    //initialize the work queue
    INIT_WORK(&wq_home,(typeof(do_home.func))do_home);
    INIT_WORK(&wq_back,(typeof(do_back.func))do_back);
    INIT_WORK(&wq_up,(typeof(do_up.func))do_up);
    INIT_WORK(&wq_down,(typeof(do_down.func))do_down);

	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : %d \n", inter_major);

  
    return 0;
}

static void __exit inter_exit(void)
{
    iounmap (fnd_addr);
    cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	del_timer (&timer);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");