

# FITS Liberator / CLI



## Requirements

You will need to install  cfitsio, libvips and libtiff

```
sudo apt install libcfitsio-dev libtiff-dev libvips-dev
```

## Building

```
make
```



## Usage

If you need processing FZ Compressed Fits files you should put the funpack utility on the same directory as the CLI.

```
 - Fits Liberator CLI
Usage:
  fitscli [OPTION...]

  -i, --infile arg           FITS Input File
  -o, --outfile arg          TIFF Output File
  -s, --stretch arg          Stretch math function (asinh,linear,pow,log)
                             (default: linear)
  -e, --exponent arg         Power Function exponent (default: 0.5)
  -k, --backgroundlevel arg  Background Level (default: 0)
  -p, --peaklevel arg        Peak Level (default: 1)
  -S, --scaledpeaklevel arg  Scaled Peak Level (default: 100)
  -b, --blacklevel arg       Black Level (default: 0)
  -w, --whitelevel arg       White Level (default: 1)
  -d, --depth arg            Bit depth (8, 16 or 32) (default: 16)
  -I, --image arg            Image index for multi-image files (default: 1)
  -z, --plane arg            Plane index for multi-plane images (default: 1)
  -f, --flip arg             Flip the image vertically (default: 1)
  -F, --outformat arg        Output format 1=tiff, 2=png (default: 1)
  -q, --quiet arg            Quiet execution without echo (default: 0)
  -a, --autolevels arg       Autoselect white & black levels using statistics
                             (default: 0)
  -u, --undefined arg        Alpha channel for undefined values
                             (0=black,1=transparent) (default: 0)
  -x, --ispreloader arg      Run CLI in preloading mode (used by GUI Only)
                             (default: 0)
  -B, --bins arg             Number of bins for histogram data (used for
                             preloading) (default: 1000)
  -Z, --deepzoom arg         Generate deepzoom (used for big images on the
                             GUI) (default: 0)
  -P, --progress arg         Print image operation progress  (default: 0)
  -X, --fastmode arg         Faster processing (reducing quality)  (default:
                             0)
  -m, --memory arg           Max memory usage (default: 2048)

```
