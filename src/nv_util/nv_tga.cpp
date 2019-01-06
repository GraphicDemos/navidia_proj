#include "nv_tga.h"

#include <tga.h>
#include <stdio.h>

//write by myselt xiangtao
namespace tga
{

	char *cmap[2] = { "not color mapped", "color mapped" };
	char *imgt[7] = { "no image data included" , "uncompressed color mapped",
		"uncompressed truecolor" , "uncompressed grayscale",
		"compressed color mapped", "compressed truecolor",
		"compressed grayscale" };


	void check(TGA *tga)
	{
		printf("-> [text] %s\n", cmap[tga->hdr.map_t]);

		printf("-> [text] %s\n", (tga->hdr.img_t > 8) ?
			imgt[tga->hdr.img_t - 5] : imgt[tga->hdr.img_t]);

	}

	tgaImage * read(const char *filename)
	{

		TGA *tga = TGAOpen(filename, "r");
		if (!TGA_SUCCEEDED(tga)) 
		{
			printf("file %s tag open error :%s ", filename, TGAStrError(tga));
			return NULL;
		}

		TGAData data;
		memset(&data, 0,sizeof(data));
		data.flags = TGA_IMAGE_INFO;

		TGAReadImage(tga, &data);

		if (!TGA_SUCCEEDED(tga))
		{
			printf("file %s tag ReadImage error :%s ", filename, TGAStrError(tga));
			TGAClose(tga);
			return NULL;
		}

		tgaImage *tag_image = NULL;

		if (data.flags & TGA_IMAGE_INFO)
		{
			tag_image = new tgaImage;
			tag_image->width = tga->hdr.width;
			tag_image->height = tga->hdr.height;

			tag_image->cmapEntries = tga->hdr.map_entry;
			tag_image->cmapFormat = tga->hdr.map_t;

			tag_image->cmap = (tbyte*)realloc(tag_image->cmap, (tga)->hdr.map_len * (tga)->hdr.map_entry / 8);
			memcpy(tag_image->cmap, data.cmap, sizeof(tag_image->cmap) / sizeof(tag_image->cmap[0]));
			tag_image->pixels = new GLubyte[(((tga)->hdr.width * (tga)->hdr.height * (tga)->hdr.depth / 8))];
			memcpy(tag_image->pixels, data.img_data, sizeof(tag_image->pixels) / sizeof(tag_image->pixels[0]));
		}

		TGAFreeTGAData(&data);
		TGAClose(tga);

		return tag_image;
	}
}