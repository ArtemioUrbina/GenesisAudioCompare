/* 
 * MDFourier
 * A Fourier Transform analysis tool to compare different 
 * Sega Genesis/Mega Drive audio hardware revisions, and
 * other hardware in the future
 *
 * Copyright (C)2019 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 *
 * Requires the FFTW library: 
 *	  http://www.fftw.org/
 * 
 */

#include "cline.h"
#include "log.h"

void PrintUsage()
{
	// b,d and y options are not documented since they are mostly for testing or found not as usefull as desired
	logmsg("  usage: mdfourier -r reference.wav -c compare.wav\n");
	logmsg("   FFT and Analysis options:\n");
	logmsg("	 -a: select <a>udio channel to compare. 's', 'l' or 'r'\n");
	logmsg("	 -w: enable <w>indowing. Default is a custom Tukey window.\n");
	logmsg("		'n' none, 't' Tukey, 'h' Hann, 'f' FlatTop & 'm' Hamming\n");
	logmsg("	 -f: Change the number of analyzed frequencies to use from FFTW\n");
	logmsg("	 -s: Defines <s>tart of the frequency range to compare with FFT\n");
	logmsg("	 -e: Defines <e>nd of the frequency range to compare with FFT\n");
	logmsg("	 -i: <i>gnores the silence block noise floor if present\n");
	logmsg("	 -t: Defines the <t>olerance when comparing amplitudes in dBFS\n");
	logmsg("	 -z: Uses <z>ero Padding to equal 1 hz FFT bins\n");
	logmsg("	 -n: <N>ormalize: 't' Time Domain Max, 'f' Frequency Domain Max or 'a' Average\n");
	logmsg("	 -B: Do not do stereo channel audio <B>alancing\n");
	logmsg("   Output options:\n");
	logmsg("	 -l: <l>og output to file [reference]_vs_[compare].txt\n");
	logmsg("	 -v: Enable <v>erbose mode, spits all the FFTW results\n");
	logmsg("	 -g: Create avera<g>e points over the plotted graphs\n");
	logmsg("	 -A: Do not weight values in <A>veraged Plot (implies -g)\n");
	logmsg("	 -L: Create 800x400 plots as shown in the manual\n");
	logmsg("	 -H: Create 1920x1080 plots\n");
	logmsg("	 -D: Don't create <D>ifferences Plots\n");
	logmsg("	 -M: Don't create <M>issing Plots\n");
	logmsg("	 -S: Don't create <S>pectrogram Plots\n");
	logmsg("	 -d: Max <d>BFS for plots vertically\n");
	logmsg("	 -k: cloc<k> FFTW operations\n");
	logmsg("	 -j: (text) Cuts per block information and shows <j>ust total results\n");
	logmsg("	 -x: (text) Enables e<x>tended log results. Shows a table with all matches\n");
	logmsg("	 -m: (text) Enables Show all blocks compared with <m>atched frequencies\n");
}

void Header(int log)
{
	char title1[] = "MDFourier " MDVERSION " [240p Test Suite Fourier Audio compare tool]\n";
	char title2[] = "Artemio Urbina 2019 free software under GPL\n\n";

	if(log)
		logmsg("%s%s", title1, title2);
	else
		printf("%s%s", title1, title2);
}

