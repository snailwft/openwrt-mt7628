/*
 * Copyright (C) 2009, Lars-Peter Clausen <lars@metafoo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>


static const struct snd_soc_dapm_widget mt7628_pcm_machine_widgets[] = {

};

static const struct snd_soc_dapm_route mt7628_pcm_machine_routes[] = {

};

#define MT7628_DAIFMT (SND_SOC_DAIFMT_NB_NF | \
			SND_SOC_DAIFMT_CBM_CFM) //mt7628 as master

static int mt7628_pcm_machine_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int mt7628_pcm_machine_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;
    
	ret = snd_soc_dai_set_fmt(cpu_dai, MT7628_DAIFMT);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set cpu dai format: %d\n", ret);
		return ret;
	}
}

static struct snd_soc_ops mt7628_pcm_machine_ops = {
	.hw_params = mt7628_pcm_machine_hw_params,
};

#if 0
static struct snd_soc_dai_link mt7628_pcm_machine_dai = { // 这里定义两个
	.name = "mt7628-pcm-machine",
	.stream_name = "mt7628-pcm-stream",
    .ops = &mt7628_pcm_machine_ops,
	.init = mt7628_pcm_machine_init,
	.codec_dai_name = "mt7628-pcm-codec",  // 这个针对codec.c mt7628_pcm_codec_dai .name
};
#endif
static struct snd_soc_dai_link mt7628_pcm_machine_dai[] = {
	{ // 这里定义两个
		.name = "mt7628-pcm-machine1",
		.stream_name = "mt7628-pcm-stream1",
		.cpu_dai_name	= "mt7628-pcm1",
	    .ops = &mt7628_pcm_machine_ops,
		.init = mt7628_pcm_machine_init,
		.codec_dai_name = "mt7628-pcm-codec",  // 这个针对codec.c mt7628_pcm_codec_dai .name
	},
	{ // 这里定义两个
		.name = "mt7628-pcm-machine2",
		.stream_name = "mt7628-pcm-stream2",
		.cpu_dai_name	= "mt7628-pcm2",
	    .ops = &mt7628_pcm_machine_ops,
		.init = mt7628_pcm_machine_init,
		.codec_dai_name = "mt7628-pcm-codec",  // 这个针对codec.c mt7628_pcm_codec_dai .name
	},
};


static struct snd_soc_card mt7628_pcm_card = {
	.name = "mt7628-pcm-machine",
	.owner = THIS_MODULE,
	.dai_link = mt7628_pcm_machine_dai,  
	.num_links = 2,		//修改为2

	.dapm_widgets = mt7628_pcm_machine_widgets,
	.num_dapm_widgets = ARRAY_SIZE(mt7628_pcm_machine_widgets),
	.dapm_routes = mt7628_pcm_machine_routes,
	.num_dapm_routes = ARRAY_SIZE(mt7628_pcm_machine_routes),
};

static int mt7628_pcm_machine_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct snd_soc_card *card = &mt7628_pcm_card;
	int ret;
	dev_err(&pdev->dev, ".............helloworld.................\n");

	card->dev = &pdev->dev;

	mt7628_pcm_machine_dai[0].cpu_of_node = of_parse_phandle(np, "cpu-dai", 0);
	mt7628_pcm_machine_dai[0].codec_of_node = of_parse_phandle(np, "codec-dai", 0);
	mt7628_pcm_machine_dai[0].platform_of_node = mt7628_pcm_machine_dai[0].cpu_of_node;
	
	mt7628_pcm_machine_dai[1].cpu_of_node = of_parse_phandle(np, "cpu-dai", 0);
	mt7628_pcm_machine_dai[1].codec_of_node = of_parse_phandle(np, "codec-dai", 0);
	mt7628_pcm_machine_dai[1].platform_of_node = mt7628_pcm_machine_dai[1].cpu_of_node;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",ret);
	}
	return ret;
}

static int mt7628_pcm_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	return 0;
}

static const struct of_device_id mt7628_pcm_machine_match[] = {
	{ .compatible = "ralink,mt7628-pcm-machine" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7628_pcm_machine_match);

static struct platform_driver mt7628_pcm_machine_driver = {
	.driver		= {
		.name	= "mt7628-pcm-machine",
		.owner	= THIS_MODULE,
		.of_match_table = mt7628_pcm_machine_match,
	},
	.probe		= mt7628_pcm_machine_probe,
	.remove		= mt7628_pcm_machine_remove,
};

module_platform_driver(mt7628_pcm_machine_driver);

MODULE_AUTHOR("Derek Quan <qdk0901@qq.com>");
MODULE_DESCRIPTION("ALSA SoC Sxx Audio support");
MODULE_LICENSE("GPL v2");
