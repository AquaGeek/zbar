//------------------------------------------------------------------------
//  Copyright 2007-2008 (c) Jeff Brown <spadix@users.sourceforge.net>
//
//  This file is part of the Zebra Barcode Library.
//
//  The Zebra Barcode Library is free software; you can redistribute it
//  and/or modify it under the terms of the GNU Lesser Public License as
//  published by the Free Software Foundation; either version 2.1 of
//  the License, or (at your option) any later version.
//
//  The Zebra Barcode Library is distributed in the hope that it will be
//  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
//  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser Public License for more details.
//
//  You should have received a copy of the GNU Lesser Public License
//  along with the Zebra Barcode Library; if not, write to the Free
//  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
//  Boston, MA  02110-1301  USA
//
//  http://sourceforge.net/projects/zebra
//------------------------------------------------------------------------

#include <Magick++.h>

/* wand/wand-config.h (or magick/deprecate.h?)
 * defines these conflicting values :|
 */
#undef PACKAGE
#undef VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <config.h>
#include <iostream>
#include <sstream>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif
#include <assert.h>
#include <string>
#include <list>

#include <zebra.h>

using namespace std;
using namespace zebra;

const char *note_usage =
    "usage: zebraimg [options] <image>...\n"
    "\n"
    "scan and decode bar codes from one or more image files\n"
    "\n"
    "options:\n"
    "    -h, --help      display this help text\n"
    "    --version       display version information and exit\n"
    "    -q, --quiet     minimal output, only print decoded symbol data\n"
    "    -v, --verbose   increase debug output level\n"
    "    --verbose=N     set specific debug output level\n"
    "    -d, --display   enable display of following images to the screen\n"
    "    -D, --nodisplay disable display of following images (default)\n"
    // FIXME overlay level
    // FIXME xml output
    ;

const char *warning_not_found =
    "WARNING: barcode data was not detected in some image(s)\n"
    "  things to check:\n"
    "    - is the barcode type supported?  currently supported\n"
    "      symbologies are: EAN-13/UPC-A and Code 128\n"
    "    - is the barcode large enough in the image?\n"
    "    - is the barcode mostly in focus?\n"
    "    - is there sufficient contrast/illumination?\n";

int notfound = 0;
int num_images = 0, num_symbols = 0;

Processor *processor = NULL;

static void scan_image (const std::string& filename)
{
    Magick::Image image;
    image.read(filename);
    image.modifyImage();

    // extract grayscale image pixels
    // FIXME color!! ...preserve most color w/422P
    // (but only if it's a color image)
    Magick::Blob scan_data;
    image.write(&scan_data, "GRAY", 8);
    unsigned width = image.columns();
    unsigned height = image.rows();
    assert(scan_data.length() == width * height);

    Image zimage(width, height, "Y800",
                 scan_data.data(), scan_data.length());
    processor->process_image(zimage);

    // output result data
    bool found = false;
    for(Image::SymbolIterator sym = zimage.symbol_begin();
        sym != zimage.symbol_end();
        ++sym)
    {
        cout << *sym << endl;
        found = true;
        num_symbols++;
    }
    cout.flush();
    if(!found)
        notfound++;
    num_images++;
    if(processor->is_visible())
        processor->user_wait();
}

int usage (int rc, const string& msg = "")
{
    ostream &out = (rc) ? cerr : cout;
    if(msg.length())
        out << msg << endl << endl;
    out << note_usage << endl;
    return(rc);
}

int main (int argc, const char *argv[])
{
    // option pre-scan
    bool quiet = false;
    int i;
    for(i = 1; i < argc; i++) {
        string arg(argv[i]);
        if(arg[0] != '-')
            // first pass, skip images
            continue;
        else if(arg[1] != '-') {
            for(int j = 1; arg[j]; j++)
                switch(arg[j]) {
                case 'h': return(usage(0));
                case 'q': quiet = true; break;
                case 'v': zebra_increase_verbosity(); break;
                case 'd':
                case 'D': break;
                default:
                    return(usage(1, string("ERROR: unknown bundled option: -") +
                                 arg[j]));
                }
        }
        else if(arg == "--help")
            return(usage(0));
        else if(arg == "--version") {
            cout << PACKAGE_VERSION << endl;
            return(0);
        }
        else if(arg == "--quiet") {
            quiet = true;
            argv[i] = NULL;
        }
        else if(arg == "--verbose")
            zebra_increase_verbosity();
        else if(arg.substr(0, 10) == "--verbose=") {
            istringstream scan(arg.substr(10));
            int level;
            scan >> level;
            zebra_set_verbosity(level);
        }
        else if(arg == "--display" ||
                arg == "--nodisplay")
            continue;
        else if(arg == "--")
            break;
        else
            return(usage(1, "ERROR: unknown option: " + arg));
    }

    /* process and display images (no video) unthreaded */
    processor = new Processor(false, NULL);

    try {
        for(i = 1; i < argc; i++) {
            if(!argv[i])
                continue;
            string arg(argv[i]);
            if(arg[0] != '-')
                scan_image(arg);
            else if(arg[1] != '-')
                for(int j = 1; arg[j]; j++)
                    switch(arg[j]) {
                    case 'd': processor->set_visible(true); break;
                    case 'D': processor->set_visible(false);
                        break;
                    }
            else if(arg == "--display")
                processor->set_visible(true);
            else if(arg == "--nodisplay")
                processor->set_visible(false);
            else if(arg == "--")
                break;
        }
        for(i++; i < argc; i++)
            scan_image(argv[i]);
    }
    catch(Magick::Exception &e) {
        cerr << "ERROR: " << e.what() << endl;
        return(1);
    }

    if(!num_images)
        return(usage(1, "ERROR: specify image file(s) to scan"));

    else if(!quiet) {
        cerr << "scanned " << num_symbols << " barcode symbols from "
             << num_images << " images";
#ifdef HAVE_SYS_TIMES_H
#ifdef HAVE_UNISTD_H
        long clk_tck = sysconf(_SC_CLK_TCK);
        struct tms tms;
        if(clk_tck > 0 && times(&tms) >= 0) {
            double secs = tms.tms_utime + tms.tms_stime;
            secs /= clk_tck;
            cerr << " in " << secs << " seconds";
        }
#endif
#endif
        cerr << endl;
        if(notfound)
            cerr << endl << warning_not_found << endl;
    }
    return(0);
}
