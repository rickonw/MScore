/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Author:      Rodolfo Zitellini
// Created:     26/06/2012
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>

#ifndef _WIN32
#include <getopt.h>
#else
#include "win_getopt.h"
#endif

#include "options.h"
#include "toolkit.h"
#include "vrv.h"

#if (defined _WIN32) || (defined __CYGWIN__)
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#define msleep(s) Sleep((s))
#include <io.h>
//#include "getopt_long.h"
#ifdef __WATCOMC__
#define _putch putch
#endif
#endif


#include "wildmidi_lib.h"
#include "wm_tty.h"
#include "filenames.h"


using namespace std;
using namespace vrv;

// Some redundant code to get basenames
// and remove extensions
// possible that it is not in std??
struct MatchPathSeparator {
    bool operator()(char ch) const { return ch == '\\' || ch == '/'; }
};

std::string basename(std::string const &pathname)
{
    return std::string(std::find_if(pathname.rbegin(), pathname.rend(), MatchPathSeparator()).base(), pathname.end());
}

std::string removeExtension(std::string const &filename)
{
    std::string::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
    return pivot == filename.rend() ? filename : std::string(filename.begin(), pivot.base() - 1);
}

std::string fromCamelCase(const std::string &s)
{
    std::regex regExp1("(.)([A-Z][a-z]+)");
    std::regex regExp2("([a-z0-9])([A-Z])");

    std::string result = s;
    result = std::regex_replace(result, regExp1, "$1-$2");
    result = std::regex_replace(result, regExp2, "$1-$2");

    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toCamelCase(const std::string &s)
{
    std::istringstream iss(s);
    std::string token;
    std::string result;

    while (getline(iss, token, '-')) {
        token[0] = toupper(token[0]);
        result += token;
    }

    result[0] = tolower(result[0]);

    return result;
}

bool dir_exists(std::string dir)
{
    struct stat st;
    if ((stat(dir.c_str(), &st) == 0) && (((st.st_mode) & S_IFMT) == S_IFDIR)) {
        return true;
    }
    else {
        return false;
    }
}


void display_version()
{
    //cerr << "Verovio " << GetVersion() << endl;
}

void display_usage()
{
    std::cout.precision(2);

    display_version();
    std::cout << std::endl << "Example usage:" << std::endl << std::endl;
    std::cout << " verovio [-s scale] [-t type] [-r resources] [-o outfile] infile" << std::endl << std::endl;

    // These need to be kept in alphabetical order:
    // -options with both short and long forms first
    // -then options with long forms only
    // -then debugging options

    vrv::Options style;

    // Options with both short and long forms
    std::cout << "Options (marked as * are repeatable)" << std::endl;

    std::cout << " -                     Use \"-\" as input file for reading from the standard input" << std::endl;
    std::cout << " -?, --help            Display this message" << std::endl;
    std::cout << " -a, --all-pages       Output all pages" << std::endl;
    std::cout << " -f, --format <s>      Select input format: darms, mei, pae, xml (default is mei)" << std::endl;
    std::cout << " -o, --outfile <s>     Output file name (use \"-\" for standard output)" << std::endl;
    std::cout << " -p, --page <i>        Select the page to engrave (default is 1)" << std::endl;
    std::cout << " -r, --resources <s>   Path to SVG resources (default is " << vrv::Resources::GetPath() << ")" << std::endl;
    std::cout << " -s, --scale <i>       Scale percent (default is " << DEFAULT_SCALE << ")" << std::endl;
    std::cout << " -t, --type <s>        Select output format: mei, svg, or midi (default is svg)" << std::endl;
    std::cout << " -v, --version         Display the version number" << std::endl;
    std::cout << " -x, --xml-id-seed <i> Seed the random number generator for XML IDs" << std::endl;

    vrv::Options options;
    std::vector<vrv::OptionGrp *> *grp = options.GetGrps();
    std::vector<vrv::OptionGrp *>::iterator grpIter;

    for (grpIter = grp->begin(); grpIter != grp->end(); ++grpIter) {

        // Options with long forms only
        std::cout << std::endl << (*grpIter)->GetLabel() << std::endl;
        const std::vector<vrv::Option *> *options = (*grpIter)->GetOptions();
        std::vector<vrv::Option *>::const_iterator iter;

        for (iter = options->begin(); iter != options->end(); ++iter) {

            std::string option = fromCamelCase((*iter)->GetKey());

            const vrv::OptionDbl *optDbl = dynamic_cast<const vrv::OptionDbl *>(*iter);
            const vrv::OptionInt *optInt = dynamic_cast<const vrv::OptionInt *>(*iter);
            const vrv::OptionIntMap *optIntMap = dynamic_cast<const vrv::OptionIntMap *>(*iter);
            const vrv::OptionString *optString = dynamic_cast<const vrv::OptionString *>(*iter);
            const vrv::OptionArray *optArray = dynamic_cast<const vrv::OptionArray *>(*iter);
            const vrv::OptionBool *optBool = dynamic_cast<const vrv::OptionBool *>(*iter);

            if (optDbl) {
                option.append(" <f>");
            }
            else if (optInt) {
                option.append(" <i>");
            }
            else if (optString) {
                option.append(" <s>");
            }
            else if (optArray) {
                option.append("* <s>");
            }
            else if (!optBool) {
                option.append(" <s>");
            }

            if (option.size() < 32) {
                option.insert(option.end(), 32 - option.size(), ' ');
            }
            else {
                option.append("\t");
            }

            std::cout << " --" << option << (*iter)->GetDescription();

            if (optInt) {
                std::cout << " (default: " << optInt->GetDefault();
                std::cout << "; min: " << optInt->GetMin();
                std::cout << "; max: " << optInt->GetMax() << ")";
            }
            if (optDbl) {
                std::cout << std::fixed << " (default: " << optDbl->GetDefault();
                std::cout << std::fixed << "; min: " << optDbl->GetMin();
                std::cout << std::fixed << "; max: " << optDbl->GetMax() << ")";
            }
            if (optString) {
                std::cout << " (default: \"" << optString->GetDefault() << "\")";
            }
            if (optIntMap) {
                std::cout << " (default: \"" << optIntMap->GetDefaultStrValue()
                     << "\"; other values: " << optIntMap->GetStrValuesAsStr(true) << ")";
            }
            std::cout << std::endl;
        }
    }
}


#ifdef __cplusplus
    extern "C"{
#endif


extern int wildmidi(const char *file, char *cfg, struct _WM_inPlayHook *wmhook);

#define DEF_RATE  32072

static struct option const long_options[] = {
    { "version", 0, 0, 'v' },
    { "help", 0, 0, 'h' },
    { "rate", 1, 0, 'r' },
    { "mastervol", 1, 0, 'm' },
    { "config", 1, 0, 'c' },
    { "wavout", 1, 0, 'o' },
    { "tomidi", 1, 0, 'x' },
    { "convert", 1, 0, 'g' },
    { "frequency", 1, 0, 'f' },
    { "log_vol", 0, 0, 'l' },
    { "reverb", 0, 0, 'b' },
    { "test_midi", 0, 0, 't' },
    { "test_bank", 1, 0, 'k' },
    { "test_patch", 1, 0, 'p' },
    { "enhanced", 0, 0, 'e' },
#if defined(AUDIODRV_OSS) || defined(AUDIODRV_ALSA)
    { "device", 1, 0, 'd' },
#endif
    { "roundtempo", 0, 0, 'n' },
    { "skipsilentstart", 0, 0, 's' },
    { "textaslyric", 0, 0, 'a' },
    { "playfrom", 1, 0, 'i'},
    { "playto", 1, 0, 'j'},
    { NULL, 0, NULL, 0 }
};

void get_playpara(void *midi_ptr, struct _WM_inPlayPara *inPlayPara)
{
    uint8_t ch = 0;
    uint16_t mixer_options = 0;
    uint8_t master_volume = 100;
    int8_t *output_buffer;
    char modes[5];
    int inpause = 0;
    int8_t kareoke = 0;
    struct _WM_Info *wm_info;
    unsigned long int seek_to_sample;
    uint32_t samples = 0;

#ifdef _WIN32
    if (_kbhit()) {
        ch = _getch();
        _putch(ch);
    }
#elif defined(__DJGPP__) || defined(__OS2__) || defined(__EMX__)
    if (kbhit()) {
        ch = getch();
        putch(ch);
    }
#elif defined(WILDMIDI_AMIGA)
    amiga_getch (&ch);
#else
    if (read(STDIN_FILENO, &ch, 1) != 1)
        ch = 0;
#endif
    if (ch) {
        wm_info = WildMidi_GetInfo(midi_ptr);
    
        switch (ch) {
        case 'l':
            WildMidi_SetOption(midi_ptr, WM_MO_LOG_VOLUME,
                               ((mixer_options & WM_MO_LOG_VOLUME) ^ WM_MO_LOG_VOLUME));
            mixer_options ^= WM_MO_LOG_VOLUME;
            modes[0] = (mixer_options & WM_MO_LOG_VOLUME)? 'l' : ' ';
            break;
        case 'r':
            WildMidi_SetOption(midi_ptr, WM_MO_REVERB,
                               ((mixer_options & WM_MO_REVERB) ^ WM_MO_REVERB));
            mixer_options ^= WM_MO_REVERB;
            modes[1] = (mixer_options & WM_MO_REVERB)? 'r' : ' ';
            break;
        case 'e':
            WildMidi_SetOption(midi_ptr, WM_MO_ENHANCED_RESAMPLING,
                               ((mixer_options & WM_MO_ENHANCED_RESAMPLING) ^ WM_MO_ENHANCED_RESAMPLING));
            mixer_options ^= WM_MO_ENHANCED_RESAMPLING;
            modes[2] = (mixer_options & WM_MO_ENHANCED_RESAMPLING)? 'e' : ' ';
            break;
        case 'a':
            WildMidi_SetOption(midi_ptr, WM_MO_TEXTASLYRIC,
                               ((mixer_options & WM_MO_TEXTASLYRIC) ^ WM_MO_TEXTASLYRIC));
            mixer_options ^= WM_MO_TEXTASLYRIC;
            break;
        case 'n':
            return;
        case 'p':
            inPlayPara->pause_set = true;
            if (inPlayPara->pause_state) {
                inPlayPara->pause_state = 0;
            } else {
                inPlayPara->pause_state = 1;
            }
            break;
        case 'q':
            printf("\r\n");
            if (inpause) return;
        case '-':
            if (master_volume > 0) {
                master_volume--;
                WildMidi_MasterVolume(master_volume);
            }
            break;
        case '+':
            if (master_volume < 127) {
                master_volume++;
                WildMidi_MasterVolume(master_volume);
            }
            break;
        case ',': /* fast seek backwards */

            if (wm_info->current_sample < DEF_RATE) {
                seek_to_sample = 0;
            } else {
                seek_to_sample = wm_info->current_sample - DEF_RATE;
            }
            WildMidi_FastSeek(midi_ptr, &seek_to_sample);
            break;
        case '.': /* fast seek forwards */
            if ((wm_info->approx_total_samples
                    - wm_info->current_sample) < DEF_RATE) {
                seek_to_sample = wm_info->approx_total_samples;
            } else {
                seek_to_sample = wm_info->current_sample + DEF_RATE;
            }
            WildMidi_FastSeek(midi_ptr, &seek_to_sample);
            break;
        case '<':
            WildMidi_SongSeek (midi_ptr, -1);
            break;
        case '>':
            WildMidi_SongSeek (midi_ptr, 1);
            break;
        case '/':
            WildMidi_SongSeek (midi_ptr, 0);
            break;
        case 'm': /* save as midi */ 

            break;
        case 'k': /* Kareoke */
                  /* Enables/Disables the display of lyrics */
            kareoke ^= 1;
            break;
        default:
            break;
        }
    }

}

uint32_t midiPlayer_lastMillisec = 0;
uint32_t midiPlayer_currentSamples = 0;
uint32_t midiPlayer_totalSamples = 0;
uint32_t midiPlayer_updateRate = 50;

#define SAMPLE_RATE 44100

void midiUpdate(void *handle, uint32_t current, uint32_t total, uint32_t total_midi_time) 
{
    int32_t time;
    Toolkit *vrvToolkit = (Toolkit *)handle;

    midiPlayer_currentSamples = current;
    midiPlayer_totalSamples = total;
    //midiPlayer_progress.style.width = (current / total * 100) + '%';
    //midiPlayer_playingTime.innerHTML = samplesToTime(current);
    //midiPlayer_totalTime.innerHTML = samplesToTime(total);

    //uint32_t millisec = Math.floor(current * 1000 / SAMPLE_RATE / midiPlayer_updateRate);
    uint32_t millisec = (current * 1000 / SAMPLE_RATE / midiPlayer_updateRate);
    if (midiPlayer_lastMillisec > millisec) {
        midiPlayer_lastMillisec = 0;
    }

    if (millisec > midiPlayer_lastMillisec)
    {
        time = (millisec * midiPlayer_updateRate);
    
        uint32_t vrvTime = (2 * time - 800 > 0) ? (2 * time - 800) : 0;
        string elementsattime = vrvToolkit->GetElementsAtTime(vrvTime);

/*
        if (elementsattime.page > 0) {
         if (elementsattime.page != page) {
             page = elementsattime.page;
             loadPage();
         }
         if ((elementsattime.notes.length > 0) && (ids != elementsattime.notes)) {
             ids.forEach(function(noteid) {
                 if ($.inArray(noteid, elementsattime.notes) == -1) {
                     $("#" + noteid).attr("fill", "#000").attr("stroke", "#000"); 
                 }
             });
             ids = elementsattime.notes;
             ids.forEach(function(noteid) {
                 if ($.inArray(noteid, elementsattime.notes) != -1) {
                     $("#" + noteid).attr("fill", "#c00").attr("stroke", "#c00");; 
                 }
             }); 
         }
       }
*/
   }

    midiPlayer_lastMillisec = millisec;

}


void create_svg(vrv::Toolkit &toolkit, int all_pages, int from, int to, std::string outfile)
{
    int p;
    for (p = from; p < to; ++p) {
        std::string cur_outfile = outfile;
        if (all_pages) {
            cur_outfile += vrv::StringFormat("_%03d", p);
        }
        cur_outfile += ".svg";
        if (!toolkit.RenderToSVGFile(cur_outfile, p)) {
            std::cerr << "Unable to write SVG to " << cur_outfile << "." << std::endl;
            exit(1);
        }
        else {
            std::cerr << "Output written to " << cur_outfile << "." << std::endl;
        }
    }
}


void create_midi(vrv::Toolkit &toolkit, int all_pages, int from, int to, std::string outfile)
{    
    char *cfg = (char *)"../thirdparty/wildmidi/freepats/freepats.cfg";
    struct _WM_inPlayHook wmhook;

    outfile += ".mid";
    if (!toolkit.RenderToMIDIFile(outfile)) {
        cerr << "Unable to write MIDI to " << outfile << "." << endl;
        return;
    }
    else {
        cerr << "Output written to " << outfile << "." << endl;
    }

    wmhook.handle = &toolkit;
    wmhook.pfn_get_para = get_playpara;
    wmhook.pfn_update = midiUpdate;
    wildmidi(outfile.c_str(), cfg, &wmhook);
}

void create_hum(vrv::Toolkit &toolkit, int all_pages, int from, int to, std::string outfile)
{
    outfile += ".krn";

    if (!toolkit.GetHumdrumFile(outfile)) {
        std::cerr << "Unable to write Humdrum to " << outfile << "." << std::endl;
        exit(1);
    }
    else {
        std::cerr << "Output written to " << outfile << "." << std::endl;
    }
}

void create_mei(vrv::Toolkit &toolkit, int all_pages, int from, int to, std::string outfile)
{
    if (all_pages) {
        toolkit.SetScoreBasedMei(true);
        if (!toolkit.SaveFile(outfile)) {
            std::cerr << "Unable to write MEI to " << outfile << "." << std::endl;
            exit(1);
        }
        else {
            std::cerr << "Output written to " << outfile << "." << std::endl;
        }
    }
    else {
        std::cerr << "MEI output of one page is available only to standard output." << std::endl;
        exit(1);
    }

}

void create_timemap(vrv::Toolkit &toolkit, int all_pages, int from, int to, std::string outfile)
{
    outfile += ".json";
    if (!toolkit.RenderToTimemapFile(outfile)) {
        std::cerr << "Unable to write MIDI to " << outfile << "." << std::endl;
        exit(1);
    }
    else {
        std::cerr << "Output written to " << outfile << "." << std::endl;
    }
}

int MScore_init(vrv::Toolkit &toolkit ,std::string &infile, 
                std::string &outfile, int argc, char **argv)
{
    int show_help = 0;
    int show_version = 0;
    int ret = 0;
    int all_pages = 1;
    int page = 1;
    std::string outformat = "midi";
    struct option *long_options = NULL;

    int i = 0;

    if (argc < 2) {
        std::cerr << "Expected one input file but found none." << std::endl << std::endl;
        display_usage();
        return 1;
    }

    static struct option base_options[] = {
        { "all-pages", no_argument, 0, 'a' },
        { "format", required_argument, 0, 'f' },
        { "help", no_argument, 0, '?' },
        { "outfile", required_argument, 0, 'o' },
        { "page", required_argument, 0, 'p' },
        { "resources", required_argument, 0, 'r' },
        { "scale", required_argument, 0, 's' },
        { "type", required_argument, 0, 't' },
        { "version", no_argument, 0, 'v' },
        { "xml-id-seed", required_argument, 0, 'x' },
        // deprecated - some use undocumented short options to catch them as such
        { "border", required_argument, 0, 'b' },
        { "ignore-layout", no_argument, 0, 'i' },
        { "no-layout", no_argument, 0, 'n' },
        { "page-height-deprecated", required_argument, 0, 'h' },
        { "page-width-deprecated", required_argument, 0, 'w' },
        { 0, 0, 0, 0 }
    };

    vrv::Options *options = toolkit.GetOptions();
    const vrv::MapOfStrOptions *params = options->GetItems();
    int mapSize = (int)params->size();

    int baseSize = sizeof(base_options) / sizeof(option);

    long_options = (struct option *)malloc(sizeof(struct option) * (baseSize + mapSize));

    // A vector of string for storing names as const char* for long_options
    std::vector<std::string> optNames;
    optNames.reserve(mapSize);

    vrv::MapOfStrOptions::const_iterator iter;
    for (iter = params->begin(); iter != params->end(); ++iter) {
        // Double check that back and forth convertion is correct
        assert(toCamelCase(fromCamelCase(iter->first)) == iter->first);

        optNames.push_back(fromCamelCase(iter->first));
        long_options[i].name = optNames.at(i).c_str();
        vrv::OptionBool *optBool = dynamic_cast<vrv::OptionBool *>(iter->second);
        long_options[i].has_arg = (optBool) ? no_argument : required_argument;
        long_options[i].flag = 0;
        long_options[i].val = 0;
        i++;
    }

    // Concatenate the base options
    assert(i == mapSize);
    for (; i < mapSize + baseSize; ++i) {
        long_options[i].name = base_options[i - mapSize].name;
        long_options[i].has_arg = base_options[i - mapSize].has_arg;
        long_options[i].flag = base_options[i - mapSize].flag;
        long_options[i].val = base_options[i - mapSize].val;
    }

    int c;
    std::string key;
    int option_index = 0;
    vrv::Option *opt = NULL;
    vrv::OptionBool *optBool = NULL;
    while ((c = getopt_long(argc, argv, "?ab:f:h:ino:p:r:s:t:w:vx:", long_options, &option_index)) != -1) {
    switch (c) {
        case 0:
            key = long_options[option_index].name;
            opt = params->at(toCamelCase(key));
            optBool = dynamic_cast<vrv::OptionBool *>(opt);
            if (optBool) {
                optBool->SetValue(true);
            }
            else if (opt) {
                if (!opt->SetValue(optarg)) {
                    vrv::LogWarning("Setting option %s with %s failed, default value used",
                        long_options[option_index].name, optarg);
                }
            }
            else {
                vrv::LogError("Something went wrong with option %s", long_options[option_index].name);
                ret = 1;
                goto end;
            }
            break;

        case 'a': all_pages = 1; break;

        case 'b':
            vrv::LogWarning("Option -b and --border is deprecated; use --page-margin-bottom, --page-margin-left, --page-margin-right and "
                       "--page-margin-top instead");
            options->m_pageMarginBottom.SetValue(optarg);
            options->m_pageMarginLeft.SetValue(optarg);
            options->m_pageMarginRight.SetValue(optarg);
            options->m_pageMarginTop.SetValue(optarg);
            break;

        case 'f':
            if (!toolkit.SetFormat(std::string(optarg))) {
                ret = 1;
                goto end;
            };
            break;

        case 'h':
            vrv::LogWarning("Option -h is deprecated; use --page-height instead");
            options->m_pageHeight.SetValue(optarg);
            break;

        case 'i':
            vrv::LogWarning("Option --ignore-layout is deprecated; use --breaks auto");
            options->m_breaks.SetValue(vrv::BREAKS_auto);
            break;

        case 'n':
            vrv::LogWarning("Option --no-layout is deprecated; use --breaks none");
            options->m_breaks.SetValue(vrv::BREAKS_none);
            break;

        case 'o': outfile = std::string(optarg); break;

        case 'p': page = atoi(optarg); break;

        case 'r': vrv::Resources::SetPath(optarg); break;

        case 't':
            outformat = std::string(optarg);
            toolkit.SetOutputFormat(std::string(optarg));
            break;

        case 's':
            if (!toolkit.SetScale(atoi(optarg))) {
                ret = 1;
                goto end;
            }
            break;

        case 'v': show_version = 1; break;

        case 'w':
            vrv::LogWarning("Option -w is deprecated; use --page-width instead");
            options->m_pageWidth.SetValue(optarg);
            break;

        case 'x': vrv::Object::SeedUuid(atoi(optarg)); break;

        case '?':
            display_usage();
            ret = 1;
            goto end;
            break;

        default: break;
    }
    }

    if (show_version) {
        display_version();
        ret = 1;
        goto end;
    }

    if (show_help) {
        display_usage();
        ret = 1;
        goto end;
    }

    if (optind <= argc - 1) {
        infile = std::string(argv[optind]);
    }
    else {
        std::cerr << "Incorrect number of arguments: expected one input file but found none." << std::endl << std::endl;
        display_usage();
        ret = 1;
		goto end;
    }

end:

    if(long_options) {
        free(long_options);
    }

    return ret;
}

int main_t(int argc, char **argv)
{
    std::string infile;
    std::string outfile;
    std::string outformat = "midi";
    bool std_output = false;
    std::string fontName;
	int from;
	int to;

    int all_pages = 1;
    int page = 1;
    vrv::Options *options;

    // Create the toolkit instance without loading the font because
    // the resource path might be specified in the parameters
    // The fonts will be loaded later with Resources::InitFonts()
    vrv::Toolkit toolkit(false);

    if(MScore_init(toolkit, infile, outfile, argc, argv)) {
        goto end;
    }

    options = toolkit.GetOptions();

    vrv::Resources::SetPath("D:\\work\\GitHub\\projects\\MScore\\data");

    // Make sure the user uses a valid Resource path
    // Save many headaches for empty SVGs
    if (!dir_exists(vrv::Resources::GetPath())) {
        std::cerr << "The resources path " << vrv::Resources::GetPath() << " could not be found; please use -r option."
             << std::endl;
        goto end;
    }

    // Load the music font from the resource directory
    if (!vrv::Resources::InitFonts()) {
        std::cerr << "The music font could not be loaded; please check the contents of the resource directory." << std::endl;
        goto end;
    }

    // Load a specified font
    if (!vrv::Resources::SetFont(options->m_font.GetValue())) {
        std::cerr << "Font '" << options->m_font.GetValue() << "' could not be loaded." << std::endl;
        goto end;
    }

    if ((outformat != "svg") && (outformat != "mei") && (outformat != "midi") && (outformat != "timemap")
        && (outformat != "humdrum") && (outformat != "hum")) {
        std::cerr << "Output format (" << outformat << ") can only be 'mei', 'svg', 'midi', or 'humdrum'." << std::endl;
        goto end;
    }

    // Make sure we provide a file name or output to std output with std input
    if ((infile == "-") && (outfile.empty())) {
        std::cerr << "Standard input can be used only with standard output or output filename." << std::endl;
        goto end;
    }

    // Hardcode svg ext for now
    if (outfile.empty()) {
        outfile = removeExtension(infile);
    }
    else if (outfile == "-") {
        // DisableLog();
        std_output = true;
    }
    else {
        outfile = removeExtension(outfile);
    }

    // Load the std input or load the file
    if (infile == "-") {
        std::ostringstream data_stream;
        for (std::string line; getline(std::cin, line);) {
            data_stream << line << std::endl;
        }
        if (!toolkit.LoadData(data_stream.str())) {
            std::cerr << "The input could not be loaded." << std::endl;
            goto end;
        }
    }
    else {
        if (!toolkit.LoadFile(infile)) {
            std::cerr << "The file '" << infile << "' could not be opened." << std::endl;
            goto end;
        }
    }

    if (toolkit.GetOutputFormat() != vrv::HUMDRUM) {
        // Check the page range
        if (page > toolkit.GetPageCount()) {
            std::cerr << "The page requested (" << page << ") is not in the page range (max is " << toolkit.GetPageCount()
                 << ")." << std::endl;
            goto end;
        }
        if (page < 1) {
            std::cerr << "The page number has to be greater than 0." << std::endl;
            goto end;
        }
    }

    from = page;
    to = page + 1;
    if (all_pages) {
        to = toolkit.GetPageCount() + 1;
    }

    if (outformat == "svg") {
        create_svg(toolkit, all_pages, from, to, outfile);
    }
    else if (outformat == "midi") {
        create_midi(toolkit, all_pages, from, to, outfile);
    }
    else if (outformat == "timemap") {

        create_timemap(toolkit, all_pages, from, to, outfile);
    }
    else if (outformat == "humdrum" || outformat == "hum") {
        create_hum(toolkit, all_pages, from, to, outfile);
    }
    else {
        create_mei(toolkit, all_pages, from, to, outfile);
    }

end:

    return 0;
}


#ifdef __cplusplus
}
#endif


