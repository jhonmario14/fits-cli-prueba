#include <iostream>
#include "fitsio.h"
#include <string.h> 
#include <vips/vips8>
using namespace vips;
using namespace std;
#define BYTE_IMG 8
#define SHORT_IMG 16
#define LONG_IMG 32
#define LONGLONG_IMG 64
#define FLOAT_IMG -32
#define DOUBLE_IMG -64
class VipsOperations
{
    
    public:
    static VImage linear(VImage image,double minvalue,double maxvalue);
    static VImage linear(VImage image,double maxvalue);
    static VImage linear(VImage image,double bglevel,double pklevel,double maxvalue);
    static VImage asinh(VImage image,double bglevel,double pklevel,double maxvalue);
    static VImage log(VImage image,double bglevel,double pklevel,double maxvalue);
    static VImage pow(VImage image,double bglevel,double pklevel,double exp);
    static void correct_stats(long nullvalues,long npix,double& mean,double& stdev);
};
