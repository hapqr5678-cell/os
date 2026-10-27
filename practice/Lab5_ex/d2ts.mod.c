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

KSYMTAB_DATA(ts, "", "");
KSYMTAB_FUNC(is_adult, "", "");

SYMBOL_CRC(ts, 0xf56982a1, "");
SYMBOL_CRC(is_adult, 0x388fe4c0, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__fentry__" },
	{ 0x800c1eb4, "mktime64" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xe8213e80, "_printk" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x15ec8bdd, "param_ops_int" },
	{ 0xd954c786, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0x800c1eb4,
	0xd272d446,
	0xbd03ed67,
	0xe8213e80,
	0x90a48d82,
	0xd272d446,
	0x15ec8bdd,
	0xd954c786,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__fentry__\0"
	"mktime64\0"
	"__x86_return_thunk\0"
	"__ref_stack_chk_guard\0"
	"_printk\0"
	"__ubsan_handle_out_of_bounds\0"
	"__stack_chk_fail\0"
	"param_ops_int\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EC0BF845437BA49A98F31F8");
