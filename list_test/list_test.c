#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");

static int n_elem = 2;
module_param(n_elem, int, 0);

struct node {
    int value;
    struct list_head kl;
};

static struct list_head head;

static int __init test_module_init(void)
{
    int i;
    struct node *n;

    INIT_LIST_HEAD(&head);
    printk("Creating a list with %d elements\n", n_elem);

    for (i = 0; i < n_elem; i++) {
        n = kmalloc(sizeof(struct node), GFP_KERNEL);
        n->value = i + 1;
        list_add(&(n->kl), &head);
    }

    return 0;
}

static void __exit test_module_unload(void)
{
    struct list_head *l, *tmp;
    struct node *n;

    printk("Cleaning up list:\n");

    list_for_each_safe(l, tmp, &head) {
        n = list_entry(l, struct node, kl);
        printk("Value: %d\n", n->value);
        list_del(l);
        kfree(n);
    }
}

module_init(test_module_init);
module_exit(test_module_unload);