void CleanParameters(parameters *config)
{
	memset(config, 0, sizeof(parameters));

	sprintf(config->profileFile, "mdfblocks.mfn");
	config->tolerance = DBS_TOLERANCE;
	config->startHz = START_HZ;
	config->endHz = END_HZ;
	config->startHzPlot = 0;
	config->endHzPlot = END_HZ;
	config->maxDbPlotZC = DB_HEIGHT;
	config->extendedResults = 0;
	config->justResults = 0;
	config->verbose = 0;
	config->window = 't';
	config->channel = 's';
	config->MaxFreq = FREQ_COUNT;
	config->clock = 0;
	config->showAll = 0;
	config->ignoreFloor = 0;
	config->useOutputFilter = 1;
	config->outputFilterFunction = 3;
	config->origSignificantVolume = SIGNIFICANT_VOLUME;
	config->significantVolume = SIGNIFICANT_VOLUME;
	config->smallerFramerate = 0;
	config->ZeroPad = 0;
	config->debugSync = 0;
	config->drawWindows = 0;
	config->channelBalance = 1;

	config->logScale = 1;
	config->reverseCompare = 0;
	config->normType = max_frequency;

	config->plotResX = PLOT_RES_X;
	config->plotResY = PLOT_RES_Y;

	config->plotDifferences = 1;
	config->plotMissing = 1;
	config->plotSpectrogram = 1;
	config->averagePlot = 0;
	config->weightedAveragePlot = 1;

	config->Differences.BlockDiffArray = NULL;
	config->Differences.cntFreqAudioDiff = 0;
	config->Differences.cntAmplAudioDiff = 0;
	config->Differences.weightedFreqAudio = 0;
	config->Differences.weightedAmplAudio = 0;
	config->Differences.cntTotalCompared = 0;
	config->Differences.cntTotalAudioDiff = 0;
	config->Differences.weightedAudioDiff = 0;
	
	config->types.totalChunks = 0;
	config->types.regularChunks = 0;
	config->types.platformMSPerFrame = 16.6905;
	config->types.pulseSyncFreq = 8820;
	config->types.pulseMinVol = -25;
	config->types.pulseVolDiff = 25;
	config->types.pulseFrameMinLen = 14;
	config->types.pulseFrameMaxLen = 18;
	config->types.pulseCount = 10;
	config->types.typeArray = NULL;
	config->types.typeCount = 0;

	config->sync_plan = NULL;
	config->model_plan = NULL;
	config->reverse_plan = NULL;
}

