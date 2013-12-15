





#include "main.h"
#include "image.h"
#include "model.h"
#include "map.h"
#include "particle.h"
#include "selection.h"
#include "projectile.h"
#include "unit.h"
#include "building.h"
#include "gui.h"
#include "save.h"

CTexture g_texture[TEXTURES];
vector<tTextureToLoad> g_texLoad;

int g_texwidth;
int g_texheight;
int g_texindex;

char jpegBuffer[JPEG_BUFFER_SIZE];
JPEGSource   jpegSource;
FILE* g_src;
//CFile g_src;
int srcLen;

tImage *LoadBMP(const char *strFileName)
{
	AUX_RGBImageRec *pBitmap = NULL;
	FILE *pFile = NULL;

	// Open a file pointer to the BMP file and check if it was found and opened 
	if((pFile = fopen(strFileName, "rb")) == NULL) 
	{
		// Display an error message saying the file was not found, then return NULL
		MessageBox(g_hWnd, ("Unable to load BMP File!"), ("Error"), MB_OK);
		return NULL;
	}

	// Load the bitmap using the aux function stored in glaux.lib
	pBitmap = auxDIBImageLoad(strFileName);				

	// Allocate the tImage
	tImage *pImage = (tImage *)malloc(sizeof(tImage));

	// Assign the channels, width, height and image bits to pImage
	pImage->channels = 3;
	pImage->sizeX = pBitmap->sizeX;
	pImage->sizeY = pBitmap->sizeY;
	pImage->data  = pBitmap->data;

	// Free the bitmap pointer (The "data" will be freed later in CreateTexture())
	free(pBitmap);
	
	/*
	int stride = pImage->channels * pBitmap->sizeX;
	int i;
	int y2;
	int temp;

	for(int y = 0; y < pImage->sizeY/2; y++)
	{
		y2 = pImage->sizeY - y;
		// Store a pointer to the current line of pixels
		unsigned char *pLine = &(pImage->data[stride * y]);
		unsigned char *pLine2 = &(pImage->data[stride * y2]);
			
		// Go through all of the pixels and swap the B and R values since TGA
		// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
		for(i = 0; i < stride; i += pImage->channels)
		{
			temp = pLine[i];
			pLine[i] = pLine2[i];
			pLine2[i] = temp;
			
			temp = pLine[i+1];
			pLine[i+1] = pLine2[i+1];
			pLine2[i+1] = temp;
			
			temp = pLine[i+2];
			pLine[i+2] = pLine2[i+2];
			pLine2[i+2] = temp;
		}
	}*/

	return pImage;
}

