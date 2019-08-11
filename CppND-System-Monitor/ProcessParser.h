#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include <algorithm>

using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumbesrOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

bool ProcessParser::isPidExisting(string pid){
    std::vector<string> pid_list = ProcessParser::getPidList();
    for(string pid_string : pid_list){
        if(pid_string == pid){
            return true;
        }
    }
    return false;
}

string ProcessParser::getOSName()
{
    string line;
    string name = "PRETTY_NAME=";

    ifstream stream; 
    Util::getStream(("/etc/os-release"),stream);

    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              std::size_t found = line.find("=");
              found++;
              string result = line.substr(found);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
        }
    }
    return "";

}

float getSysActiveCpuTime(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}


std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    float active_time = getSysActiveCpuTime(values1) - getSysActiveCpuTime(values2);
    float idle_time = getSysIdleCpuTime(values1) - getSysIdleCpuTime(values2);
    float total_time = active_time + idle_time;
    float cpu_stat = 100*(active_time/total_time);
    std::to_string(cpu_stat);
}


//one of the more involved functions
vector<string> ProcessParser::getPidList(){
    //get the list of directories contatining integer naming and append to a string vector

    DIR* dir; //simple pointer to a directory, we'll first check if the /proc folder exists
    //upon successful opening of the directory, it returns a pointer of type DIR*,
    //hence throw an error if its not the case
    if(!(dir = opendir("/proc"))){
        throw std::runtime_error(std::strerror(errno));
    }

    dirent* dirp; //again a pointer to store the subdirectories in the DIR* type. 
    
    //again readdir(dir) returns a pointer of type dirent*
    std::vector<string> container;
    while(dirp = readdir(dir)){
        //since the folder contains non-directory files, check for it
        // struct dirent has d_type field and if its a directory it's d_type is DT_DIR
        if(dirp->d_type != DT_DIR){
            continue;
        }
        //d_name is a char array [] and hence dereferncing it will give the first value
        //all_of returns true if each check based on lambda function 
        //lambda function checks if each car is a digit not not
        if(all_of(dirp->d_name,dirp->d_name +std::strlen(dirp->d_name),[](char c){return std::isdigit(c);})){
            container.push_back(dirp->d_name);
        }

    }

    if(closedir(dir)!= 0){
        std::runtime_error(std::strerror(errno));
    }
    return container;

}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber)
{
    // It is possible to use this method for selection of data for overall cpu or every core.
    // when nothing is passed "cpu" line is read
    // when, for example "0" is passed  -> "cpu0" -> data for first core is read
    string line;
    string name = "cpu" + coreNumber;
    ifstream stream; 
    Util::getStream((Path::basePath() + Path::statPath()),stream);
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            // set of cpu data active and idle times;
            return values;
        }
    }
    return (vector<string>());
}



string ProcessParser::getCmd(string pid){
    //grab the process path
    std::string line;
    std::ifstream stream;
    Util::getStream(Path::basePath() + pid + Path::cmdPath(),stream);
    std::getline(stream, line);
    return line;
}

int ProcessParser::getNumberOfCores()
{
    // Get the number of host cpu cores
    string line;
    string name = "cpu cores";
    std::ifstream stream; 
    Util::getStream((Path::basePath() + "cpuinfo"),stream);
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;
}




string ProcessParser::getVmSize(string pid){
   //grab the string that matches a part
    std::string req_string = "VmData";
    std::ifstream stream;
    Util::getStream(Path::basePath() + pid  +Path::statusPath(),stream);
    std::string line;
    //read line by line
    float result;
    while(std::getline(stream,line)){
        //compare each line and check if it matches with our req template
        if(line.compare(0,req_string.length(),req_string)==0){
            //now that it matches, we need chunks of string
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf),end;
            std::vector<string> values(beg,end);

            result = stof(values[1])/float(1024*1024);
            break;
        }
    }
    return std::to_string(result);
}


long int ProcessParser::getSysUpTime(){
    string path = Path::basePath() + Path::upTimePath();
    //open the file using ifstream and then get the line
    // long in/t result; 
    std::ifstream stream;
    Util::getStream(path,stream);
    std::string line;
    std::getline(stream,line);
    //create a string stream buffer and append chunks of string into a vec.
    std::stringstream buf(line);
    std::istream_iterator<string> beg(buf),end;
    std::vector<string> values(beg,end);
    stream.close();
    return stoi(values[0]);
 }



