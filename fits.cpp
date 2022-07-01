#include "include/fits.h"
#include <iostream>


int Fits::getNumOfImagesPlanes(fitsfile *fptr,int *imgs,int *planes,int *retstatus,string& header)
{
  int status=0;
  int nAxis;
  long nAxes[4] = {0};
  int bitDepth = 0;
  int hduCount;
  int hduType;
  int imgidx = 0;
  int nplanes=1;
  
  int undefined=0;
  int anynul;
  char *nullarray;
  double *pixels;
  long countnull=0;
  long npix;
  VImage img;
  VImage undefimg;
  *retstatus=0;
  header = "";
  int nkeys, ii;
  char card[FLEN_CARD];
    // Get the number of HDUs in the file
    if (fits_get_num_hdus(fptr, &hduCount, &status))
    {
        *retstatus=status;        
        cout<<hduCount;
        return GETHDU_ERROR;
    }
    // Iterate over all the HDUs of FIT File
    for (unsigned int i = 1; i <= hduCount ; i++)
    {
        // We will be getting any type of HDU, Images and Tables
        hduType = ANY_HDU;
        
        fits_get_hdrspace(fptr, &nkeys, NULL, &status); /* get # of keywords */
        for (ii = 1; ii <= nkeys; ii++)
        {
            /* Read each keywords */
            if (fits_read_record(fptr, ii, card, &status))
                break;
            header += string(card) + "\n";
        }
        //Go to the i-th HDU
        if (fits_movabs_hdu(fptr, i, &hduType, &status))
        {
          *retstatus=status;
          return HDUPOS_ERROR;
        }
        // Image HDU Found
        if (IMAGE_HDU == hduType && !fits_get_img_dim(fptr, &nAxis, &status))
        {
            // Make sure we don't do an address violation.
            if (nAxis <= 4)
            {
                // Get the size of the image
                if (fits_get_img_size(fptr, 4, nAxes, &status))
                {
                    *retstatus=status;
                    return IMGSIZE_ERROR;
                }
                // Only accept images with NAXIS < 4 or the special case NAXIS = 4 and NAXIS4 = 1
                if (nAxis > 1 && ((nAxis == 4 && nAxes[3] == 1) || nAxis < 4))
                {
                    if (nAxes[2] == 0)
                        nAxes[2] = 1;
                   nplanes=nAxes[2];
                   imgidx++;              
                }
              
            }
          }
    }
    *imgs=imgidx;
    *planes=nplanes;
    return 0;
}
int Fits::getImage(fitsfile *fptr,int ix,int px,VImage& outimg,VImage& undef,int *retstatus,long *nullvalues,int *sizepix,string outpath,long max_mem)
{
    vips_cache_set_max (0);
  int status=0;
  int numofImages;
  int numofPlanes;
  int nAxis;
  long nAxes[4] = {0};
  int bitDepth = 0;
  int hduCount;
  int hduType;
  int imgidx = 1;
  int foundimg = 0;
  int undefined=0;
  int anynul;
  char *nullarray;
  double *pixels;
  long countnull=0;
  long long npix;
  long width;
  long height;
  long long totalPixels=0;
  int isbig=0;
  *retstatus=0;
  long long sizeofpixel;
  string outpathimage=outpath+".vips";
  string outpathalfa=outpath+"a.vips";
  long long max_memory=(long long) 1048576*max_mem;

    // Get the number of HDUs in the file
    if (fits_get_num_hdus(fptr, &hduCount, &status))
    {
        *retstatus=status;        
        cout<<hduCount;
        return GETHDU_ERROR;
    }
    // Iterate over all the HDUs of FIT File
    for (unsigned int i = 1; i <= hduCount && !foundimg; i++)
    {
        // We will be getting any type of HDU, Images and Tables
        hduType = ANY_HDU;
        //Go to the i-th HDU
        if (fits_movabs_hdu(fptr, i, &hduType, &status))
        {
          *retstatus=status;
          return HDUPOS_ERROR;
        }
        // Image HDU Found
        if (IMAGE_HDU == hduType && !fits_get_img_dim(fptr, &nAxis, &status))
        {
            // Make sure we don't do an address violation.
            if (nAxis <= 4)
            {
                // Get the size of the image
                if (fits_get_img_size(fptr, 4, nAxes, &status))
                {
                    *retstatus=status;
                    return IMGSIZE_ERROR;
                }
                // Only accept images with NAXIS < 4 or the special case NAXIS = 4 and NAXIS4 = 1
                if (nAxis > 1 && ((nAxis == 4 && nAxes[3] == 1) || nAxis < 4))
                {
                    // Get the bit depth, using this method we don't have to check the
                    // values of BSCALE & BZERO
                    if (fits_get_img_equivtype(fptr, &bitDepth, &status))
                    {
                        *retstatus=status;
                        return BITDEPTH_ERROR;
                    }     
                    
                    //Calculate the number of Pixels for each image plane
                    if (imgidx == ix)
                    {
                        VImage img;
                        VImage undefimg;
                        width=nAxes[0];
                        height=nAxes[1];
                        totalPixels=height*width;
                        sizeofpixel = 0;
                        VipsBandFormat format=VIPS_FORMAT_DOUBLE;
                        bitDepth=DOUBLE_IMG;
                        sizeofpixel = abs(bitDepth);
                                
                        // Some files does not have number of planes
                        if (nAxes[2] == 0)
                            nAxes[2] = 1;
                        // plane does not exists
                        if (px>nAxes[2] || px<1)
                        {
                            return IMGPLANE_ERROR;
                        }
                        if (totalPixels*8>max_memory)
                        {
                            isbig=1;
                            //low memory consumption version
                            // keep vips image objects in a block for memory release
                            {       
                                VImage black=VImage::black(width,height,VImage::option()->set("bands", 1));
                                black=black.cast(VIPS_FORMAT_FLOAT);
                                black.vipssave((char *)&outpathimage[0]);
                                VImage blackundef=VImage::black(width,height,VImage::option()->set("bands", 1));
                                blackundef=blackundef.cast(VIPS_FORMAT_UCHAR);
                                blackundef.vipssave((char *)&outpathalfa[0]);
                            }
                            img=VImage::vipsload((char *)&outpathimage[0]);
                            undefimg=VImage::vipsload((char *)&outpathalfa[0]);

                            long k, j,x,y;
                            long tilesize=width*8;
                            long tiles=max_memory/tilesize;
                            long last_tile=height%tiles;
                            long tiles_count=height/tiles;
                            long count=0;
                            npix=width*tiles;
                            pixels = new double[npix];
                            nullarray = new char[npix];
                            y=1;

                            while (y<=height)
                            {
                                if (count==tiles_count)
                                {
                                 tiles=last_tile;    
                                 npix=tiles*width;
                                }
                                for(k=0;k<npix;k++) 
                                {
                                    nullarray[k] = 0;
                                    pixels[k]=0;
                                }
                                long topLeft[4] = {1, y, px, 1};
                                fits_read_pixnull(fptr, TDOUBLE, topLeft, npix, pixels, nullarray, &anynul, &status);

                                for (k=0;k<npix;k++)
                                {
                                countnull+=nullarray[k];
                                }
                                VImage tileundef = VImage::new_from_memory(nullarray, npix, width, tiles, 1,VIPS_FORMAT_UCHAR);
                                undefimg.draw_image(tileundef,0,y-1);    
                                VImage tileimage=VImage::new_from_memory(pixels, npix  * 8, width, tiles, 1, format);
                                tileimage=tileimage.cast(VIPS_FORMAT_FLOAT);
                                img.draw_image(tileimage,0,y-1);
                                y+=tiles;
                                count++;
                            }
                            

                        }
                        else
                        {
                            npix=width*height;
                            pixels = new double[npix];
                            nullarray = new char[npix];
                            for(long i=0;i<npix;i++) nullarray[i] = 0;
                            long topLeft[4] = {1, 1, px, 1};
                            format = VIPS_FORMAT_DOUBLE;
                            fits_read_pixnull(fptr, TDOUBLE, topLeft, npix, pixels, nullarray, &anynul, &status);
                            if (anynul==1)
                            {
                                for (long i=0;i<npix;i++)
                                {
                                    countnull+=nullarray[i];
                                }
                                undef = VImage::new_from_memory(nullarray, npix, nAxes[0], nAxes[1], 1,VIPS_FORMAT_UCHAR);
                            }
                            outimg = VImage::new_from_memory(pixels, nAxes[0] * nAxes[1] * 8, nAxes[0], nAxes[1], 1, format);
                            outimg = outimg.cast(VIPS_FORMAT_FLOAT);
                        }
                             
                        foundimg = 1;
                    }
                  imgidx++; 
                }
            }
             
        }
    }
    
    if  (!foundimg)
    {
        return IMGIDX_ERROR;
    }
    // There is no Error, so we return image, nullimage and undefined array if exists
   // outimg=img;
   if (isbig==1)
   {
    outimg=VImage::vipsload((char *)&outpathimage[0]);

    if (countnull>0)
    {
        undef=VImage::vipsload((char *)&outpathalfa[0]);
    }
   }
    *nullvalues=countnull;
    *sizepix=sizeofpixel;
    return totalPixels;
}

