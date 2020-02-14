/*
 * ioctl.c
 *
 *  Created on: 2012. 1. 20.
 *      Author: MYHOME
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

int kw_device_open(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "kw_device open fuction called\n");
	return 0;
}

int kw_device_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "kw_device release fuction called\n");
	return 0;
}

//ioctl 연산 구현 부분 기존 read, write연산 제거하고 모두 ioctl로 구현
int kw_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case 1: //read
		printk(KERN_ALERT "ioctl read...\n");
		break;
	case 2: //write
		printk(KERN_ALERT "ioctl write...\n");
		break;
		
	default:
		printk(KERN_ALERT "ioctl unknown command...\n");
		break;
	}
	return 0;
}

//동작만 살펴보기 위해 간단한 메세지만 출력하 였다.

static struct file_operations vd_fops ={
		.owner  	= THIS_MODULE,
		.open   	= kw_device_open,
//		.release	= kw_device_release,
		.unlocked_ioctl	= kw_device_ioctl
};

int __init kw_device_init(void){
	// 문자 디바이스를 등록한다. 
	if(register_chrdev(241,"kw_device", &vd_fops) <0 )
		printk(KERN_ALERT "driver init failed\n");
	else
		printk(KERN_ALERT "driver init successful\n");
	return 0;
}

void __exit kw_device_exit(void){
	unregister_chrdev(241,"kw_device");
	printk(KERN_ALERT "driver cleanup successful!\n");
}

module_init(kw_device_init);
module_exit(kw_device_exit);

MODULE_LICENSE("GPL");