tImage *LoadTGA(const char *strFileName)
{
	tImage *pImageData = NULL;			// This stores our important image data
	WORD width = 0, height = 0;			// The dimensions of the image
	byte length = 0;					// The length in bytes to the pixels
	byte imageType = 0;					// The image type (RLE, RGB, Alpha...)
	byte bits = 0;						// The bits per pixel for the image (16, 24, 32)
	FILE *pFile = NULL;					// The file pointer
	int channels = 0;					// The channels of the image (3 = RGA : 4 = RGBA)
	int stride = 0;						// The stride (channels * width)
	int i = 0;							// A counter

	// Open a file pointer to the targa file and check if it was found and opened 
	if((pFile = fopen(strFileName, "rb")) == NULL) 
	{
		// Display an error message saying the file was not found, then return NULL
		MessageBox(g_hWnd, ("Unable to load TGA File!"), ("Error"), MB_OK);
		return NULL;
	}
		
	// Allocate the structure that will hold our eventual image data (must free it!)
	pImageData = (tImage*)malloc(sizeof(tImage));

	// Read in the length in bytes from the header to the pixel data
	fread(&length, sizeof(byte), 1, pFile);
	
	// Jump over one byte
	fseek(pFile,1,SEEK_CUR); 

	// Read in the imageType (RLE, RGB, etc...)
	fread(&imageType, sizeof(byte), 1, pFile);
	
	// Skip past general information we don't care about
	fseek(pFile, 9, SEEK_CUR); 

	// Read the width, height and bits per pixel (16, 24 or 32)
	fread(&width,  sizeof(WORD), 1, pFile);
	fread(&height, sizeof(WORD), 1, pFile);
	fread(&bits,   sizeof(byte), 1, pFile);
	
	// Now we move the file pointer to the pixel data
	fseek(pFile, length + 1, SEEK_CUR); 

	// Check if the image is RLE compressed or not
	if(imageType != TGA_RLE)
	{
		// Check if the image is a 24 or 32-bit image
		if(bits == 24 || bits == 32)
		{
			// Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
			// Next, we calculate the stride and allocate enough memory for the pixels.
			channels = bits / 8;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			// Load in all the pixel data line by line
			for(int y = 0; y < height; y++)
			{
				// Store a pointer to the current line of pixels
				unsigned char *pLine = &(pImageData->data[stride * y]);

				// Read in the current line of pixels
				fread(pLine, stride, 1, pFile);
			
				// Go through all of the pixels and swap the B and R values since TGA
				// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
				for(i = 0; i < stride; i += channels)
				{
					int temp     = pLine[i];
					pLine[i]     = pLine[i + 2];
					pLine[i + 2] = temp;
				}
			}
		}
		// Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
		else if(bits == 16)
		{
			unsigned short pixels = 0;
			int r=0, g=0, b=0;

			// Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
			// We then calculate the stride and allocate memory for the pixels.
			channels = 3;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			// Load in all the pixel data pixel by pixel
			for(int i = 0; i < width*height; i++)
			{
				// Read in the current pixel
				fread(&pixels, sizeof(unsigned short), 1, pFile);
				
				// Convert the 16-bit pixel into an RGB
				b = (pixels & 0x1f) << 3;
				g = ((pixels >> 5) & 0x1f) << 3;
				r = ((pixels >> 10) & 0x1f) << 3;
				
				// This essentially assigns the color to our array and swaps the
				// B and R values at the same time.
				pImageData->data[i * 3 + 0] = r;
				pImageData->data[i * 3 + 1] = g;
				pImageData->data[i * 3 + 2] = b;
			}
		}	
		// Else return a NULL for a bad or unsupported pixel format
		else
			return NULL;
	}
	// Else, it must be Run-Length Encoded (RLE)
	else
	{
		// Create some variables to hold the rleID, current colors read, channels, & stride.
		byte rleID = 0;
		int colorsRead = 0;
		channels = bits / 8;
		stride = channels * width;

		// Next we want to allocate the memory for the pixels and create an array,
		// depending on the channel count, to read in for each pixel.
		pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));
		byte *pColors = ((byte*)malloc(sizeof(byte)*channels));

		// Load in all the pixel data
		while(i < width*height)
		{
			// Read in the current color count + 1
			fread(&rleID, sizeof(byte), 1, pFile);
			
			// Check if we don't have an encoded string of colors
			if(rleID < 128)
			{
				// Increase the count by 1
				rleID++;

				// Go through and read all the unique colors found
				while(rleID)
				{
					// Read in the current color
					fread(pColors, sizeof(byte) * channels, 1, pFile);

					// Store the current pixel in our image array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}
			}
			// Else, let's read in a string of the same character
			else
			{
				// Minus the 128 ID + 1 (127) to get the color count that needs to be read
				rleID -= 127;

				// Read in the current color, which is the same for a while
				fread(pColors, sizeof(byte) * channels, 1, pFile);

				// Go and read as many pixels as are the same
				while(rleID)
				{
					// Assign the current pixel to the current index in our pixel array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}
				
			}
				
		}
	}

	// Close the file pointer that opened the file
	fclose(pFile);

	// Flip upside-down
	int x;
	int y2;
	byte temp[4];
	for(int y=0; y<height/2; y++)
	{
		y2 = height - y - 1;
		
		unsigned char *pLine = &(pImageData->data[stride * y]);
		unsigned char *pLine2 = &(pImageData->data[stride * y2]);

		for(x=0; x<width*channels; x+=channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(bits == 32)
				temp[3] = pLine[x + 3];
			
			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(bits == 32)
				pLine[x + 3] = pLine2[x + 3];
			
			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(bits == 32)
				pLine2[x + 3] = temp[3];
		}
	}

	// Fill in our tImage structure to pass back
	pImageData->channels = channels;
	pImageData->sizeX    = width;
	pImageData->sizeY    = height;

	// Return the TGA data (remember, you must free this data after you are done)
	return pImageData;
}

