/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/delay.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/dmaengine_pcm.h>

#include <ralink_regs.h>

#define PCM_GLB_CFG 0x00
#define PCM_GLB_CFG_EN BIT(31)
#define PCM_GLB_CFG_DMA_EN BIT(30)
#define PCM_GLB_CFG_CH0_TX_EN BIT(8)   //have problems
#define PCM_GLB_CFG_CH0_RX_EN BIT(0)  // channel 0 TX、RX enable

#define PCM_GLB_CFG_RFF_THRES 20
#define PCM_GLB_CFG_TFF_THRES 16

#define PCM_GLB_CFG_DFT_THRES	(4 << PCM_GLB_CFG_RFF_THRES) | \
					(4 << PCM_GLB_CFG_TFF_THRES)

#define PCM_PCM_CFG 0x04
#define PCM_PCM_CFG_CLKOUT_EN BIT(30)
#define PCM_PCM_CFG_EXT_FSYNC BIT(27)
#define PCM_PCM_CFG_LONG_FSYNC BIT(26)
#define PCM_PCM_CFG_FSYNC_POL BIT(25)
#define PCM_PCM_CFG_DTX_TRI	BIT(24)



#define PCM_INT_STATUS 0x08
#define PCM_INT_EN 0x0C
#define PCM_FF_STATUS 0x10
#define PCM_CH0_CFG 0x20
#define CMP_MODE2 BIT(29)
#define CMP_MODE1 BIT(28)
#define CMP_MODE0 BIT(27)



#define PCM_CH1_CFG 0x24
#define PCM_FSYNC_CFG 0x30
#define PCM_CFG_FSYNC_EN BIT(31)
#define PCM_POS_CAP_DT BIT(30)
#define PCM_POS_DRV_DT BIT(29)
#define PCM_POS_CAP_FSYNC BIT(28)
#define PCM_POS_DRV_FSYNC BIT(27)



#define PCM_CH_CFG2 0x34
#define PCM_DIVCOMP_CFG 0x50
#define PCM_DIVCOMP_CFG_CLK_EN BIT(31)

#define PCM_DIVINT_CFG 0x54
#define PCM_DIGDELAY_CFG 0x60
#define PCM_CH0_FIFO 0x80
#define PCM_CH1_FIFO 0x84

#define PCM_DEBUG
#undef PCM_DEBUG

struct mt7628_pcm {
	struct resource *mem;
	void __iomem *base;
	dma_addr_t phys_base;

	struct snd_dmaengine_dai_dma_data playback_dma_data;
	struct snd_dmaengine_dai_dma_data capture_dma_data;
};

static inline uint32_t mt7628_pcm_read(const struct mt7628_pcm *pcm,
	unsigned int reg)
{
	return readl(pcm->base + reg);
}

static inline void mt7628_pcm_write(const struct mt7628_pcm *pcm,
	unsigned int reg, uint32_t value)
{
	writel(value, pcm->base + reg);
}

static int mt7628_pcm_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	if (dai->active)
	{
		return 0;
	}
	else
	{
#ifdef PCM_DEBUG
		printk("mt7628_pcm dai->active=0\n");
#endif
	}

	cfg = mt7628_pcm_read(pcm, PCM_GLB_CFG);
	cfg |= PCM_GLB_CFG_EN;
	mt7628_pcm_write(pcm, PCM_GLB_CFG, cfg);

	return 0;
}

static void mt7628_pcm_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
#ifdef PCM_DEBUG
	printk("hello mt7628_pcm_shutdown............\n");
#endif
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	if (dai->active)
	{
#ifdef PCM_DEBUG
		printk("mt7628_pcm_shutdown....dai->active!=0\n");
#endif
		return 0;
	}

	cfg = mt7628_pcm_read(pcm, PCM_GLB_CFG);
	cfg &= ~PCM_GLB_CFG_EN;
	mt7628_pcm_write(pcm, PCM_GLB_CFG, cfg);
}

