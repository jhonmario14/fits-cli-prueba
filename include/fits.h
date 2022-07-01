#include <vips/vips8>
#include <stdint.h>
#include "fitsio.h"
#define FILE_ERROR -1
#define GETHDU_ERROR -2;
#define HDUPOS_ERROR -3;
#define IMGIDX_ERROR -4;
#define IMGPLANE_ERROR -5;
#define BITDEPTH_ERROR -6;
#define IMGSIZE_ERROR -7;
using namespace vips;
using namespace std;
class Fits 
{  
    public:
   
        int getNumOfImagesPlanes(fitsfile *fptr,int *imgs,int *planes,int *retstatus,string& header);
        int getImage(fitsfile *fptr,int imgidx,int planeidx,VImage& outimg,VImage& undef,int *retstatus,long *nullvalues,int *sizepix,string outpath,long max_mem);



};