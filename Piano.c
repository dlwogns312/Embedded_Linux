#include "Piano.h"

static int dev_usage=0;
static unsigned char *iom_lcd_addr;
static unsigned char *iom_switch_addr;
static int term=0;


unsigned char switch_value[9];

unsigned char text_lcd[33];

static struct file_operations fops={
    .owner=THIS_MODULE,
    .open=dev_driver_open,
    .release=dev_driver_release,
    .read = dev_driver_read,
    .write = dev_driver_write,
};

irqreturn_t inter_back(int irq, void* dev_id, struct pt_regs* reg)
{
	printk(KERN_ALERT"STOP\n");
	term=1;
	return IRQ_HANDLED;
}

int dev_driver_open(struct inode *minode ,struct file*mfile)
{
    int ret,irq;

    term=0;

    if(dev_usage!=0)
        return -EBUSY;    
	
   gpio_direction_input(IMX_GPIO_NR(1,12));

   irq=gpio_to_irq(IMX_GPIO_NR(1,12));
    ret=request_irq(irq,inter_back,IRQF_TRIGGER_FALLING,"back",0);
    
//    memset(text_lcd,' ',sizeof(text_lcd));
  //  memset(switch_value,0,sizeof(switch_value));

  // display();
    printk(KERN_ALERT"dev_lib Open!\n");

    dev_usage=1;

    return 0;
}

int dev_driver_release(struct inode *minode,struct file *mfile)
{
    dev_usage=0;
    printk(KERN_ALERT"dev_driver_release\n");
   free_irq(gpio_to_irq(IMX_GPIO_NR(1,12)),NULL);
    return 0;
}

void display(void)
{
    int i;
    unsigned short int _s_value=0;

    text_lcd[32]=0;
    for(i=0;i<32;i++)
    {
        _s_value = ((text_lcd[i]) & 0xFF) << 8 | (text_lcd[i + 1]) & 0xFF;
		outw(_s_value,(unsigned int)iom_lcd_addr+i);
        i++;
    }
}

static int __init iom_init(void)
{
   int result;

   result=register_chrdev(DEVICE_MAJOR,DEVICE_NAME,&fops);
   if(result<0)
   {
       printk(KERN_WARNING"error %d\n",result);
       return result;
   }
    printk("dev_driver_init\n");

   iom_lcd_addr=ioremap(LCD_ADDRESS,0x32);
   iom_switch_addr=ioremap(SWITCH_ADDRESS, 0x18);
   
   memset(text_lcd,' ',sizeof(text_lcd));
   memset(switch_value,0,sizeof(switch_value));

   if(iom_lcd_addr==NULL||iom_switch_addr==NULL)
   {
       printk(KERN_WARNING"ioremap fail!\n");
       return -EFAULT;
   }

//	int ret, irq;
//	gpio_direction_input(IMX_GPIO_NR(1,12));
//	irq=gpio_to_irq(IMX_GPIO_NR(1,12));
//	ret=request_irq(irq,inter_back,IRQF_TRIGGER_FALLING,"back",0);
   printk("init module, %s major number : %d\n",DEVICE_NAME,DEVICE_MAJOR);
   return 0;
}

ssize_t dev_driver_write(struct file *inode, const char* gdata,size_t length, loff_t *off_what)
{
    int i;
    unsigned short int _s_value=0;

    const char* tmp=gdata;

    if(copy_from_user(text_lcd, tmp, length))
        return -EFAULT;

    display();

    return length;
}

ssize_t dev_driver_read(struct file *inode, char * gdata, size_t length, loff_t *off_what)
{
    int i;
    unsigned short int _s_value;

    for(i=0;i<length;i++) 
    {
		_s_value = inw((unsigned int)iom_switch_addr+i*2);
        switch_value[i] = _s_value &0xFF;
    }

    if(term)
	{
		switch_value[0]=1;
	}
	else
		switch_value[0]=0;
    if(copy_to_user(gdata, switch_value, length))
    {
        printk("Fail copying!\n");
        return -EFAULT;
    }    
    for(i=0;i<500000;i++);	
   // msleep(1000);
    display();

    return length;
}

void __exit iom_exit(void)
{
    iounmap(iom_lcd_addr);
    iounmap(iom_switch_addr);

//    free_irq(gpio_to_irq(IMX_GPIO_NR(1,11)),NULL);
	 unregister_chrdev(DEVICE_MAJOR,DEVICE_NAME);
    printk("Exit module\n");
}

module_init(iom_init);
module_exit(iom_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LEEJAEHOON");
