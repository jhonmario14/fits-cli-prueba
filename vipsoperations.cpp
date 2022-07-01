#include "include/vipsoperations.h"


VImage VipsOperations::linear(VImage image,double maxvalue)
{
        VImage stats=image.stats();
        double min=stats(0,0)[0];
        double max=stats(1,0)[0];
        double f = maxvalue / (max - min);
        double a = -(min * f) ;
        return image.linear(f,a);
}
VImage VipsOperations::linear(VImage image,double minvalue,double maxvalue)
{
        VImage stats=image.stats();
        double min=stats(0,0)[0];
        double max=stats(1,0)[0];
        double f = (maxvalue-minvalue) / (max - min);
        double a = -(min * f) ;
        
        return image.linear(f,a);

}
VImage VipsOperations::linear(VImage image,double bglevel,double pklevel,double maxvalue)
{
                double f = maxvalue / (pklevel - bglevel);
                double a = -(bglevel * f);
                return image.linear(f,a);
        }
VImage VipsOperations::asinh(VImage image,double bglevel,double pklevel,double maxvalue)
{
        double f = maxvalue / (pklevel - bglevel);
        double a = -(bglevel * f);
        VImage scaled=image.linear(f,a);
        VImage stretched=scaled.pow(2);
        stretched=stretched.linear(1,1);
        stretched=stretched.pow(0.5);
        stretched=stretched.add(scaled);
        return stretched.log();
}
VImage VipsOperations::log(VImage image,double bglevel,double pklevel,double maxvalue)
{
        double f = maxvalue / (pklevel - bglevel);
        double a = -(bglevel * f);
        VImage stretched=image.linear(f,a);
        /*VImage stretched=scaled.pow(2);
        stretched=stretched.linear(1,1);
        stretched=stretched.pow(0.5);*/
        VImage imgadd=image.new_from_image(1);
        stretched=stretched.add(imgadd);
        return stretched.log10();
}

VImage VipsOperations::pow(VImage image,double bglevel,double pklevel,double exp)
{
    double f = pklevel / (pklevel - bglevel);
    double a = -(bglevel * f) ;
    
    VImage scaled=image.linear(f,a);
    VImage signs=scaled.sign();
    VImage stretched=scaled.abs().pow(exp);                    
    if (exp<1)
    {
     return stretched.multiply(signs);
    }
    else
    {
     return stretched;       
    }
     
    
}
void VipsOperations::correct_stats(long nullvalues,long npix,double& mean,double& stdev)
{
        double tempmean=mean;
        double sum=mean*npix;
        double corrected=sum/(npix-nullvalues);
        mean=corrected;
        double root=sqrt(npix);
        sum=root*stdev;
        sum=sum*sum+tempmean;
        sum=sum-stdev;
        stdev=sqrt(sum/(npix-nullvalues));



}