int commandline(int argc , char *argv[], parameters *config)
{
	FILE *file = NULL;
	int c, index, ref = 0, tar = 0;
	
	opterr = 0;
	
	CleanParameters(config);

	while ((c = getopt (argc, argv, "hxjzmviklygLHo:s:e:f:t:p:a:w:r:c:d:P:SDMNRAWDBn:")) != -1)
	switch (c)
	  {
	  case 'h':
		PrintUsage();
		return 0;
		break;
	  case 'x':
		config->extendedResults = 1;
		break;
	  case 'j':
		config->justResults = 1;
		break;
	  case 'm':
		config->showAll = 1;
		break;
	  case 'z':
		config->ZeroPad = 1;
		break;
	  case 'v':
		config->verbose = 1;
		break;
	  case 'i':
		config->ignoreFloor = 1;
		break;
	  case 'k':
		config->clock = 1;
		break;
	  case 'y':
		config->debugSync = 1;
		break;
	  case 'g':
		config->averagePlot = 1;
		break;
	  case 'l':
		EnableLog();
		break;
	  case 'W':
		config->drawWindows = 1;
		break;
	  case 'L':
		config->plotResX = PLOT_RES_X_LOW;
		config->plotResY = PLOT_RES_Y_LOW;
		break;
	  case 'H':
		config->plotResX = PLOT_RES_X_HIGH;
		config->plotResY = PLOT_RES_Y_HIGH;
		break;
	  case 'o':
		config->outputFilterFunction = atoi(optarg);
		if(config->outputFilterFunction < 0 || config->outputFilterFunction > 5)
			config->outputFilterFunction = 3;
		if(!config->outputFilterFunction)
			config->useOutputFilter = 0;
		break;
	  case 's':
		config->startHz = atof(optarg);
		if(config->startHz < 1.0 || config->startHz > END_HZ-100.0)
			config->startHz = START_HZ;
		break;
	  case 'e':
		config->endHz = atof(optarg);
		if(config->endHz < START_HZ*2.0 || config->endHz > END_HZ)
			config->endHz = END_HZ;
		break;
	  case 'd':
		config->maxDbPlotZC = atof(optarg);
		if(config->maxDbPlotZC < 0.1 || config->maxDbPlotZC > 60.0)
			config->maxDbPlotZC = DB_HEIGHT;
		break;
	  case 'f':
		config->MaxFreq = atoi(optarg);
		if(config->MaxFreq < 1 || config->MaxFreq > MAX_FREQ_COUNT)
			config->MaxFreq = MAX_FREQ_COUNT;
		break;
	  case 't':
		config->tolerance = atof(optarg);
		if(config->tolerance < 0.0 || config->tolerance > 40.0)
			config->tolerance = DBS_TOLERANCE;
		break;
	  case 'p':
		config->significantVolume = atof(optarg);
		if(config->significantVolume <= -100.0 || config->significantVolume >= -1.0)
			config->significantVolume = SIGNIFICANT_VOLUME;
		config->origSignificantVolume = config->significantVolume;
		break;
	  case 'a':
		switch(optarg[0])
		{
			case 'l':
			case 'r':
			case 's':
				config->channel = optarg[0];
				break;
			default:
				logmsg("Invalid audio channel option '%c'\n", optarg[0]);
				logmsg("\tUse l for Left, r for Right or s for Stereo\n");
				return 0;
				break;
		}
		break;
	 case 'w':
		switch(optarg[0])
		{
			case 'n':
			case 'f':
			case 'h':
			case 't':
			case 'm':
				config->window = optarg[0];
				break;
			default:
				logmsg("Invalid Window for FFT option '%c'\n", optarg[0]);
				logmsg("\tUse n for None, t for Tukey window (default), f for Flattop, h for Hann or m for Hamming window\n");
				return 0;
				break;
		}
		break;
	  case 'r':
		sprintf(config->referenceFile, "%s", optarg);
		ref = 1;
		break;
	  case 'c':
		sprintf(config->targetFile, "%s", optarg);
		tar = 1;
		break;
	  case 'P':
		sprintf(config->profileFile, "%s", optarg);
		break;
	  case 'D':
		config->plotDifferences = 0;
		break;
	  case 'M':
		config->plotMissing = 0;
		break;
	  case 'S':
		config->plotSpectrogram = 0;
		break;
	  case 'N':
		config->logScale = 0;
		break;
	  case 'R':
		config->reverseCompare = 1;
		break;
	  case 'B':
		config->channelBalance = 0;
		break;
	  case 'n':
		switch(optarg[0])
		{
			case 't':
				config->normType = max_time;
				break;
			case 'f':
				config->normType = max_frequency;
				break;
			case 'a':
				config->normType = average;
				break;
			default:
				logmsg("Invalid Normalization option '%c'\n", optarg[0]);
				logmsg("\tUse 't' Time Domain Max, 'f' Frequency Domain Max or 'a' Average\n");
				return 0;
				break;
		}
		break;
		break;
	  case 'A':
		config->averagePlot = 1;
		config->weightedAveragePlot = 0;
		break;
	  case '?':
		if (optopt == 'r')
		  logmsg("Reference File -%c requires an argument.\n", optopt);
		else if (optopt == 'c')
		  logmsg("Compare File -%c requires an argument.\n", optopt);
		else if (optopt == 'a')
		  logmsg("Audio channel option -%c requires an argument: l,r or s\n", optopt);
		else if (optopt == 'w')
		  logmsg("FFT Window option -%c requires an argument: n,t,f or h\n", optopt);
		else if (optopt == 'o')
		  logmsg("Output curve -%c requires an argument 0-4\n", optopt);
		else if (optopt == 't')
		  logmsg("Amplitude tolerance -%c requires an argument: 0.0-40.0 dBFS\n", optopt);
		else if (optopt == 'p')
		  logmsg("Significant Volume -%c requires an argument: -1.0 to -100.0 dBFS\n", optopt);
		else if (optopt == 'f')
		  logmsg("Max # of frequencies to use from FFTW -%c requires an argument: 1-%d\n", optopt, MAX_FREQ_COUNT);
		else if (optopt == 's')
		  logmsg("Min frequency range for FFTW -%c requires an argument: %d-%d\n", 1, END_HZ-100, optopt);
		else if (optopt == 'e')
		  logmsg("Max frequency range for FFTW -%c requires an argument: %d-%d\n", START_HZ*2, END_HZ, optopt);
		else if (optopt == 'd')
		  logmsg("Max DB Height for Plots -%c requires an argument: %g-%g\n", 0.1, 60.0, optopt);
		else if (isprint (optopt))
		  logmsg("Unknown option `-%c'.\n", optopt);
		else if (optopt == 'P')
		  logmsg("Profile File -%c requires a file argument\n", optopt);
		else
		  logmsg("Unknown option character `\\x%x'.\n", optopt);
		return 0;
		break;
	  default:
		logmsg("Invalid argument %c\n", optopt);
		return(0);
		break;
	  }
	
	for (index = optind; index < argc; index++)
	{
		logmsg("Invalid argument %s\n", argv[index]);
		return 0;
	}

	if(!ref || !tar)
	{
		logmsg("Please define both reference and compare audio files\n");
		return 0;
	}

	if(config->extendedResults && config->justResults)
	{
		logmsg("Just Results cancels Extended results\n");
		return 0;
	}

	if(config->showAll && config->justResults)
	{
		logmsg("Just Results cancels Show All\n");
		return 0;
	}
	if(config->endHz <= config->startHz)
	{
		logmsg("Invalid frequency range for FFTW (%g Hz to %g Hz)\n", 
				config->startHz, config->endHz);
		return 0;
	}

	if(!config->plotDifferences && !config->plotMissing &&
		!config->plotSpectrogram && !config->averagePlot)
	{
		logmsg("It makes no sense to process everything and plot nothing\nAborting.\n");
		return 0;
	}

	file = fopen(config->referenceFile, "rb");
	if(!file)
	{
		logmsg("\tCould not open REFERENCE file: \"%s\"\n", config->referenceFile);
		return 0;
	}
	fclose(file);

	file = fopen(config->targetFile, "rb");
	if(!file)
	{
		logmsg("\tCould not open COMPARE file: \"%s\"\n", config->targetFile);
		return 0;
	}
	fclose(file);

	CreateFolderName(config);
	CreateBaseName(config);

	if(IsLogEnabled())
	{
		char tmp[T_BUFFER_SIZE*2];

		ComposeFileName(tmp, "Log", ".txt", config);

		if(!setLogName(tmp))
			return 0;

		DisableConsole();
		Header(1);
		EnableConsole();
	}

	logmsg("\tUsing %s profile configuration file\n", config->profileFile);
	if(config->window != 'n')
		logmsg("\tA %s window will be applied to each block to be compared\n", GetWindow(config->window));
	else
		logmsg("\tNo window (rectangle) will be applied to each block to be compared\n");
	if(config->useOutputFilter)
	{
		logmsg("\tOutput Filter function #%d will be applied to the results\n", 
				config->outputFilterFunction);
	}
	else
		logmsg("\tNo filtering will be applied to the results\n");
	if(config->ZeroPad)
		logmsg("\tFFT bins will be aligned to 1hz, this is slower\n");
	if(config->ignoreFloor)
		logmsg("\tIgnoring Silence block noise floor\n");
	if(config->channel != 's')
		logmsg("\tAudio Channel is: %s\n", GetChannel(config->channel));
	if(config->tolerance != 0.0)
		logmsg("\tAmplitude tolerance while comparing is +/-%0.2f dBFS\n", config->tolerance);
	if(config->MaxFreq != FREQ_COUNT)
		logmsg("\tMax frequencies to use from FFTW are %d (default %d)\n", config->MaxFreq, FREQ_COUNT);
	if(config->startHz != START_HZ)
		logmsg("\tFrequency start range for FFTW is now %g (default %g)\n", config->startHz, START_HZ);
	if(config->endHz != END_HZ)
		logmsg("\tFrequency end range for FFTW is now %g (default %g)\n", config->endHz, END_HZ);
	if(config->normType != max_frequency)
	{
		if(config->normType == max_time)
			logmsg("\tUsing Time Domain Normalization\n");
		if(config->normType == average)
			logmsg("\tUsing Average Fundamental Frequency Normalization\n");
	}
	if(!config->logScale)
		logmsg("\tPlots will not be adjusted to log scale\n");
	if(config->averagePlot && !config->weightedAveragePlot)
		logmsg("\tAveraged Plots will not be weighted\n");
	return 1;
}