void DecodeJPG(jpeg_decompress_struct* cinfo, tImage *pImageData)
{
	// Read in the header of the jpeg file
	jpeg_read_header(cinfo, TRUE);
	
	// Start to decompress the jpeg file with our compression info
	jpeg_start_decompress(cinfo);

	// Get the image dimensions and channels to read in the pixel data
	pImageData->channels = cinfo->num_components;
	pImageData->sizeX    = cinfo->image_width;
	pImageData->sizeY    = cinfo->image_height;

	// Get the row span in bytes for each row
	int rowSpan = cinfo->image_width * cinfo->num_components;
	
	// Allocate memory for the pixel buffer
	pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizeY));
			
	// Create an array of row pointers
	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];

	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i * rowSpan]);

	// Now comes the juice of our work, here we extract all the pixel data
	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height) 
	{
		// Read in the current row of pixels and increase the rowsRead count
		rowsRead += jpeg_read_scanlines(cinfo, 
										&rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	
	// Delete the temporary row pointers
	delete [] rowPtr;

	// Finish decompressing the data
	jpeg_finish_decompress(cinfo);
}

bool source_init(const char *filename) 
{
	g_log<<"source_init "<<filename<<endl;
	
    g_src = fopen(filename, "rb");

    if (g_src == NULL) return 0;

    fseek(g_src, 0, SEEK_END);

    srcLen = ftell(g_src);

    fseek(g_src, 0, SEEK_SET);

	/*
	//g_src = CFile(filename);

	//if(g_src.fsize <= 0)
	if(!g_src.mFile)
		return false;

	srcLen = g_src.remain();

	LOGI("source_init size=%d", srcLen);
	*/
    return true;
}

void source_close() 
{
	g_log<<"source_close"<<endl;;
		g_log.flush();
    fclose(g_src);
	//g_src.close();
}

int source_read(char* buffer) 
{
    //int len = min(JPEG_BUFFER_SIZE, g_src.fsize-g_src.position);
	//int len = min(JPEG_BUFFER_SIZE, 

    //fread(buffer, len, 1, g_src);
	//g_src.read((void*)buffer, len);

    //return len;
	//LOGI("source_read");
	
	//return g_src.read((void*)buffer, JPEG_BUFFER_SIZE);

	/*
	LOGI("source_read...");
	
	int toread = JPEG_BUFFER_SIZE;
	if(g_src.remain() < toread)
		toread = g_src.remain();

	LOGI("source_read %d", toread);

	//return g_src.read((void*)buffer, toread);
	int ret = g_src.read((void*)buffer, toread);

	LOGI("read %d", ret);
	
	return ret;
	*/

	 int len = JPEG_BUFFER_SIZE;

    if (len > srcLen) len = srcLen;

    srcLen -= len;

	g_log<<"source_read "<<len<<endl;
		g_log.flush();

    fread(buffer, len, 1, g_src);

    return len;
}

void source_seek(int num) 
{
	g_log<<"source_seek "<<num<<endl;;
		g_log.flush();
    fseek(g_src, num, SEEK_CUR);
	//g_src.seek(num);
}

static void init_sourceFunc(j_decompress_ptr cinfo) 
{
	g_log<<"init_sourceFunc"<<endl;
		g_log.flush();
    ((JPEGSource*)cinfo->src)->pub.bytes_in_buffer = 0;
}

static boolean fill_input_bufferFunc(j_decompress_ptr cinfo) 
{
	g_log<<"fill_input_bufferFunc"<<endl;
		g_log.flush();
    JPEGSource  *src = (JPEGSource*)cinfo->src;

    src->pub.bytes_in_buffer = source_read(jpegBuffer);

    src->pub.next_input_byte = (const unsigned char*)jpegBuffer;

    return TRUE;
}

void skip_input_dataFunc(j_decompress_ptr cinfo, long num_bytes) 
{
	g_log<<"skip_input_dataFunc "<<(int)num_bytes<<endl;
		g_log.flush();
    JPEGSource  *src = (JPEGSource*)cinfo->src;

    if (num_bytes > 0) 
	{
        source_seek(num_bytes);

        if (num_bytes > src->pub.bytes_in_buffer) src->pub.bytes_in_buffer = 0;

        else 
		{
            src->pub.next_input_byte += num_bytes;
            src->pub.bytes_in_buffer -= num_bytes;
        }
    }
}

void term_sourceFunc(j_decompress_ptr cinfo) 
{
	g_log<<"term_sourceFunc"<<endl;
		g_log.flush();
}

tImage *LoadJPG2(const char *strFileName)
{
		g_log<<"JPG "<< strFileName<<" 0"<<endl;
		g_log.flush();

	tImage *pImageData = NULL;
    struct jpeg_decompress_struct cinfo;

    jpeg_error_mgr jerr;
	
	g_log<<"JPEG_LIB_VERSION = "<<JPEG_LIB_VERSION<<endl;
	
		g_log<<"JPG "<< strFileName<<" 1"<<endl;
		g_log.flush();

    if (!source_init(strFileName))
	{
		g_log<<"Error opening jpeg "<<strFileName<<endl;
		return NULL;
	}
	
		g_log<<"JPG "<< strFileName<<" 2"<<endl;
		g_log.flush();

	pImageData = (tImage*)malloc(sizeof(tImage));

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpegSource.pub.init_source = init_sourceFunc;
    jpegSource.pub.fill_input_buffer = fill_input_bufferFunc;
    jpegSource.pub.skip_input_data = skip_input_dataFunc;
    jpegSource.pub.resync_to_restart = jpeg_resync_to_restart;
    jpegSource.pub.term_source = term_sourceFunc;
    jpegSource.pub.next_input_byte = NULL;
    jpegSource.pub.bytes_in_buffer = 0;
    cinfo.src = (struct jpeg_source_mgr*)&jpegSource;
	
		g_log<<"JPG "<< strFileName<<" 3"<<endl;
		g_log.flush();

    jpeg_read_header(&cinfo, TRUE);
	
		g_log<<"JPG "<< strFileName<<" 3.1"<<endl;
		g_log.flush();

    jpeg_start_decompress(&cinfo);
	
		g_log<<"JPG "<< strFileName<<" 4"<<endl;
		g_log.flush();

    pImageData->channels = cinfo.num_components;
    pImageData->sizeX    = cinfo.image_width;
    pImageData->sizeY    = cinfo.image_height;

    //printf("%d %d\n", pImageData.sizeX, pImageData.sizeY);

    int rowSpan = cinfo.image_width * cinfo.num_components;
	
		g_log<<"JPG "<< strFileName<<" 5"<<endl;
		g_log.flush();

    pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizeY));

    unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];

    for (int i = 0; i < pImageData->sizeY; i++)
        rowPtr[i] = &(pImageData->data[i * rowSpan]);
	
		g_log<<"JPG "<< strFileName<<" 6"<<endl;
		g_log.flush();

    int rowsRead = 0;

    while (cinfo.output_scanline < cinfo.output_height)
        rowsRead += jpeg_read_scanlines(&cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead);

	
		g_log<<"JPG "<< strFileName<<" 7"<<endl;
		g_log.flush();

    delete [] rowPtr;

    //free(pImageData->data);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
	
		g_log<<"JPG "<< strFileName<<" 8"<<endl;
		g_log.flush();

    source_close();
	
		g_log<<"JPG "<< strFileName<<" 9"<<endl;
		g_log.flush();

    return pImageData;
}