static int mt7628_pcm_trigger(struct snd_pcm_substream *substream, int cmd,
	struct snd_soc_dai *dai)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
#ifdef PCM_DEBUG	
#endif
	uint32_t cfg;
	uint32_t mask;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		mask = PCM_GLB_CFG_CH0_RX_EN;// open channel 0
	else
		mask = PCM_GLB_CFG_CH0_RX_EN;// open channel 0

	cfg = mt7628_pcm_read(pcm, PCM_GLB_CFG);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		printk("SNDRV_PCM_TRIGGER_START\n");
		cfg |= mask;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		cfg &= ~mask;
		printk("SNDRV_PCM_TRIGGER_STOP\n");
		break;
	default:
		return -EINVAL;
	}

	if (cfg & PCM_GLB_CFG_CH0_RX_EN)
	{
		cfg |= PCM_GLB_CFG_DMA_EN;// enable dma
#ifdef PCM_DEBUG
#endif
	}
	else
		cfg &= ~PCM_GLB_CFG_DMA_EN;

	mt7628_pcm_write(pcm, PCM_GLB_CFG, cfg);

	return 0;
}

static int mt7628_pcm_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	cfg = mt7628_pcm_read(pcm, PCM_PCM_CFG);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		cfg &= ~PCM_PCM_CFG_CLKOUT_EN; // pcm clock from external
       cfg |= PCM_PCM_CFG_EXT_FSYNC; // pcm sync from external
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		cfg |= PCM_PCM_CFG_CLKOUT_EN; // pcm clock from internal
        cfg &= ~PCM_PCM_CFG_EXT_FSYNC; // pcm sync from internal
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		return -EINVAL;
	}
 
	mt7628_pcm_write(pcm, PCM_PCM_CFG, cfg);
	return 0;
}

static int mt7628_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{

	return 0;
}

#define PCMCLOCK_OUT 0 // 256KHz  int 78   comp 64

unsigned long i2sMaster_inclk_int[11] = {
	117,     56,     52,     39,     28,     26,     19,     14,     13,     9,      6};
unsigned long i2sMaster_inclk_comp[11] = {
	0,     352,    42,     32,     176,    21,     272,    88,     10,     455,    261};

//FreqOut = FreqIn *(1/2) *{1 / [DIVINT+DIVCOMP/(512)]}

static int mt7628_pcm_set_sysclk(struct snd_soc_dai *dai, int clk_id,
	unsigned int freq, int dir) // 这个函数基本没用到
{
    struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);


    //When using the external clock, the frequency clock
    //should be equal to the PCM_clock out. Otherwise, the
    //PCM_CLKin should be 8.192 MHz.
	mt7628_pcm_write(pcm, PCM_DIVINT_CFG, i2sMaster_inclk_int[PCMCLOCK_OUT]);// 整数部分
	mt7628_pcm_write(pcm, PCM_DIVCOMP_CFG, i2sMaster_inclk_comp[PCMCLOCK_OUT] | PCM_DIVCOMP_CFG_CLK_EN);// 分数部分
	return 0;
}

static int mt7628_pcm_suspend(struct snd_soc_dai *dai)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;
	if (dai->active) {
		cfg = mt7628_pcm_read(pcm, PCM_GLB_CFG);
		cfg &= ~PCM_GLB_CFG_CH0_RX_EN;
		mt7628_pcm_write(pcm, PCM_GLB_CFG, cfg);
	}

	return 0;
}

static int mt7628_pcm_resume(struct snd_soc_dai *dai)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	if (dai->active) {
		cfg = mt7628_pcm_read(pcm, PCM_GLB_CFG);
		cfg |= PCM_GLB_CFG_CH0_RX_EN;
		mt7628_pcm_write(pcm, PCM_GLB_CFG, cfg);
	}

	return 0;
}

