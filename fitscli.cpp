#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <math.h>
#include "include/cxxopts.hpp"
#include <vips/vips8>
#include "fitsio.h"
#include "include/vipsoperations.h"
#include "include/fits.h"
#include "include/ConsoleTable.h"
#define MAXPIXELS 1e8
using namespace std;
using namespace vips;
void saveheader(string header,string filename);
string histogram(VImage img,int bins);
void report_progress(int showprogress,string message)
{
    if (showprogress==1)
      cout<<"["<<message<<"]"<<endl;
}
string getpathname(const string& s) {
   char sep = '/';
#ifdef _WIN32
   sep = '\\';
#endif
   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(0, i));
   }
   return("");
}
string getfilename(const string &s)
{
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    size_t i = s.rfind(sep, s.length());
    if (i != string::npos)
    {
        return (s.substr(i + 1, s.length() - i));
    }
    return ("");
}
string getbasename(string filename)
{
    string delimiter = ".";
    return filename.substr(0, filename.find(delimiter));
}


int main(int argc, char *argv[])
{
    cxxopts::Options options("fitscli", " - Fits Liberator CLI");
    std::string stretch;
    float exponent;
    float backgroundLevel;
    float peakLevel;
    float scaledPeakLevel;
    float blackLevel;
    float whiteLevel;
    int depth;
    string filepath;
    string outfilepath;
    int ix = 1;
    int px = 1;
    int flip=1;
    int ispreloader=0;
    int outformat=1;
    int quiet=0;
    int autolevels=0;
    int undefined=0;
    int fitsresult=0;
    int status = 0;
    long nullvalues=0;
    long npix;
    int is_wdefined=0;
    int is_bdefined=0;
    int is_pdefined=0;
    int is_bgdefined=0;
    int is_pkdefined=0;
    int sizeofpixel=0;
    int bins=0;
    int dz=0;
    int showprogress=0;
    int fastmode=1;
    VImage img,undef;
    long original_width;
    long original_height;
    long max_mem;
    int iscompressed=0;
    try
    {
        options.add_options()("i,infile", "FITS Input File", cxxopts::value<std::string>())
        ("o,outfile", "TIFF Output File", cxxopts::value<std::string>())
        ("s,stretch", "Stretch math function (asinh,linear,pow,log)", cxxopts::value<std::string>()->default_value("linear"))
        ("e,exponent", "Power Function exponent", cxxopts::value<float>()->default_value("0.5"))
        ("k,backgroundlevel", "Background Level", cxxopts::value<float>()->default_value("0"))
        ("p,peaklevel", "Peak Level", cxxopts::value<float>()->default_value("1"))
        ("S,scaledpeaklevel", "Scaled Peak Level", cxxopts::value<float>()->default_value("100"))
        ("b,blacklevel", "Black Level", cxxopts::value<float>()->default_value("0"))
        ("w,whitelevel", "White Level", cxxopts::value<float>()->default_value("1"))
        ("d,depth", "Bit depth (8, 16 or 32)", cxxopts::value<int>()->default_value("16"))
        ("I,image", "Image index for multi-image files", cxxopts::value<int>()->default_value("1"))
        ("z,plane", "Plane index for multi-plane images", cxxopts::value<int>()->default_value("1"))
        ("f,flip", "Flip the image vertically", cxxopts::value<int>()->default_value("1"))
        ("F,outformat", "Output format 1=tiff, 2=png", cxxopts::value<int>()->default_value("1"))
        ("q,quiet", "Quiet execution without echo", cxxopts::value<int>()->default_value("0"))
        ("a,autolevels", "Autoselect white & black levels using statistics", cxxopts::value<int>()->default_value("0"))
        ("u,undefined", "Alpha channel for undefined values (0=black,1=transparent)", cxxopts::value<int>()->default_value("0"))
        ("x,ispreloader", "Run CLI in preloading mode (used by GUI Only)", cxxopts::value<int>()->default_value("0"))
        ("B,bins", "Number of bins for histogram data (used for preloading)", cxxopts::value<int>()->default_value("1000"))
        ("Z,deepzoom", "Generate deepzoom (used for big images on the GUI)", cxxopts::value<int>()->default_value("0"))
        ("P,progress", "Print image operation progress ", cxxopts::value<int>()->default_value("0"))
        ("X,fastmode", "Faster processing (reducing quality) ", cxxopts::value<int>()->default_value("0"))
        ("m,memory", "Max memory usage", cxxopts::value<long>()->default_value("2048"));
        if (argc < 5)
        {
            std::cout << options.help({""}) << std::endl;
            return 0;
        }
        auto result = options.parse(argc, argv);
        filepath = result["infile"].as<std::string>();
        outfilepath = result["outfile"].as<std::string>();
        stretch = result["stretch"].as<std::string>();
        backgroundLevel = result["backgroundlevel"].as<float>(); 
        peakLevel = result["peaklevel"].as<float>();             
        scaledPeakLevel = result["scaledpeaklevel"].as<float>(); 
        blackLevel = result["blacklevel"].as<float>();           

        whiteLevel = result["whitelevel"].as<float>();           
        exponent = result["exponent"].as<float>();               
        depth = result["depth"].as<int>();
        ix = result["image"].as<int>();
        px = result["plane"].as<int>();
        flip = result["flip"].as<int>();
        outformat = result["outformat"].as<int>();
        quiet = result["quiet"].as<int>();
        autolevels = result["autolevels"].as<int>();
        undefined = result["undefined"].as<int>();
        ispreloader = result["ispreloader"].as<int>();
        bins=result["bins"].as<int>();
        dz=result["deepzoom"].as<int>();
        showprogress=result["progress"].as<int>();
        fastmode=result["fastmode"].as<int>();
        max_mem=result["memory"].as<long>();
        
        if (result.count("w")>0)
            is_wdefined=1;
        if (result.count("b")>0)
            is_bdefined=1;
        if (result.count("p")>0)
            is_pdefined=1;
        if (result.count("k")>0)
            is_bgdefined=1;
        if (result.count("p")>0)
            is_pkdefined=1;
        
    }
    catch (const cxxopts::OptionException &e)
    {
        std::cout << "error parsing options: " << options.help({""}) << std::endl;
        exit(1);
    }
    if (VIPS_INIT(argv[0]))
        vips_error_exit(NULL);

    //  Check if the file is compressed with fpack by getting the .fz
    string stringpath=filepath.substr(filepath.length()-3);
    for(int i = 0; i < stringpath.size(); i++)
    {
        stringpath[i] = tolower(stringpath[i]);
    }
    // If the file is compressed, use funpack for decompress original data
    if(stringpath==".fz")
    {
        string path = argv[0];
        iscompressed=1;
        char sep = '/';
        #ifdef _WIN32
        sep = '\\';
        #endif   
        string basename = getpathname(path);
        string newfilepath=outfilepath+".fits";
        remove((char *)&newfilepath[0]);
        string cmdunpack="\""+basename+sep+"funpack\" -O \""+newfilepath+"\" \""+filepath+"\"";
        report_progress(showprogress,"F-unpacking");
        std::system((char*)&cmdunpack[0]);
        filepath=newfilepath;
    }  
	
    fitsfile *fptr;     
    if (fits_open_diskfile(&fptr, (char *)&filepath[0], READONLY, &status))
    {
        cout << "Error loading FITS File " <<endl;
        exit(1);
    }
    Fits fits;
    report_progress(showprogress,"Extracting");
    npix=fits.getImage(fptr,ix,px,img,undef,&status,&nullvalues,&sizeofpixel,outfilepath,max_mem);
    
    if  (npix<0)
    {
        std::cout << "Fits Image processing error: " <<npix<<", error code "<<status<< endl;
        exit(1);
    }
    if (showprogress==1)
            vips_progress_set(true);
    original_width=img.width();    
    original_height=img.height();
    if (fastmode==1)
    {
        long max=(img.width()>img.height()?img.width():img.height());

        if (max>2560)
        {
        double factor=(double)2560/max; 
        img=img.resize(factor,VImage::option ()->set("kernel",VIPS_KERNEL_NEAREST));
        if (nullvalues>0)
            undef=undef.resize(factor   ,VImage::option ()->set("kernel",VIPS_KERNEL_NEAREST));
        }
    }
    VImage out;
    VImage preout = img;
    report_progress(showprogress,"Getting in stats");
    VImage statsin=img.stats();
    if (is_bgdefined==0)
        backgroundLevel = statsin(0, 0)[0];
    if (is_pkdefined==0)
        peakLevel = statsin(1, 0)[0];
  
    // this is the initial Linear Scaling
    out=img;
    report_progress(showprogress,"Scaling");

    if (stretch=="asinh")
    {

           preout=VipsOperations::asinh(img,backgroundLevel,peakLevel,scaledPeakLevel); 
           out=VipsOperations::asinh(img,backgroundLevel,peakLevel,scaledPeakLevel); 
    }
    if (stretch=="pow")
    {
            preout=VipsOperations::pow(img,backgroundLevel,peakLevel,exponent);
            out=VipsOperations::pow(img,backgroundLevel,peakLevel,exponent);
    }
    if (stretch=="log")
    {
            preout=VipsOperations::log(img,backgroundLevel,peakLevel,scaledPeakLevel);
            out=VipsOperations::log(img,backgroundLevel,peakLevel,scaledPeakLevel);
    }
    if (stretch=="linear" || stretch=="none")
    {
            preout=VipsOperations::linear(img,backgroundLevel,peakLevel,scaledPeakLevel);
            out=VipsOperations::linear(img,backgroundLevel,peakLevel,scaledPeakLevel);
    }

    VImage statsout;
    report_progress(showprogress,"Getting out stats");

    statsout=preout.stats();
    
    if (is_bdefined==0)
    {    
        double min = statsout(0, 0)[0];
        blackLevel=min;
    }
    
    if (is_wdefined==0)
    {    
        double max = statsout(1, 0)[0];
        whiteLevel=max;
    }

    if (autolevels==1)
    {
        double min = statsout(0, 0)[0];
        double max = statsout(1, 0)[0];
        double mean = statsout(4, 0)[0];
        double stdev = statsout(5, 0)[0];
        if (nullvalues>0)
            VipsOperations::correct_stats(nullvalues,npix,mean,stdev);
        
        blackLevel=mean-stdev;
        if (blackLevel<min)
            blackLevel=min;    
        whiteLevel=mean+stdev;
        if (whiteLevel>max)
            whiteLevel=max;    
        
    }     
        // On preloading we generate only png, and stats files
    if (ispreloader==1)
    {
        int numimages,numplanes;
        string header;
        fits.getNumOfImagesPlanes(fptr,&numimages,&numplanes,&status,header);
        string histogramdata=histogram(preout,bins);
        saveheader(header,outfilepath);
        int has_undefined=0;
        if (nullvalues>0)
          has_undefined=1;
        if (stretch=="none")
            statsout=img.stats();
        else    
            statsout=out.stats();
        double min = statsout(0, 0)[0];
        double max = statsout(1, 0)[0];
        double mean = statsout(4, 0)[0];
        double stdev = statsout(5, 0)[0];
        
        if (nullvalues>0)
        {
            VipsOperations::correct_stats(nullvalues,npix,mean,stdev);
        
        }    
        string histfile =  outfilepath + ".json";
        ofstream outhist(histfile);
        outhist << "{"<<'\"'<<"num_images"<<'\"'<<":" << numimages << ",\n";
        outhist << '\"'<<"num_planes"<<'\"'<<":" << numplanes << ",\n";
        outhist << '\"'<<"image"<<'\"'<<":" << ix << ",\n";
        outhist << '\"'<<"plane"<<'\"'<<":" << px << ",\n";
        outhist << '\"'<<"size"<<'\"'<<": {\n";
        outhist << '\"'<<"width"<<'\"'<<":" << preout.width() << ",\n";
        outhist << '\"'<<"height"<<'\"'<<":" << preout.height() << ",\n";
        outhist << '\"'<<"original_width"<<'\"'<<":" << original_width << ",\n";
        outhist << '\"'<<"original_height"<<'\"'<<":" << original_height << "\n";
        outhist << "}" << ",\n";
        outhist << '\"'<<"markpixel"<<'\"'<<": {\n";
        outhist << '\"'<<"undefined"<<'\"'<<":" << has_undefined << ",\n";
        outhist << '\"'<<"depthbit"<<'\"'<<":" << sizeofpixel << "\n";
        outhist << "}" << ",\n";
        outhist << '\"'<<"statistics"<<'\"'<<": {\n";
        outhist << '\"'<<"min"<<'\"'<<":" << min << ",\n";
        outhist << '\"'<<"max"<<'\"'<<":" << max << ",\n";
        outhist << '\"'<<"mean"<<'\"'<<":" << mean << ",\n";
        outhist << '\"'<<"stdev"<<'\"'<<":" << stdev << "\n";
        outhist << "}" << ",\n";
        outhist << '\"'<<"histogram"<<'\"'<<": [\n" << histogramdata << "]}";
        
    }

    int maxvalue=0;
    VipsBandFormat vbf;

    report_progress(showprogress,"Preparing");
    switch (depth)
    {
     case 8:
            out = VipsOperations::linear(preout,blackLevel,whiteLevel,0xFF);
            out = out.cast(VIPS_FORMAT_UCHAR);
            vbf=VIPS_FORMAT_UCHAR;
            maxvalue=255;
            break;
     case 16:
            out = VipsOperations::linear(preout,blackLevel,whiteLevel,0xFFFF);
            out = out.cast(VIPS_FORMAT_USHORT);
            vbf=VIPS_FORMAT_USHORT;
            maxvalue=65535;
            break;
    case 32:
            out = VipsOperations::linear(preout,blackLevel,whiteLevel,1);
            out = out.cast(VIPS_FORMAT_FLOAT);
            vbf=VIPS_FORMAT_FLOAT;
            maxvalue=1;
            break;
    }
    if (flip==1)
    {
        out=out.flip(VIPS_DIRECTION_VERTICAL);
        if (nullvalues>0)
            undef=undef.flip(VIPS_DIRECTION_VERTICAL);
    }
 
    if (nullvalues>0)
    {
        report_progress(showprogress,"Adding undefined channel");

        // Add Transparent on Undefined Pixels    
        if (undefined==1)
        {
            VImage minus=undef.new_from_image(1);
            VImage undefforout=minus.subtract(undef);
                undefforout=undefforout.cast(VIPS_FORMAT_UCHAR);
            VImage topvalue=out.new_from_image(maxvalue);
            VImage transparent=undefforout.multiply(topvalue);
            out=out.bandjoin(transparent);
            out=out.cast(vbf);
        }   
        // or black
        else
        {
            VImage minus=undef.new_from_image(1);
            VImage undefforout=minus.subtract(undef);
            undefforout=undefforout.cast(VIPS_FORMAT_UCHAR);
            out=undefforout.multiply(out);
            out=out.cast(vbf);
        } 
    }
    
    double size = img.width() * img.height() * depth / 8;
    int bigtiff = 0;
    if (size > 4294967296)
        bigtiff = 1;
    report_progress(showprogress,"Final save");
    
    if (outformat==1)
        out.tiffsave((char *)&outfilepath[0], VImage::option()->set("bigtiff", bigtiff));
    else    
    {
        if (dz==0)
        {
            out.pngsave((char *)&outfilepath[0], VImage::option()->set("compression",0));
        }
        else
        {
            string suffix=".png[compression=0,filter=none]";
            out.dzsave((char *)&outfilepath[0],VImage::option ()->set ("suffix", (char *)&suffix[0])->set("region_shrink",VIPS_REGION_SHRINK_MODE));
        }
    }
    // Being verbose

    
    if (quiet==0)
    {
	    
        ConsoleTable table{"Input Parameters: Level & Settings",""};
        table.setPadding(2);
        table.setStyle(0);
        table += {"-b Black:       "+to_string(blackLevel),      "-w White:       "+to_string(whiteLevel) };
        table += {"-k Background:  "+to_string(backgroundLevel), "-p Peak:        "+to_string(peakLevel)};
        table += {"-e Exponent:    "+to_string(exponent),        "-S Scaled Peak: "+to_string(scaledPeakLevel)};
        table += {"-s Scaling Fn:  "+stretch,                    "-f Flip:        "+to_string(flip)};
        table += {"-I Image index: "+to_string(ix),              "-z Plane index: "+to_string(px)};
        cout << table <<endl;

        ConsoleTable tablestats{"Statistics","Min","Max","Mean","StDev"};
        tablestats.setPadding(2);
        tablestats.setStyle(0);
    
        int imgs,planes;
        string header;
        fits.getNumOfImagesPlanes(fptr,&imgs,&planes,&status,header);
        statsout=preout.stats();
        double min = statsin(0, 0)[0];
        double max = statsin(1, 0)[0];
        double mean = statsin(4, 0)[0];
        double stdev = statsin(5, 0)[0];
        double min2 = statsout(0, 0)[0];
        double max2 = statsout(1, 0)[0];
        double mean2 = statsout(4, 0)[0];
        double stdev2 = statsout(5, 0)[0];
        
        if (nullvalues>0)
        {
            VipsOperations::correct_stats(nullvalues,npix,mean,stdev);
            VipsOperations::correct_stats(nullvalues,npix,mean2,stdev2);
        }    
        tablestats += {"Input Image:",to_string(min),to_string(max),to_string(mean),to_string(stdev) };
        tablestats += {"Scaled Image:",to_string(min2),to_string(max2),to_string(mean2),to_string(stdev2) };
        cout << tablestats;

        ConsoleTable tableinfo{"Image Width","Image Height","# of Images","# of Planes"};
        tableinfo.setPadding(2);
        tableinfo.setStyle(0);
    
        tableinfo +={to_string(img.width()),to_string(img.height()),to_string(imgs),to_string(planes)};
        cout << tableinfo;
    }

    vips_shutdown();
    if (iscompressed==1)
    {
        remove((char *)&filepath[0]);
    }
    string outpathimage=outfilepath+".vips";
    string outpathalfa=outfilepath+"a.vips";
    remove((char *)&outpathimage[0]);
    remove((char *)&outpathalfa[0]);
    
    return 0;
}
void saveheader(string header,string filename)
{
        string headerfile = filename + ".hdr";
        ofstream outheader(headerfile);
        outheader << header << endl;
}
string histogram(VImage img,int bins)
{
        VImage stats = img.stats();
        double min = stats(0, 0)[0];
        double max = stats(1, 0)[0];
        double mean = stats(4, 0)[0];
        double stdev = stats(5, 0)[0];
        VImage png = VipsOperations::linear(img, 0xFFFF);
        VImage imgforhist=png;
        png = png.cast(VIPS_FORMAT_USHORT); 
        //calculate histogram
        VImage hist=imgforhist.hist_find();
        string histogramdata = "";
        string h2="";
        void *v;
        size_t size;
        v=hist.write_to_memory(&size);
        uint32_t arr[0xFFFF];
        memcpy(arr,v,size);
        long sum=0;
        
        long *histvalues=new long[bins+1];
        for (int i = 0; i < bins; i++)
        {
             histvalues[i]=0;
        }
        long slide=0xFFFF/bins;
        for (unsigned int i = 0; i < 0xFFFF; i++)        
        {
            int bin=i/slide;
            if (bin>bins)
                bin=bins;
            histvalues[bin]+=arr[i];
        }
        
        for (int i = 0; i < bins; i++)
        {
            long realvalue;
            if (histvalues[i]==0)
              realvalue=0;
            else
              realvalue=log10(histvalues[i])*1e4;    
            histogramdata += to_string((long)realvalue) + ",\n";
        }
        histogramdata += to_string((long)hist(bins-1, 0)[0]) + "\n";
        delete[] histvalues;
        return histogramdata;
}