tImage *LoadJPG(const char *strFileName)
{
	struct jpeg_decompress_struct cinfo;
	tImage *pImageData = NULL;
	FILE *pFile;
		
	// Open a file pointer to the jpeg file and check if it was found and opened 
	if((pFile = fopen(strFileName, "rb")) == NULL) 
	{
		// Display an error message saying the file was not found, then return NULL
		//MessageBox(g_hWnd, "Unable to load JPG File!", "Error", MB_OK);
		return NULL;
	}
	
	// Create an error handler
	jpeg_error_mgr jerr;

	// Have our compression info object point to the error handler address
	cinfo.err = jpeg_std_error(&jerr);
	
	// Initialize the decompression object
	jpeg_create_decompress(&cinfo);
	
	// Specify the data source (Our file pointer)	
	jpeg_stdio_src(&cinfo, pFile);
	
	// Allocate the structure that will hold our eventual jpeg data (must free it!)
	pImageData = (tImage*)malloc(sizeof(tImage));

	// Decode the jpeg file and fill in the image data structure to pass back
	DecodeJPG(&cinfo, pImageData);
	
	// This releases all the stored memory for reading and decoding the jpeg
	jpeg_destroy_decompress(&cinfo);
	
	// Close the file pointer that opened the file
	fclose(pFile);

	// Return the jpeg data (remember, you must free this data after you are done)
	return pImageData;
}


