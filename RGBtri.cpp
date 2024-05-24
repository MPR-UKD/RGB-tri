// RGBtri.cpp


#include "RGBtri.h"


using namespace std;

// Function to display usage information
void displayUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options]\n\n"
        << "Options:\n"
        << "  -h, --help         Show this help message and exit\n"
        << "  -v, --version      Show version information and exit\n"
        << "  -f                 Use scaling factor\n"
        << "  -Ff   FILE         Input file Ff\n"
        << "  -Fi   FILE         Input file Fi\n"
        << "  -o    FILE         Output file (default = \"F_rgb.nii\")\n\n"
        << "Example:    RGBtri -Ff file1.nii -Fi file2.nii [-o output-file.nii]\n"
        << "            (with file1.nii = F-fast, file2.nii = F-intermediate)\n";
}


// Function to display version information
void displayVersion() {
    std::cout << "RGBtri version 1.0" << endl;
}


// Function for histogram equalization 
// use of a center factor as images often should be equalized due to the image center only...
void equalizeHistogram(int* pdata, int width, int height, int noslice, int max_val, float scalefact, float fCenter)
{
    int total = width * height * noslice;
    int n_bins = max_val + 1;
    int hist[256];
    int lut[256];
    for (int i = 0; i < 256; i++) {
        hist[i] = 0;
        lut[i] = 0;
    }
    //
    // build histogram for center and middle slice only - set outside to zero
    //
    int center = width / fCenter;	// for size of image center (see below) prostat=4 kidney=6
    // alloc 3D array
    int*** pdata2 = new int** [noslice];
    for (int i = 0; i < noslice; i++) {
        pdata2[i] = new int* [width];
        for (int j = 0; j < width; j++) {
            pdata2[i][j] = new int[height];
        }
    }
    // fill array
    int midslice = noslice / 2;
    if (noslice == 1)
        midslice = 1;
    for (int i = 0; i < noslice; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < height; k++) {
                if (j > center && j<width - center && k>center && k < height - center
                    && (i == midslice - 2 || i == midslice - 1 || i == midslice)) {
                    pdata2[i][j][k] = pdata[k + j * height + i * height * width];
                }
                else {
                    pdata2[i][j][k] = 0;
                }
            }
        }
    }
    // Compute histogram
    for (int i = 0; i < noslice; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < height; k++) {
                hist[pdata2[i][j][k]]++;
            }
        }
    }
    // clean up
    for (int i = 0; i < noslice; i++) {
        for (int j = 0; j < width; j++) {
            delete[] pdata2[i][j];
        }
        delete[] pdata2[i];
    }
    delete[] pdata2;
    // build histogram for center only - set outside to zero - END
    //
    //
    // Compute histogram   -   over the whole image
    /*vector<int> hist(n_bins, 0);
    for (int i = 0; i < total; ++i) {	// here maybe only over the middle of the image?
    	hist[pdata[i]]++;
    }*/
    //
    // 
    // Build LUT from cumulative histrogram
    // Find first non-zero bin
    //
    int i = 0;
    while (!hist[i]) ++i;
    if (hist[i] == total) {	// if all pix = 0
        for (int j = 0; j < total; ++j) {
            pdata[j] = i;
        }
        return;
    }
    //
    // Compute scale
    float scale = (n_bins - 1.f) / (total - hist[i]);
    i++;
    int sum = 0;
    for (; i < n_bins; ++i) {
        sum += hist[i];
        lut[i] = std::max(0, std::min(static_cast<int>(std::round(sum * scale * scalefact)), max_val));
    }
    // Apply equalization
    for (int j = 0; j < total; ++j) {
        pdata[j] = lut[pdata[j]];
    }
    //
}

