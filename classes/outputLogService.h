#ifndef OUTPUTLOGSERVICE_H
#define OUTPUTLOGSERVICE_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <assert.h>
#include "timeStructure.h"

using namespace std;

class OutputLogService
{
  public:
    //files that we know will be used so we dont have to manage strings
    string CellDataKey;
    string ConnSdLogKey;
    string HappinessLogKey;
    string OutputConLogKey;
    string OutputSumLogKey;
    string SpikeTimesLogKey;
    string EatenFoodLogKey;

    //Many things need to log time so put a reference to the time structure
    TimeStructure &Time;

    OutputLogService(string outputFolderPath, string trial, TimeStructure &time);
    ~OutputLogService();

    ofstream &GetLog(const string &key);
    void CloseLog(const string &key);
    void CloseAllFiles();

  private:
    string Trial;
    string OutputFolderPath;
    map<string, ofstream *> Logs;

    bool EnsureOutputLogExists(string key);
};

#endif // OUTPUTLOGSERVICE_H