std::string ProcessParser::getCpuPercent(string pid){
    //get the uptime of the cpu first
    long int uptime = ProcessParser::getSysUpTime();
    //open file stream to pid specific stat
    std::ifstream stream;
    // std::string values;
    Util::getStream(Path::basePath() + pid  + "/" + Path::statPath(),stream);
    //read the line, create a buffer and get the specific time intervals
    std::string line;
    std::getline(stream,line);
    std::istringstream buf(line);
    std::istream_iterator<string> beg(buf),end;
    std::vector<string> values(beg,end);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float hertz = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = (uptime - (starttime/hertz));  

    float cpu_usage = 100 * ((total_time/hertz)/seconds);
    stream.close();
    return std::to_string(cpu_usage);
}


std::string ProcessParser::getProcUpTime(string pid){
    std::ifstream stream;
    std::string line;
    // std::string values;
    Util::getStream(Path::basePath() + pid + "/" + Path::statPath(),stream);
    //get the stream line
    std::getline(stream, line);
    std::stringstream buf(line);
    //get the  start and end of this buffer without the whitespaces
    std::istream_iterator<string> beg(buf),end;
    //store in a vector of strings
    std::vector<string> values(beg,end);
    float seconds = stof(values[13])/(sysconf(_SC_CLK_TCK));
    stream.close();
    return std::to_string(seconds);
}


string ProcessParser::getProcUser(string pid){
    
    //get the uid first from /[PID]/status
    std::string req_string = "Uid:";
    std::ifstream stream;
    Util::getStream(Path::basePath() + pid + Path::statusPath(),stream);
    std::string line;
    //read line by line
    string result;
    std::string value;
    while(std::getline(stream,line)){
        //compare each line and check if it matches with our req template
        if(line.compare(0,req_string.length(),req_string)==0){
            //now that it matches, we need chunks of string
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf),end;
            std::vector<string> value(beg,end);
            
            result = value[1];
            break;
        }
    }
    stream.close();
    //now open /etc/passwd to get get username corresponding to the Uid
    //overwriting the stream
    std::string username;
    Util::getStream("/etc/passwd",stream);
    while(std::getline(stream,line)){
        //compare each line and check if it matches with our req template
        if(line.find(result)!=std::string::npos){
            //now that it matches, we need chunks of string
            username = line.substr(0,line.find(":"));
            // result = value[0];
            return username;
        }
    }
    stream.close();
    return "";
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    string value;
    int result;
    ifstream stream;
    Util::getStream((Path::basePath() + Path::memInfoPath()),stream);
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
            break;
        if (line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    //calculating usage:
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

string ProcessParser::getSysKernelVersion()
{
    string line;
    string name = "Linux version ";
    ifstream stream ;
    Util::getStream((Path::basePath() + Path::versionPath()),stream);
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";
}

int ProcessParser::getTotalNumberOfProcesses()
{
    string line;
    int result = 0;
    string name = "processes";
    ifstream stream;
    Util::getStream((Path::basePath() + Path::statPath()),stream);
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

int ProcessParser::getNumberOfRunningProcesses()
{
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream stream;
    Util::getStream((Path::basePath() + Path::statPath()),stream);
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

int ProcessParser::getTotalThreads(){

    //get the pidlist and for each pid check the number of threads.
    std::vector<string> pid_list = ProcessParser::getPidList();
    //grab the Threads: integet from /proc/[pid]/stat

    std::string line;
    std::string req_string = "Threads:";
    std::ifstream stream;
    std::string values;
    int threads = 0;
    for (auto list : pid_list){
        Util::getStream(Path::basePath() + list + Path::statusPath(),stream);
        while(std::getline(stream,line)){
            if(line.compare(0,req_string.size(),req_string)==0){
                std::istringstream buf(line);
                std::istream_iterator<string> beg(buf),end;
                std::vector<string> values(beg,end);
                threads += stoi(values[1]);
            }

        }
    }
    return threads;
}