tImage *LoadPNG(const char *strFileName)
{
	tImage *pImageData = NULL;
	
	png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    //int color_type, interlace_type;
    FILE *fp;

	/*
	if ((fp = fopen(strFileName, "rb")) == NULL)
        return NULL;
	png_byte header[8];
	fread(header, sizeof(png_byte), 8, fp);
	fclose(fp);
	
	g_log<<"PNG header "<<strFileName<<" "
		<<(int)header[0]<<","<<(int)header[1]<<","<<(int)header[2]<<","<<(int)header[3]<<","
		<<(int)header[4]<<","<<(int)header[5]<<","<<(int)header[6]<<","<<(int)header[7]<<endl;
	*/
	if ((fp = fopen(strFileName, "rb")) == NULL)
        return NULL;


	/* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL) 
	{
        fclose(fp);
        return NULL;
    }

	/* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) 
	{
        fclose(fp);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return NULL;
    }

	/* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) 
	{
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        /* If we get here, we had a
         * problem reading the file */
        return NULL;
    }

	/* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, fp);

	/* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

	/*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);
	
	pImageData = (tImage*)malloc(sizeof(tImage));

	pImageData->sizeX = png_get_image_width(png_ptr, info_ptr); //info_ptr->width;
    pImageData->sizeY = png_get_image_height(png_ptr, info_ptr); //info_ptr->height;
    //switch (info_ptr->color_type) 
	switch( png_get_color_type(png_ptr, info_ptr) )
	{
        case PNG_COLOR_TYPE_RGBA:
            pImageData->channels = 4;
            break;
        case PNG_COLOR_TYPE_RGB:
            pImageData->channels = 3;
            break;
        default:
			g_log<<strFileName<<" color type "<<png_get_color_type(png_ptr, info_ptr)<<" not supported"<<endl;
            //std::cout << "Color type " << info_ptr->color_type << " not supported" << std::endl;
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
			free(pImageData);
            return NULL;
    }

	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	pImageData->data = (unsigned char*) malloc(row_bytes * pImageData->sizeY);

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	
	for (int i = 0; i < pImageData->sizeY; i++) 
	{
        // note that png is ordered top to
        // bottom, but OpenGL expect it bottom to top
        // so the order or swapped
		
        memcpy((void*)(pImageData->data+(row_bytes * i)), row_pointers[i], row_bytes);
        //memcpy((void*)(pImageData->data+(row_bytes * (pImageData->sizeY-1-i))), row_pointers[i], row_bytes);
    }

	/* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);

	return pImageData;
}

bool FindTexture(unsigned int &texture, const char* filepath)
{
	char corrected[1024];
	strcpy(corrected, filepath);

	for(int i=0; i<strlen(corrected); i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';

	for(int i=0; i<TEXTURES; i++)
		if(g_texture[i].loaded && stricmp(g_texture[i].filepath, corrected) == 0)
		{
			g_texindex = i;
			texture = g_texture[i].tex;
			g_texwidth = g_texture[i].width;
			g_texheight = g_texture[i].height;
			return true;
		}

	return false;
}

int NewTexture()
{
	for(int i=0; i<TEXTURES; i++)
		if(!g_texture[i].loaded)
			return i;

	return -1;
}

void TextureLoaded(unsigned int texture, const char* filepath, int id)
{
	char corrected[1024];
	strcpy(corrected, filepath);

	for(int i=0; i<strlen(corrected); i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';

	int i = id;

	if(id < 0)
		i = NewTexture();

	if(i < 0)
		return;

	g_texindex = i;

	g_texture[i].loaded = true;
	strcpy(g_texture[i].filepath, corrected);
	g_texture[i].tex = texture;
	g_texture[i].width = g_texwidth;
	g_texture[i].height = g_texheight;
}

void FreeTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		if(!g_texture[i].loaded)
			continue;

		glDeleteTextures(1, &g_texture[i].tex);
		g_texture[i].loaded = false;
	}
}

void FindTextureExtension(char *strFileName)
{
	char strJPGPath[MAX_PATH] = {0};
	char strPNGPath[MAX_PATH] = {0};
	char strTGAPath[MAX_PATH]    = {0}; 
	char strBMPPath[MAX_PATH]    = {0}; 
	FILE *fp = NULL;

	//GetCurrentDirectory(MAX_PATH, strJPGPath);

	//strcat(strJPGPath, "\\");
	FullPath("", strJPGPath);

	strcat(strJPGPath, strFileName);
	strcpy(strTGAPath, strJPGPath);
	strcpy(strBMPPath, strTGAPath);
	strcpy(strPNGPath, strBMPPath);
	
	strcat(strJPGPath, ".jpg");
	strcat(strTGAPath, ".tga");
	strcat(strBMPPath, ".bmp");
	strcat(strPNGPath, ".png");

	if((fp = fopen(strJPGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(strFileName, ".jpg");
		return;
	}
	
	if((fp = fopen(strPNGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(strFileName, ".png");
		return;
	}

	if((fp = fopen(strTGAPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(strFileName, ".tga");
		return;
	}

	if((fp = fopen(strBMPPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(strFileName, ".bmp");
		return;
	}
}

int g_lastLTex = -1;
bool Load1Texture()
{
	if(g_lastLTex+1 < g_texLoad.size())
		Status(g_texLoad[g_lastLTex+1].filepath);

	if(g_lastLTex >= 0)
	{
		tTextureToLoad* t = &g_texLoad[g_lastLTex];
		CreateTexture(*t->tex, t->filepath, t->clamp);
	}

	g_lastLTex ++;

	if(g_lastLTex >= g_texLoad.size())
	{
		g_texLoad.clear();
		return true;	// Done loading all textures
	}

	return false;	// Not finished loading textures
}

void QueueTexture(unsigned int* tex, const char* strFileName, bool clamp)
{
	tTextureToLoad toLoad;
	toLoad.tex = tex;
	strcpy(toLoad.filepath, strFileName);
	toLoad.clamp = clamp;

	g_texLoad.push_back(toLoad);
}

bool CreateTexture(unsigned int &texture, const char* strFileName, bool clamp, int id)
{
	if(!strFileName) 
		return false;

	if(id < 0)
		if(FindTexture(texture, strFileName))
			return true;

	// Define a pointer to a tImage
	tImage *pImage = NULL;

	char full[1024];
	FullPath(strFileName, full);

	// If the file is a jpeg, load the jpeg and store the data in pImage
	if(strstr(strFileName, ".jpg"))
	{
		pImage = LoadJPG(full);
		//pImage = LoadJPG2(strFileName);
	}
	else if(strstr(strFileName, ".png"))
	{
		pImage = LoadPNG(full);
	}
	// If the file is a tga, load the tga and store the data in pImage
	else if(strstr(strFileName, ".tga"))
	{
		pImage = LoadTGA(full);
	}
	// If the file is a bitmap, load the bitmap and store the data in pImage
	else if(strstr(strFileName, ".bmp"))
	{
		pImage = LoadBMP(full);
	}

	// Make sure valid image data was given to pImage, otherwise return false
	if(pImage == NULL)		
	{
		g_log<<"Failed to load "<<strFileName<<"\n\r";
		g_log.flush();
		return false;
	}

	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, &texture);

	// This sets the alignment requirements for the start of each pixel row in memory.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Assume that the texture is a 24 bit RGB texture (We convert 16-bit ones to 24-bit)
	int textureType = GL_RGB;

	// If the image is 32-bit (4 channels), then we need to specify GL_RGBA for an alpha
	if(pImage->channels == 4)
		textureType = GL_RGBA;
		
	// Option 1: with mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, pImage->channels, pImage->sizeX, pImage->sizeY, textureType, GL_UNSIGNED_BYTE, pImage->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if(clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	// Option 2: without mipmaps
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);c
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizeX, pImage->sizeY, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

	// Now we need to free the image data that we loaded since openGL stored it as a texture
	if (pImage)										// If we loaded the image
	{
		g_texwidth = pImage->sizeX;
		g_texheight = pImage->sizeY;

		if (pImage->data)							// If there is texture data
		{
			free(pImage->data);						// Free the texture data, we don't need it anymore
		}

		free(pImage);								// Free the image structure

		g_log<<strFileName<<"\n\r";
		g_log.flush();
	}

	TextureLoaded(texture, strFileName, id);

	// Return a success
	return true;
}

void ReloadTextures()
{
	FreeTextures();

	for(int i=0; i<TEXTURES; i++)
	{
		if(g_texture[i].loaded)
			CreateTexture(g_texture[i].tex, g_texture[i].filepath, i);
	}
	
	LoadParticles();
	LoadProjectiles();
	LoadTerrainTextures();
	LoadUnitSprites();
	BSprites();

	for(int i=0; i<MODELS; i++)
	{
		if(g_model[i].on)
			g_model[i].ReloadTexture();
	}
}

