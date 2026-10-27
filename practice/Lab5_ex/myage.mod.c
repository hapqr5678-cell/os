#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__fentry__" },
	{ 0x7a6661ca, "ktime_get_real_seconds" },
	{ 0xf56982a1, "ts" },
	{ 0xe8213e80, "_printk" },
	{ 0x388fe4c0, "is_adult" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xd954c786, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0x7a6661ca,
	0xf56982a1,
	0xe8213e80,
	0x388fe4c0,
	0xd272d446,
	0xd954c786,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__fentry__\0"
	"ktime_get_real_seconds\0"
	"ts\0"
	"_printk\0"
	"is_adult\0"
	"__x86_return_thunk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "d2ts");


MODULE_INFO(srcversion, "75E4B1D3531F1D513733DD0");
