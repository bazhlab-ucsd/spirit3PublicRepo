#include "outputLogService.h"

using namespace std;

OutputLogService::OutputLogService(string outputFolderPath, string trial, TimeStructure &time) : Time(time)
{
    OutputFolderPath = outputFolderPath;
    Trial = trial;
    CellDataKey = "CellDataFile";
    ConnSdLogKey = "conn_sd.out";
    HappinessLogKey = "happiness.out";
    OutputConLogKey = "output.conn";
    OutputSumLogKey = "output_sum";
    SpikeTimesLogKey = "spike_times.out";
    EatenFoodLogKey = "eatenFood.out";
}

bool OutputLogService::EnsureOutputLogExists(string key)
{
    if (Logs[key] == NULL) // if log doesnt exist
    {
      //create new log
      Logs[key] = new ofstream();
      Logs[key]->open((OutputFolderPath + key + "." + Trial).c_str());
    }
    if(!(Logs[key]->good())){
      cout << "Log file not good: " << key << endl;
      cout << "pointer: " << Logs[key] << endl;
      cout << " good()=" << Logs[key]->good() << endl;
      cout << " eof()=" << Logs[key]->eof() << endl;
      cout << " fail()=" << Logs[key]->fail() << endl;
      cout << " bad()=" << Logs[key]->bad() << endl;
      return false;
    }
    return true;
}

ofstream &OutputLogService::GetLog(const string &key)
{
    assert(EnsureOutputLogExists(key));
    return *Logs[key];
}

void OutputLogService::CloseAllFiles()
{
  cout << "          Start Close all Files" << endl;
  for (std::map<string, ofstream *>::iterator it = Logs.begin(); it != Logs.end(); ++it)
  {
    cout << "          Closing " << it->first << endl;
    cout << "          Pointer " << it->second << endl;
    if (it->second != NULL)
    {
      it->second->flush();
      it->second->close();
      delete it->second;
    }
    cout << "          -EndClose " << endl; 
  }
  Logs.clear();
}

OutputLogService::~OutputLogService()
{
    CloseAllFiles();
}

void OutputLogService::CloseLog(const string &key){
  if (Logs[key] != NULL){
    Logs[key]->flush();
    Logs[key]->close();
    delete Logs[key];
  }
  Logs.erase(key);  
}
