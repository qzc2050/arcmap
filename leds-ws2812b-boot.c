/*
 * drivers/leds/leds-ws2812b-boot.c
 *
 * 内核启动期 WS2812B 状态灯驱动（ArcMap-II 面板 31 灯）
 *
 * 目的：在内核 late_initcall 阶段（rootfs 挂载、应用启动之前）通过 SPI MOSI
 *       脚（PI12 = spi0.0）播放开机动画，覆盖 boot → app 的视觉空窗期。
 *
 * 开机动画（默认 3 秒，节奏对齐）：
 *   前 80%：两路并行
 *     ① 左右半圆：左上→左下→右下→右上 闭环旋转两圈
 *     ② Logo + 射灯：两排同时 左→右→右→左，共走两轮
 *   后 20%：整板（半圆 + Logo + 射灯）一起常亮
 *   结束后整条切 idle_color（默认暗蓝），交给用户态 App。
 *
 *      Logo 左→右：15,14,13,12,11
 *      射灯 左→右：16,17,18,19,20,21
 *      半圆路径：1..10 → 22..31
 *
 * 硬件：WS2812B × N 颗，DIN 接到 SPI0 MOSI（PI12），共地，5V 单独供电。
 *
 * 时序（8 SPI bit 编码 1 WS bit @ 6.4MHz）：
 *     '0' bit -> 0xC0    '1' bit -> 0xF8
 *     每颗灯 GRB，帧头/帧尾各 320 字节 0x00 作 RESET。
 *
 * 关于设备冲突：
 *   不创建新 spi_device；bus_find_device_by_name("spi0.0") 临时 spi_sync，
 *   完成后 put_device，用户态仍可用 /dev/spidev0.0。
 *
 * cmdline（常用）：
 *   leds_ws2812b_boot.leds=31
 *   leds_ws2812b_boot.chase_ms=3000
 *   leds_ws2812b_boot.chase_step_ms=40
 *   leds_ws2812b_boot.color=0x0000FF
 *   leds_ws2812b_boot.idle_color=0x000080
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/spi/spi.h>

#define WS2812_BYTES_PER_LED          24
#define WS2812_SYM_ZERO               0xC0
#define WS2812_SYM_ONE                0xF8
#define WS2812_RESET_BYTES_PRE        320
#define WS2812_RESET_BYTES_POST       320
#define WS2812_MAX_LEDS               1024
#define WS2812_DEFAULT_LEDS           31
#define WS2812_DEFAULT_COLOR          0x0000FFu   /* 开机动画蓝 */
#define WS2812_DEFAULT_IDLE_COLOR     0x000080u   /* 结束后暗蓝 */
#define WS2812_DEFAULT_SPI_HZ         6400000u
#define WS2812_DEFAULT_BUS            "spi0.0"
#define WS2812_DEFAULT_CHASE_MS       3000u
#define WS2812_DEFAULT_CHASE_STEP_MS  40u
#define WS2812_ARCMAP_LEDS            31
#define WS2812_ARC_PATH_LEN           20
#define WS2812_ARC_TAIL               3
#define WS2812_LOGO_LEN               5
#define WS2812_CHIN_LEN               6

static unsigned int ws_color = WS2812_DEFAULT_COLOR;
module_param_named(color, ws_color, uint, S_IRUGO);
MODULE_PARM_DESC(color, "0xRRGGBB animation color; default bright blue");

static unsigned int ws_leds = WS2812_DEFAULT_LEDS;
module_param_named(leds, ws_leds, uint, S_IRUGO);
MODULE_PARM_DESC(leds, "LED count (ArcMap-II = 31)");

static unsigned int ws_spi_hz = WS2812_DEFAULT_SPI_HZ;
module_param_named(spi_hz, ws_spi_hz, uint, S_IRUGO);
MODULE_PARM_DESC(spi_hz, "SPI clock Hz (default 6.4MHz)");

static char *ws_bus_name = WS2812_DEFAULT_BUS;
module_param_named(bus_name, ws_bus_name, charp, S_IRUGO);
MODULE_PARM_DESC(bus_name, "spi device name, e.g. spi0.0");

static unsigned int ws_chase_ms = WS2812_DEFAULT_CHASE_MS;
module_param_named(chase_ms, ws_chase_ms, uint, S_IRUGO);
MODULE_PARM_DESC(chase_ms,
	"boot animation duration ms (0 = one solid frame); default 3000");

static unsigned int ws_chase_step_ms = WS2812_DEFAULT_CHASE_STEP_MS;
module_param_named(chase_step_ms, ws_chase_step_ms, uint, S_IRUGO);
MODULE_PARM_DESC(chase_step_ms, "frame interval ms; default 40");

static unsigned int ws_idle_color = WS2812_DEFAULT_IDLE_COLOR;
module_param_named(idle_color, ws_idle_color, uint, S_IRUGO);
MODULE_PARM_DESC(idle_color,
	"solid color after animation (0xRRGGBB); default dim blue");