void CreateFolderName(parameters *config)
{
	int len;
	char tmp[BUFFER_SIZE];

	if(!config)
		return;

	sprintf(tmp, "%s", basename(config->referenceFile));
	len = strlen(tmp);
	sprintf(tmp+len-4, "_vs_%s", basename(config->targetFile));
	len = strlen(tmp);
	tmp[len-4] = '\0';

	len = strlen(tmp);
	for(int i = 0; i < len; i++)
	{
		if(tmp[i] == ' ')
			tmp[i] = '_';
	}

	sprintf(config->compareName, "%s", tmp);
	sprintf(config->folderName, "MDFResults\\%s", tmp);

#if defined (WIN32)
	mkdir("MDFResults");
	mkdir(config->folderName);
#else
	mkdir("MDFResults", 0755);
	mkdir(config->folderName, 0755);
#endif
}

void InvertComparedName(parameters *config)
{
	int len;
	char tmp[2500];

	sprintf(tmp, "%s", basename(config->targetFile));
	len = strlen(tmp);
	sprintf(tmp+len-4, "_vs_%s", basename(config->referenceFile));
	len = strlen(tmp);
	tmp[len-4] = '\0';

	sprintf(config->compareName, "%s", tmp);
}

void CreateFolderName_wave(parameters *config)
{
	int len;
	char tmp[2500];

	if(!config)
		return;

	sprintf(tmp, "MDWave\\%s", basename(config->referenceFile));
	len = strlen(tmp);
	tmp[len-4] = '\0';

	sprintf(config->folderName, "%s", tmp);

#if defined (WIN32)
	mkdir("MDWave");
	mkdir(config->folderName);
#else
	mkdir("MDWave", 0755);
	mkdir(config->folderName, 0755);
#endif
}

