/*
 * es9038.c
 *
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <linux/of.h>

/*
 * Note this is a simple chip with no configuration interface, sample rate is
 * determined automatically by examining the Master clock and Bit clock ratios
 */
#define MT7628_PCM_RATES  (SNDRV_PCM_RATE_8000_48000)
#define MT7628_FORMATS (SNDRV_PCM_FMTBIT_S8|SNDRV_PCM_FMTBIT_U8|SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S16_BE)

static struct snd_soc_dai_driver mt7628_pcm_codec_dai[] = {
	{
		.name = "mt7628-pcm-codec",
		.playback = {
			.stream_name = "mt7628-pcm-codec-playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = MT7628_PCM_RATES,
			.formats = MT7628_FORMATS,
		},
        .capture = {
            .stream_name = "mt7628-pcm-codec-capture",
            .channels_min = 1,
            .channels_max = 2,
            .rates = MT7628_PCM_RATES,
            .formats = MT7628_FORMATS,
        },
	},
	{
		.name = "mt7628-pcm-codec",
		.playback = {
			.stream_name = "mt7628-pcm-codec-playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = MT7628_PCM_RATES,
			.formats = MT7628_FORMATS,
		},
        .capture = {
            .stream_name = "mt7628-pcm-codec-capture",
            .channels_min = 1,
            .channels_max = 2,
            .rates = MT7628_PCM_RATES,
            .formats = MT7628_FORMATS,
        },
	},
};

static struct snd_soc_codec_driver soc_codec_dev_mt7628_pcm;

static const struct of_device_id mt7628_pcm_codec_match[] = {
	{ .compatible = "ralink,mt7628-pcm-codec" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7628_pcm_codec_match);

static int mt7628_pcm_codec_probe(struct platform_device *pdev)
{
	dev_err(&pdev->dev, ".............helloworld.................\n");
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_mt7628_pcm, mt7628_pcm_codec_dai,1); // ARRAY_SIZE(mt7628_pcm_codec_dai)
}

static int mt7628_pcm_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver mt7628_pcm_codec_driver = {
	.driver = {
			.name = "mt7628-pcm-codec",
			.owner = THIS_MODULE,
            .of_match_table = mt7628_pcm_codec_match,
	},

	.probe = mt7628_pcm_codec_probe,
	.remove = mt7628_pcm_codec_remove,
};

module_platform_driver(mt7628_pcm_codec_driver);

MODULE_DESCRIPTION("ASoC PCM driver");
MODULE_AUTHOR("Derek Quan");
MODULE_LICENSE("GPL");