// Function for white balancing
void WhiteBalanceBand(int* bandArray, int length) {
    // Copy the array
    double* sortedBandArray = new double[length]();
    for (int i = 0; i < length; i++) {
        sortedBandArray[i] = bandArray[i];
    }
    // Sort the array
    std::sort(sortedBandArray, sortedBandArray + length);
    // Percent (configurable) for balance
    double percentForBalance = 0.6;
    // Calculate the percentile
    double perc05 = Percentile(sortedBandArray, length, percentForBalance);
    double perc95 = Percentile(sortedBandArray, length, 100 - percentForBalance);
    // create balanced array
    double* bandBalancedArray = new double[length];
    for (int i = 0; i < length; i++) {
        // Calculate balanced value
        double valueBalanced = (bandArray[i] - perc05) * 255.0 / (perc95 - perc05);
        // Limit values between 0 and 255
        bandBalancedArray[i] = LimitToByte(valueBalanced);
    }
    for (int i = 0; i < length; i++) {
        bandArray[i] = (int)bandBalancedArray[i];
    }
    // clean up
    delete[] sortedBandArray;
    delete[] bandBalancedArray;
}


// helper function for WhiteBalanceBand()
double Percentile(double array[], int length, double percentile) {
    int nArray = length;
    double nPercent = (nArray + 1) * percentile / 100;
    if (nPercent == 1) {
        return array[0];
    }
    else if (nPercent == nArray) {
        return array[nArray - 1];
    }
    else {
        int intNPercent = (int)nPercent;
        double d = nPercent - intNPercent;
        return array[intNPercent - 1] + d * (array[intNPercent] - array[intNPercent - 1]);
    }
}


// helper function for WhiteBalanceBand()
double LimitToByte(double value) {
    if (value < 0) {
        return 0;
    }
    else if (value > 255) {
        return 255;
    }
    return value;
}