char *GetNormalization(enum normalize n)
{
	switch(n)
	{
		case max_time:
			return "TD";
		case max_frequency:
			return "FD";
		case average:
			return "AV";
		default:
			return "ERROR";
	}
}

void CreateBaseName(parameters *config)
{
	if(!config)
		return;
	
	sprintf(config->baseName, "_f%d_%s_%s_v_%g_OF%d_%s_%s_%s",
			config->MaxFreq,
			GetWindow(config->window),
			GetChannel(config->channel),
			fabs(config->significantVolume),
			config->outputFilterFunction,
			GetNormalization(config->normType),
			config->ZeroPad ? "ZP" : "NP",
			config->channelBalance ? "B" : "NB");
}

void ComposeFileName(char *target, char *subname, char *ext, parameters *config)
{
	if(!config)
		return;

	sprintf(target, "%s\\%s%s%s",
		config->folderName, subname, config->baseName, ext); 
}

double TimeSpecToSeconds(struct timespec* ts)
{
	return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

char *GetChannel(char c)
{
	switch(c)
	{
		case 'l':
			return "Left";
		case 'r':
			return "Right";
		case 's':
			return "Stereo";
		default:
			return "ERROR";
	}
}

char *GetWindow(char c)
{
	switch(c)
	{
		case 'n':
			return "Rectangular";
		case 't':
			return "Tukey";
		case 'f':
			return "Flattop";
		case 'h':
			return "Hann";
		case 'm':
			return "Hamming";
		default:
			return "ERROR";
	}
}
