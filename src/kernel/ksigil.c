#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sebastian Czapla");
MODULE_DESCRIPTION("SigilVM Auxiliary module");
MODULE_VERSION("1.0");

static int __init hello_init(void)
{
    pr_info("SigilVM module loaded\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("SigilVM module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);
