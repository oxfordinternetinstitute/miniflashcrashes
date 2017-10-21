# Code and data for Mini Flash Crashes paper

If you use this code or data, please cite our academic publication:


    Levine, Z.S., Hale, S.A., and Floridi, L. (2017). The October 2014 United States Treasury Bond Flash Crash and the Contributory Effect of Mini Flash Crashes. PLoS ONE.


## Raw data processing
The raw data we used for the paper is available for purchase from [Nanex](http://www.nanex.net/historical.html) and other companies. We purchased our data from Nanex for the period of 1 October 2014 to 15 October 2014. The data was in a binary format as [described within their documentation](https://web.archive.org/web/20160416094247/http://nxcoreapi.com:80/doc/concept_Basics.html).

We first converted the data to a text format (tab-separated values) by writing [a simple C++ program](TradeCSV/). The source code for this program is in the TradeCSV directory and can be compiled with Microsoft Visual Studio or another suitable compiler. Accessing the Nanex data requires the NxCoreAPI.dll library, which was only available for Windows at the time of our analysis. 

The NxCoreAPI.dll file should be placed in the same directory as the executable. The program can then be executed from the command line. It expects the first argument to be the name of a Nanex data file. If additional arguments are given, the program will only output data for ticker symbols matching one of these additional arguments. For our analysis, we processed each data file from Nanex one at a time with the name of the data file as the only argument. For example,

    TradeCSV.exe 20141001.GS.nxc
produced a file named ``output_20141001.GS.nxc.txt`` with the data in text format. Specifically, each line of the file has the following fields separated with tabs.

    symbol: The ticker symbol of the equity being traded
    time_of_day: The time of the day in the format HH:MM:SS.milliseconds
    size: The size of the trade
    price: The price at which the trade was executed
    opening_price
    high_price
    low_price
    last_price
    total_volume
    net_change

Within our further analysis, we only use columns 1, 2, and 4 (symbol, time_of_day, and price).

## Extracting miniflash crashes

Having converted each Nanex data file to a text file using the above steps, we processed the files using Python3. 

### Sorting data to separate files
We first partitioned the data for each equity into separate files by writing [a simple Python3 script](processing/partition_data.ipynb). This file also adds the date as the first field in the file.

### Counting miniflash crashes
After partitioning the data, we had one directory per day and within each directory one file per equity. Each of these files contained all the trades for that equity for the day in chronological order.

We wrote [another Python script](processing/count_flash_crashes.ipynb) to process each file and identify all miniflash crashes within the data. The results of this script are in the [data/](data/) directory of this repository. Specifically, there is one comma-separated values file per day with a filename following the format of ``YYYYMMDD_miniflash_crashes.csv`` . Each file has a header row and the following fields separated with commas.


    symbol: A four letter code that is unique to the equity being traded
    start_time: The start of the decrease (increase) corresponding to one miniflash crash
    end_time: The end of the decrease (increase) corresponding to one miniflash crash
    price_change: The change in the price during the miniflash crash: (end_price-start_price)/start_price
    ticks: The number of trades during the miniflash crash

## Analysis

Finally, we analysed our data with [Python3](analysis/basic_stats.ipynb). The figures and table in the paper are in this notebook. The figures were post-processed and annotated with Inkscape and GIMP.