/*
 * 半圆闭环（index = 灯号-1）：
 * 左上→左下 1..10，再 右下→右上 22..31
 */
static const u8 ws_arc_path[WS2812_ARC_PATH_LEN] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

/* Logo 左→右：15,14,13,12,11 */
static const u8 ws_logo_ltr[WS2812_LOGO_LEN] = { 14, 13, 12, 11, 10 };

/* 射灯 左→右：16..21 */
static const u8 ws_chin_ltr[WS2812_CHIN_LEN] = { 15, 16, 17, 18, 19, 20 };

static void ws2812b_encode_pixel(u8 *out, u8 r, u8 g, u8 b)
{
	u8 grb[3] = { g, r, b };
	int byte_idx, bit;
	u8 *p = out;

	for (byte_idx = 0; byte_idx < 3; byte_idx++) {
		u8 v = grb[byte_idx];

		for (bit = 7; bit >= 0; bit--)
			*p++ = (v & (1u << bit)) ? WS2812_SYM_ONE : WS2812_SYM_ZERO;
	}
}

static void ws2812b_set_rgb(u8 *rgb, unsigned int idx, unsigned int n,
			    u8 r, u8 g, u8 b)
{
	if (idx >= n)
		return;
	rgb[idx * 3 + 0] = r;
	rgb[idx * 3 + 1] = g;
	rgb[idx * 3 + 2] = b;
}