int main(int argc, char* argv[])
{
    bFact = true;
    // Store the program name
    std::string programName = argv[0];
    // Vector to hold non-option arguments
    std::vector<std::string> args;
    // String to hold input file names
    std::string FfFileName;
    std::string FiFileName;
    // String to hold the output file name
    std::string outputFileName = "F_rgb.nii";;
    // Parse the command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            displayUsage(programName);
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            displayVersion();
            return 0;
        }
        else if (arg == "-f") {
            bFact = false;
        }
        else if (arg == "-Ff") {
            if (i + 1 < argc) {
                FfFileName = argv[++i];
            }
            else {
                std::cerr << "Command line error - abort...\n";
                return 1;
            }
        }
        else if (arg == "-Fi") {
            if (i + 1 < argc) {
                FiFileName = argv[++i];
            }
            else {
                std::cerr << "Command line error - abort...\n";
                return 1;
            }
        }
        else if (arg == "-o") {
            if (i + 1 < argc) {
                outputFileName = argv[++i];
            }
            else {
                outputFileName = "F_rgb.nii";
            }
        }
        else {
            // Collect non-option arguments
            args.push_back(arg);
        }
    }
    //
    // conversion factor to get data range from 0 to 1.0
    // 
    double dFact;
    if (bFact) {
        dFact = 100.0;
    }
    else {
        dFact = 1.0;
    }
    // 
    // read Ff Nifti
    //
    nifti_image* nFf;
    nFf = nifti_image_read(FfFileName.c_str(), 1);
    if (nFf == nullptr) {
        printf("Can't open %s.  Abort...\n", FfFileName.c_str());
        return 1;
    }
    // check for datatype: only 32bit float or 16bit int are allowed currently
    int datatype = nFf->datatype;
    if (datatype != 16 && datatype != 4) {
        printf("Please use 32bit float or 16bit int data only!  Abort...\n");
        return 1;
    }
    // read dimensions
    int nx = nFf->nx;
    int ny = nFf->ny;
    int nz = nFf->nz;
    // pointers to read data
    float* fpxFf;
    short* spxFf;
    // get address of data
    if (datatype == 16) {   // case of float 32bit
        fpxFf = static_cast<float*>(nFf->data);
    }
    else if (datatype == 4) {   // case of int 16bit
        spxFf = static_cast<short*>(nFf->data);
    }
    // create double array pixFf[] to hold pixels
    double* pixFf = new double[nx*ny*nz] {0};
    // read data to array
    for(int i=0;i<nx*ny*nz;i++){
        if (datatype == 16) {
            pixFf[i] = (double)fpxFf[i] / dFact;
        }
        else if (datatype == 4) {
            pixFf[i] = (double)spxFf[i] / dFact;
        }
    }
    // pixFf[] = Ff pixel values
    // 
    // 
    // read Fi Nifti
    //
    nifti_image* nFi;
    nFi = nifti_image_read(FiFileName.c_str(), 1);
    if (nFi == nullptr) {
        printf("Can't open %s.  Abort...\n", FiFileName.c_str());
        return 1;
    }
    // check for datatype: only 32bit float or 16bit int are allowed currently
    datatype = nFi->datatype;
    if (datatype != 16 && datatype != 4) {
        printf("Please use 32bit float or 16bit int data only!  Abort...\n");
        return 1;
    }
    // check if dimensions are the same as in Ff
    if (nFi->nx != nx || nFi->ny != ny || nFi->nz != nz) {
        printf("Ff and Fi have different dimensions.  Abort...\n");
        return 1;
    }
    // pointers to read data
    float* fpxFi;
    short* spxFi;
    // get address of data
    if (datatype == 16) {   // case of float 32bit
        fpxFi = static_cast<float*>(nFi->data);
    }
    else if (datatype == 4) {   // case of int 16bit
        spxFi = static_cast<short*>(nFi->data);
    }
    // create double array pixFi[] to hold pixels
    double* pixFi = new double[nx * ny * nz] {0};
    // read data to array
    for (int i = 0; i < nx * ny * nz; i++) {
        if (datatype == 16) {
            pixFi[i] = (double)fpxFi[i] / dFact;
        }
        else if (datatype == 4) {
            pixFi[i] = (double)spxFi[i] / dFact;
        }
    }
    // pixFi[] = Fi pixel values
    // 
    // 
    // create Fs array and fill with data: Fs = 1 - Fi - Ff
    //
    double* pixFs = new double[nx * ny * nz] {0};
    for (int i = 0; i < nx * ny * nz; i++) {
        pixFs[i] = 1.0 - pixFi[i] - pixFf[i];
    }
    // pixFs[] = Fs pixel values
    //
    //
    //  start processing of data here...
    //
    //
    // create int arrays for each channel i.e. Ff, Fi, Fs
    // 
    int* bFf = new int[nx*ny*nz];
    int* bFi = new int[nx*ny*nz];
    int* bFs = new int[nx*ny*nz];
    // feed in fraction values and normalize to [0..255]
    for (int i = 0; i < nx*ny*nz; i++) {
        bFf[i] = (int)(pixFf[i] * 255.0);
        bFi[i] = (int)(pixFi[i] * 255.0);
        if (bFf[i] == 0 && bFi[i] == 0) {
            bFs[i] = 0;
        }
        else {
            bFs[i] = (int)((1.0 - pixFi[i] - pixFf[i]) * 255.0);
        }
        if (bFs[i] <= 0) bFs[i] = 0; // don't let Fd be negative
        // don't let fractions be > 255
        if (bFf[i] > 255) bFf[i] = 255;
        if (bFi[i] > 255) bFi[i] = 255;
        if (bFs[i] > 255) bFs[i] = 255;
    }
    // 
    // now we have all original values scaled to 0 - 255
    // 
    // 
    // do histogram euqalization
    //
    // by fCenter value only the middle of the image is used for equalization
    // 
    // fCenter = 4.0 => the outer 1/4 of the image is NOT used for equalization
    // changes of fCenter should be done to adapt equalization!
    //
    float fCenter = 4.0;    // 
    equalizeHistogram(bFf, ny, ny, nz, 255, 1.0, fCenter);	//blue
    equalizeHistogram(bFi, ny, ny, nz, 255, 0.95, fCenter);	//green 0.95
    equalizeHistogram(bFs, ny, ny, nz, 255, 1.0, fCenter);  //red
    //
    //
    // do gamma correction
    //
    //
    float gamma_b = 2.2;	//gamma for blue channel	2.2
    float gamma_g = 2.0;	//gamma for green channel	2.0
    float gamma_r = 2.4;	//gamma for red channel		1.1	kidney 3.0 prostate
    for (int i = 0; i < nx*ny*nz; i++) {
        // gamma
        float fFp = static_cast<float>(bFf[i]) / 255.0;
        float fFi = static_cast<float>(bFi[i]) / 255.0;
        float fFd = static_cast<float>(bFs[i]) / 255.0;
        float dbFp = pow(fFp, gamma_b) * 255.0;
        float dbFi = pow(fFi, gamma_g) * 255.0;
        float dbFd = pow(fFd, gamma_r) * 255.0;
        bFf[i] = static_cast<unsigned char>(std::round(dbFp));
        bFi[i] = static_cast<unsigned char>(std::round(dbFi));
        bFs[i] = static_cast<unsigned char>(std::round(dbFd));
    }
    //
    //
    // do white balance
    //
    //
    WhiteBalanceBand(bFf, nx*ny*nz);
    WhiteBalanceBand(bFi, nx*ny*nz);
    WhiteBalanceBand(bFs, nx*ny*nz);
    // now bFp, bFi, bFd are the corrected color values
    //
    //
    // create a byte array for output of RGB values as RGB Nifti (24bit, datatype = 128)
    std::byte* pBits = new std::byte[nx*ny*nz*3];
    //
    //
    // fill corrected values of Ff, Fi, Fs to byte array
    //
    int c = 0;  // just a counter variable

    for (int i = 0; i < nz; i++) {
        //for (int j = ny-1; j >= 0; j--) {       // invers for correct image orientation?
        for (int j = 0; j < ny; j++) {
            for (int k = 0; k < nx; k++) {
                byte ff = (byte)bFf[k+j*nx+i*nx*nx];
                byte fi = (byte)bFi[k+j*nx+i*nx*nx];
                byte fs = (byte)bFs[k+j*nx+i*nx*nx];
                if ((int)ff > 255) fs = (byte)255;
                if ((int)fi > 255) fi = (byte)255;
                if ((int)fs > 255) ff = (byte)255;
                pBits[c + 0] = fs;    // blue
                pBits[c + 1] = fi;    // green
                pBits[c + 2] = ff;    // red
                c += 3;
            }
        }
    }

    //
    //
    // create an output Nifti with datatype = 128 
    // 
    //
    nifti_image* nimout;
    nimout = nifti_simple_init_nim();
    nimout->nx = nx;
    nimout->ny = ny;
    nimout->nz = nz;
    nimout->nt = 1;
    nimout->ndim = 3;
    nimout->dim[0] = 3;
    nimout->dim[1] = nx;
    nimout->dim[2] = ny;
    nimout->dim[3] = nz;
    nimout->dim[4] = 1;
    nimout->dim[5] = 1;
    nimout->dim[6] = 1;
    nimout->dim[7] = 1;
    nimout->nvox = nx * ny * nz;
    nimout->datatype = 128; // DT_* code => 2=UINT8, 4=INT16, 8=INT32, 16=FLOAT32, 128=RGB24
    nimout->nbyper = 3;	// RGB24
    nimout->dx = nFf->dx;
    nimout->dy = nFf->dy;
    nimout->dz = nFf->dz;
    nimout->dt = 0.0;   // nim->dt;
    nimout->du = 1.0;
    nimout->dv = 1.0;
    nimout->dw = 1.0;
    nimout->pixdim[1] = nFf->dx;
    nimout->pixdim[2] = nFf->dy;
    nimout->pixdim[3] = nFf->dz;
    nimout->pixdim[4] = 0.0;    // nim->dt;
    nimout->pixdim[5] = 1.0;
    nimout->pixdim[6] = 1.0;
    nimout->pixdim[7] = 1.0;
    nimout->scl_slope = 1.0;
    nimout->nifti_type = 1;
    nimout->data = (void*)pBits;
    nimout->fname = (char*)outputFileName.c_str();
    nifti_image_write(nimout);
    //
    //
    // display user information
    //
    //
    cout << "\nWrote RGB Nifti image: " << outputFileName << endl;
    cout << "Thank you for using RGBtri.  Byebye...\n" << endl;
    //
    //
    // clean up
    //
    //
    delete[] pBits;
    delete[] bFf;
    delete[] bFi;
    delete[] bFs;
    delete[] pixFf;
    delete[] pixFi;
    delete[] pixFs;
	return 0;
}