static void mt7628_init_pcm_config(struct mt7628_pcm *pcm)
{
	struct snd_dmaengine_dai_dma_data *dma_data;

	/* Playback */
	dma_data = &pcm->playback_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 6;
	dma_data->addr = pcm->phys_base + PCM_CH0_FIFO; //only use channel 0

	/* Capture */
	dma_data = &pcm->capture_dma_data;
	dma_data->maxburst = 16;
	dma_data->slave_id = 4;
	dma_data->addr = pcm->phys_base + PCM_CH0_FIFO;
}

static int mt7628_pcm_dai_probe(struct snd_soc_dai *dai)
{
	struct mt7628_pcm *pcm = snd_soc_dai_get_drvdata(dai);
	uint32_t cfg;

	mt7628_init_pcm_config(pcm);
	dai->playback_dma_data = &pcm->playback_dma_data;
	dai->capture_dma_data = &pcm->capture_dma_data;

	mt7628_pcm_write(pcm, PCM_GLB_CFG, PCM_GLB_CFG_DFT_THRES);
	mt7628_pcm_write(pcm, PCM_INT_EN, 0);// disable all interrupts

	///////////// pcm general config
	cfg = mt7628_pcm_read(pcm, PCM_PCM_CFG);
	cfg &= ~PCM_PCM_CFG_LONG_FSYNC; //short sync mode
	//cfg |= PCM_PCM_CFG_LONG_FSYNC; //long sync mode
	cfg |= PCM_PCM_CFG_FSYNC_POL; // sync high active
	cfg |= PCM_PCM_CFG_DTX_TRI;  // Tristate DTX
	//slot mode, pcm clock = 2.048M
	cfg &= ~(0x7);
	cfg |= 0x3; // 32 slots
	mt7628_pcm_write(pcm, PCM_PCM_CFG, cfg);

	////////////channel0 config///////////
	cfg = mt7628_pcm_read(pcm, PCM_CH0_CFG);

	cfg &=~(0x3ff);// raw data(16-bit),timeslot start 0

	mt7628_pcm_write(pcm,PCM_CH0_CFG,cfg);

	///////////// pcm fsync config/////////
	cfg = mt7628_pcm_read(pcm, PCM_FSYNC_CFG);
	// pol, etc.
	cfg |= PCM_CFG_FSYNC_EN;
	cfg &=~PCM_POS_CAP_DT;// if DTX_TRI=1 it should be 0
	cfg |=PCM_POS_DRV_DT; // positive drive data
	cfg &=~PCM_POS_CAP_FSYNC;// falling edge capture fsync
	cfg |= PCM_POS_DRV_FSYNC; // positive driver fsync
	cfg |= 0x1;// 1 interve

	mt7628_pcm_write(pcm, PCM_FSYNC_CFG, cfg);

	//When using the external clock, the frequency clock
	//should be equal to the PCM_clock out. Otherwise, the
	//PCM_CLKin should be 8.192 MHz.
	mt7628_pcm_write(pcm, PCM_DIVINT_CFG, i2sMaster_inclk_int[PCMCLOCK_OUT]);
	mt7628_pcm_write(pcm, PCM_DIVCOMP_CFG, i2sMaster_inclk_comp[PCMCLOCK_OUT] | PCM_DIVCOMP_CFG_CLK_EN);

	return 0;
}

static int mt7628_pcm_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops mt7628_pcm_dai_ops = {
	.startup = mt7628_pcm_startup,
	.shutdown = mt7628_pcm_shutdown,
	.trigger = mt7628_pcm_trigger,
	.hw_params = mt7628_pcm_hw_params,
	.set_fmt = mt7628_pcm_set_fmt,
	.set_sysclk = mt7628_pcm_set_sysclk,
};

#define MT7628_PCM_FMTS (SNDRV_PCM_FMTBIT_S8 |SNDRV_PCM_FMTBIT_U8|SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S16_BE)