static int ws2812b_boot_send_rgb(struct spi_device *spi, const u8 *rgb,
				 unsigned int n_leds)
{
	struct spi_transfer xfer;
	struct spi_message msg;
	size_t frame_bytes = n_leds * WS2812_BYTES_PER_LED;
	size_t total_bytes = WS2812_RESET_BYTES_PRE + frame_bytes
				+ WS2812_RESET_BYTES_POST;
	u8 *buf;
	unsigned int i;
	int ret;

	buf = kzalloc(total_bytes, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	for (i = 0; i < n_leds; i++) {
		const u8 *p = rgb + i * 3;

		ws2812b_encode_pixel(buf + WS2812_RESET_BYTES_PRE
					 + i * WS2812_BYTES_PER_LED,
				     p[0], p[1], p[2]);
	}

	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf        = buf;
	xfer.len           = total_bytes;
	xfer.speed_hz      = ws_spi_hz;
	xfer.bits_per_word = 8;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = spi_sync(spi, &msg);

	kfree(buf);
	return ret;
}

static int ws2812b_boot_send(struct spi_device *spi,
			     u8 r, u8 g, u8 b, unsigned int n_leds)
{
	unsigned int i;
	u8 *rgb;
	int ret;

	rgb = kmalloc(n_leds * 3, GFP_KERNEL);
	if (!rgb)
		return -ENOMEM;

	for (i = 0; i < n_leds; i++) {
		rgb[i * 3 + 0] = r;
		rgb[i * 3 + 1] = g;
		rgb[i * 3 + 2] = b;
	}

	ret = ws2812b_boot_send_rgb(spi, rgb, n_leds);
	kfree(rgb);
	return ret;
}

/*
 * 3 秒开机动画（ArcMap 31 灯，节奏对齐）：
 *  - 前 80%：半圆转两圈 + Logo/射灯两轮扫
 *  - 后 20%：整板一起点亮
 */
static int ws2812b_chase_threadfn(void *data)
{
	struct device *dev = data;
	struct spi_device *spi = to_spi_device(dev);
	u8 *rgb;
	u8 r, g, b;
	u8 ir, ig, ib;
	unsigned int n = ws_leds;
	unsigned int duration = ws_chase_ms ? ws_chase_ms : WS2812_DEFAULT_CHASE_MS;
	unsigned int step_ms = ws_chase_step_ms ? ws_chase_step_ms : 1;
	unsigned long start_j;
	unsigned long deadline;

	rgb = kzalloc(n * 3, GFP_KERNEL);
	if (!rgb)
		goto out_put;

	r = (ws_color >> 16) & 0xff;
	g = (ws_color >>  8) & 0xff;
	b = (ws_color >>  0) & 0xff;

	ir = (ws_idle_color >> 16) & 0xff;
	ig = (ws_idle_color >>  8) & 0xff;
	ib = (ws_idle_color >>  0) & 0xff;

	start_j = jiffies;
	deadline = start_j + msecs_to_jiffies(duration);

	while (time_before(jiffies, deadline)) {
		unsigned int elapsed = jiffies_to_msecs(jiffies - start_j);
		unsigned int p;
		unsigned int arc_step;
		unsigned int t;
		unsigned int mid_step;
		unsigned int logo_i;
		unsigned int chin_i;

		if (elapsed > duration)
			elapsed = duration;
		p = (elapsed * 1000u) / duration;

		memset(rgb, 0, n * 3);

		if (p < 800) {
			/* 前 80%：两路并行，各走完“两圈/两轮” */
			unsigned int pe = (p * 1000u) / 800; /* 映射到 0..1000 */
			unsigned int seg = p / 200;
			unsigned int local = p % 200;
			int go_ltr = ((seg % 2) == 0);

			arc_step = (pe * (WS2812_ARC_PATH_LEN * 2)) / 1000;
			if (arc_step >= WS2812_ARC_PATH_LEN * 2)
				arc_step = WS2812_ARC_PATH_LEN * 2 - 1;

			for (t = 0; t < WS2812_ARC_TAIL; t++) {
				unsigned int s = (arc_step + WS2812_ARC_PATH_LEN * 2 - t)
						% (WS2812_ARC_PATH_LEN * 2);
				unsigned int idx = ws_arc_path[s % WS2812_ARC_PATH_LEN];
				u8 fade = (u8)(255u * (WS2812_ARC_TAIL - t)
						/ WS2812_ARC_TAIL);

				if (idx < n) {
					ws2812b_set_rgb(rgb, idx, n,
							(u8)(r * fade / 255u),
							(u8)(g * fade / 255u),
							(u8)(b * fade / 255u));
				}
			}

			mid_step = (local * WS2812_CHIN_LEN) / 200;
			if (mid_step >= WS2812_CHIN_LEN)
				mid_step = WS2812_CHIN_LEN - 1;

			if (go_ltr) {
				chin_i = mid_step;
				logo_i = (mid_step < WS2812_LOGO_LEN)
						? mid_step : (WS2812_LOGO_LEN - 1);
			} else {
				chin_i = WS2812_CHIN_LEN - 1 - mid_step;
				logo_i = WS2812_LOGO_LEN - 1
					- ((mid_step < WS2812_LOGO_LEN)
						? mid_step : (WS2812_LOGO_LEN - 1));
			}
			ws2812b_set_rgb(rgb, ws_logo_ltr[logo_i], n, r, g, b);
			ws2812b_set_rgb(rgb, ws_chin_ltr[chin_i], n, r, g, b);
		} else {
			/* 后 20%：整板一起点亮 */
			for (t = 0; t < n; t++)
				ws2812b_set_rgb(rgb, t, n, r, g, b);
		}

		if (ws2812b_boot_send_rgb(spi, rgb, n))
			goto out_idle;

		msleep(step_ms);
	}

out_idle:
	ws2812b_boot_send(spi, ir, ig, ib, n);
	kfree(rgb);

out_put:
	put_device(dev);
	return 0;
}

static int __init ws2812b_boot_init(void)
{
	struct device *dev;
	struct spi_device *spi;
	u8 r, g, b;
	int ret;

	if (ws_leds == 0 || ws_leds > WS2812_MAX_LEDS) {
		pr_err("ws2812b-boot: invalid leds=%u (1..%d)\n",
		       ws_leds, WS2812_MAX_LEDS);
		return 0;
	}

	if (ws_leds < WS2812_ARCMAP_LEDS) {
		pr_warn("ws2812b-boot: leds=%u < 31, ArcMap path needs 31\n",
			ws_leds);
	}

	dev = bus_find_device_by_name(&spi_bus_type, NULL, ws_bus_name);
	if (!dev) {
		pr_err("ws2812b-boot: spi device '%s' not found "
		       "(check sys_config.fex spi0 / spidev settings)\n",
		       ws_bus_name);
		return 0;
	}

	spi = to_spi_device(dev);

	r = (ws_color >> 16) & 0xff;
	g = (ws_color >>  8) & 0xff;
	b = (ws_color >>  0) & 0xff;

	pr_info("ws2812b-boot: ArcMap anim %ums, %u LEDs, color #%02x%02x%02x, "
		"idle #%02x%02x%02x, %u Hz, %s\n",
		ws_chase_ms, ws_leds, r, g, b,
		(ws_idle_color >> 16) & 0xff, (ws_idle_color >> 8) & 0xff,
		ws_idle_color & 0xff,
		ws_spi_hz, ws_bus_name);

	if (ws_chase_ms > 0) {
		struct task_struct *tsk;

		get_device(dev);
		tsk = kthread_run(ws2812b_chase_threadfn, dev, "ws2812b-boot");
		if (IS_ERR(tsk)) {
			pr_err("ws2812b-boot: kthread_run failed %ld\n", PTR_ERR(tsk));
			put_device(dev);
			put_device(dev);
			return 0;
		}
		pr_info("ws2812b-boot: arc x2 + logo/chin sweep started\n");
		put_device(dev);
		return 0;
	}

	ret = ws2812b_boot_send(spi, r, g, b, ws_leds);
	if (ret)
		pr_err("ws2812b-boot: spi_sync failed: %d\n", ret);
	else
		pr_info("ws2812b-boot: frame sent OK\n");

	put_device(dev);
	return 0;
}

late_initcall(ws2812b_boot_init);

MODULE_AUTHOR("ws2812b boot indicator");
MODULE_DESCRIPTION("WS2812B ArcMap boot: arc rotate x2 + logo/chin sweep in 3s");
MODULE_LICENSE("GPL");