static struct snd_soc_dai_driver mt7628_pcm_dai = {
	.probe = mt7628_pcm_dai_probe,
	.remove = mt7628_pcm_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = MT7628_PCM_FMTS,
	},
	.capture = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = MT7628_PCM_FMTS,
	},
	.symmetric_rates = 1,
	.ops = &mt7628_pcm_dai_ops,
	.suspend = mt7628_pcm_suspend,
	.resume = mt7628_pcm_resume,
};

static const struct snd_pcm_hardware mt7620_pcm_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = MT7628_PCM_FMTS,
	.period_bytes_min	= 128,//PAGE_SIZE,
	.period_bytes_max	= 64 * 1024,
	.periods_min		= 1,	//这里要与应用层代码匹配
	.periods_max		= 128,
	.buffer_bytes_max	= 128 * 1024,
	.fifo_size		= 32,
};

static const struct snd_dmaengine_pcm_config mt7628_dmaengine_pcm_config = {
	.prepare_slave_config = snd_dmaengine_pcm_prepare_slave_config,
	.pcm_hardware = &mt7620_pcm_hardware,
	.prealloc_buffer_size = 128 * PAGE_SIZE,
};

static const struct snd_soc_component_driver mt7628_pcm_component = {
	.name = "mt7628-pcm",
};

static int mt7628_pcm_dev_probe(struct platform_device *pdev)
{
	struct mt7628_pcm *pcm;
	int ret;

	dev_err(&pdev->dev, "add by telpo team\n");
	snd_dmaengine_pcm_register(&pdev->dev,
		&mt7628_dmaengine_pcm_config,
		SND_DMAENGINE_PCM_FLAG_COMPAT | SND_DMAENGINE_PCM_FLAG_NO_RESIDUE);

	pcm = kzalloc(sizeof(*pcm), GFP_KERNEL);
	if (!pcm)
		return -ENOMEM;

	pcm->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!pcm->mem) {
		ret = -ENOENT;
		goto err_free;
	}

	pcm->mem = request_mem_region(pcm->mem->start, resource_size(pcm->mem),
				pdev->name);
	if (!pcm->mem) {
		ret = -EBUSY;
		goto err_free;
	}

	pcm->base = ioremap_nocache(pcm->mem->start, resource_size(pcm->mem));
	if (!pcm->base) {
		ret = -EBUSY;
		goto err_release_mem_region;
	}

	pcm->phys_base = pcm->mem->start;

	platform_set_drvdata(pdev, pcm);
	ret = snd_soc_register_component(&pdev->dev, &mt7628_pcm_component,
					 &mt7628_pcm_dai, 1);

	if (!ret) {
		dev_err(&pdev->dev, "loaded\n");
		return ret;
	}

	dev_err(&pdev->dev, "Failed to register DAI\n");
	iounmap(pcm->base);

err_release_mem_region:
	release_mem_region(pcm->mem->start, resource_size(pcm->mem));
err_free:
	kfree(pcm);

	return ret;
}

static int mt7628_pcm_dev_remove(struct platform_device *pdev)
{
	struct mt7628_pcm *pcm = platform_get_drvdata(pdev);

	snd_soc_unregister_component(&pdev->dev);

	iounmap(pcm->base);
	release_mem_region(pcm->mem->start, resource_size(pcm->mem));

	kfree(pcm);

	snd_dmaengine_pcm_unregister(&pdev->dev);

	return 0;
}

static const struct of_device_id mt7628_pcm_match[] = {
	{ .compatible = "ralink,mt7628-pcm" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7628_pcm_match);

static struct platform_driver mt7628_pcm_driver = {
	.probe = mt7628_pcm_dev_probe,
	.remove = mt7628_pcm_dev_remove,
	.driver = {
		.name = "mt7628-pcm",
		.owner = THIS_MODULE,
		.of_match_table = mt7628_pcm_match,
	},
};

module_platform_driver(mt7628_pcm_driver);

MODULE_AUTHOR("Derek Quan <qdk0901@qq.com>");
MODULE_DESCRIPTION("MT7628 PCM Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mt7628-pcm");
