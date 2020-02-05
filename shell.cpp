#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <vector>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <algorithm>
using namespace std;
int fd[2],in,out,pid;
vector<string> tparts;
bool quotTrig=false;

string trim (string input){
    int i=0;
    while (i < input.size() && input [i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr (i);
    else{
        return "";
    }
    
    i = input.size() - 1;
    while (i>=0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr (0, i+1);
    else
        return "";
    
    return input;
}

/*vector<string> split (string line, string separator=" "){
    int front = line.find('"');
    int rear = line.rfind('"');
    if (front == -1&& rear ==-1)
    {
        front = line.find('\'');
        rear = line.rfind('\'');
        if(front !=-1 && rear !=-1)
        {
            line.erase(front-1);
            line.erase(rear-1);
        }
    }
    else
    {
        line.erase(front-1);
        line.erase(rear-1);
    }
    vector<string> result;
    while (line.size()){
        size_t found = line.find(separator);
        // && found<front) || (found == string::npos&& found>rear) || (front ==-1)
        if ((found == string::npos){
            string lastpart = trim (line);
            if (lastpart.size()>0){
                result.push_back(lastpart);
            }
            break;
        }
        string segment = trim (line.substr(0, found));
        //cout << "line: " << line << "found: " << found << endl;
        line = line.substr (found+1);

        //cout << "[" << segment << "]"<< endl;
        if (segment.size() != 0) 
            result.push_back (segment);
    }
    return result;
}*/

vector<string> split (string line, string separator=" "){
    vector<string> result;
    while (line.size()){
        size_t found = line.find(separator);
        if (found == string::npos){
                string lastpart = trim (line);
                if (lastpart.size()>0){
                    result.push_back(lastpart);
                }
                break;
            }
        string segment = trim (line.substr(0, found));
        line = line.substr (found+1);
        if (segment.size() != 0) 
            result.push_back (segment);
    }
    return result;
}

char** vec_to_char_array (vector<string> parts){
    char ** result = new char * [parts.size() + 1]; // add 1 for the NULL at the end
    for (int i=0; i<parts.size(); i++){
        // allocate a big enough string
        result [i] = new char [parts [i].size() + 1]; // add 1 for the NULL byte
        strcpy (result [i], parts[i].c_str());
    }
    result [parts.size()] = NULL;
    return result;
}

void changeDirectory(vector<string>  args){
    if(args.size()>1){
            //cout<<"change dir "<<argstrings[1]<<endl;
        //treat the ~ as the home dir
        string prevDir = getenv("OLDPWD");
        setenv("OLDPWD",getenv("PWD"),1);
        if(args[1].substr(0,1) == "~"){
            //args[1] = args[1].substr(1,args[1].size());//remove the ~
            //cerr<<"cd to home dir"<<endl;
            string homeDir = getenv("HOME");
            args[1] = homeDir;
        }
        else if(args[1].substr(0,1) == "-"){
            //args[1] = args[1].substr(1,args[1].size());//remove the -
            //cerr<<"cd to prev dir"<<endl;
            //cerr<<prevDir<<endl;
            args[1] = prevDir;
        }
        else if(args[1].substr(0,2) == ".."){
            //args[1] = args[1].substr(1,args[1].size());//remove the -
            //cerr<<"cd to prev dir"<<endl;
            args[1] = getenv("PWD");
            
            string dir = args[1];
            int parPos;
            for(int i = args[1].size()-2;i >=  0; i-- ){//-2 because we dont care if the final / exists
                //find first occurance of the /
                if(dir[i]=='/'){
                    parPos = i+1;
                    break;
                }
            }
            
            dir = dir.substr(0,parPos);
            //dir += '/';
            args[1] = dir;
            //cerr<<"final Dir:"<<args[1]<<" "<<parPos<<endl;
        }
        if (0 == chdir(args[1].c_str())) {

            char* buf;
            getcwd(buf,FILENAME_MAX);
            setenv("PWD",buf,1);
        }
        else{
            cout <<args[1].c_str()<<"failed to change dir"<<endl;
        }
    }
    else{
        //cout<<"no directory given"<<endl;
        char* homeDir = getenv("HOME");

        if (0 == chdir(homeDir)) {
            char* buf;
            getcwd(buf,FILENAME_MAX);
            setenv("PWD",buf,1);
        }
        else{
            cout << "failed to change dir"<<endl;
        }
    }
}

/*void execute (string command){
    vector<string> argstrings = split (command, " "); // split the command into space-separated parts
    char** args = vec_to_char_array (argstrings);// convert vec<string> into an array of char*


    vector<string> inString = split(command,"<");
    if(inString.size()> 1){
        vector<string> fileInVec = split(inString[1]," ");
        int fdIn = open(fileInVec[0].c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
        if(fdIn == -1){//error handle 
            perror("");
        }   
        else{
            dup2(fdIn,0);
            close(fdIn);
        }
        command = inString[0];
        for(int i = 1; i<fileInVec.size();i++){
            command += " "+ fileInVec[i];
        }
    }


   vector<string> outString = split(command,">");
    if(outString.size()> 1){
        vector<string> fileOutVec = split(outString[1]," ");
        int fdOut = open(fileOutVec[0].c_str(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
        dup2(fdOut,1);
        close(fdOut);
        command = outString[0];
        for(int i = 1; i<fileOutVec.size();i++){
            command += " "+ fileOutVec[i];
        }
    }


    if (argstrings[0]== "cd"){//determine if the command is cd
        cout<<"cd found"<<endl;
        changeDirectory(argstrings);
    }
    else
    {
        cout<<args[0]<<endl;
        execvp(args[0],args);
    }
    
}*/
void execute (string commandIn){
    string command = commandIn;
    vector<string> inString = split(command,"<");
    if(inString.size()> 1&&quotTrig==false){
        vector<string> fileInVec = split(inString[1]," ");
        int fdIn = open(fileInVec[0].c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
        if(fdIn == -1){//error handle 
            perror("");
        }   
        else{
            dup2(fdIn,0);
            close(fdIn);
        }
        command = inString[0];
        for(int i = 1; i<fileInVec.size();i++){
            command += " "+ fileInVec[i];
        }
    }
    
    vector<string> outString = split(command,">");
    if(outString.size()> 1&&quotTrig==false){
        vector<string> fileOutVec = split(outString[1]," ");
        int fdOut = open(fileOutVec[0].c_str(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
        dup2(fdOut,1);
        close(fdOut);
        command = outString[0];
        for(int i = 1; i<fileOutVec.size();i++){
            command += " "+ fileOutVec[i];
        }
    }
    
    vector<string> argstrings = split (command, " "); // split the command into space-separated parts
    
    char** args = vec_to_char_array (argstrings);// convert vec<string> into an array of char*
    if (argstrings[0]== "cd"){//determine if the command is cd
        
        changeDirectory(argstrings);
    }
    else{

        if( -1 == execvp (args[0], args)){//handle exec error
            perror("");
        }
    }

}
void initDir(){//set the default diretory to the users home dir
    char buf[4096];
    char* homeDir = getenv("HOME");
    //std::cout  << "CWD: " << getcwd(buf, sizeof buf) << std::endl;
    if (0 == chdir(homeDir)) {
      //std::cout << "CWD changed to: " << getcwd(buf, sizeof buf) << std::endl;
      setenv("PWD",homeDir,1);
    }
    else{
        //std::cout << "failed to change dir"<<endl;
    }
}

int main ()
{
    while (true)
    { // repeat this loop until the user presses Ctrl + C
            cout<<getenv("USER")<<"@PA4:"<<getenv("PWD")<<"$ ";
            string commandline;/*get from STDIN, e.g., "ls  -la |   grep Jul  | grep . | grep .cpp" */
            getline(cin,commandline);
            commandline.erase(std::remove(commandline.begin(), commandline.end(), '\''), commandline.end());
            if(commandline.find('"')!=-1)
            {
                quotTrig = true;
                commandline.erase(std::remove(commandline.begin(), commandline.end(), '"'), commandline.end());
            }
            if(quotTrig==false)
                tparts = split (commandline, "|");
            else{
                tparts.push_back(commandline);
            }   // split the command by the "|", which tells you the pipe levels
            
            in = 0; // for each pipe, do the following:
            for (int i=0; i<tparts.size(); i++){
                pipe(fd); // make pipe
                if ((pid=fork()==0)){
                    out = fd[1];// redirect output to the next level
                    if(in != 0)
                    {
                        dup2(in,0);
                        close(in);
                    }
                    if(i<tparts.size()-1)
                    {// unless this is the last level
                        dup2(out,1);
                        close(out);
                    }
                    execute(tparts[i]); // this is where you execute // find out all the arguments, see the definition
                }else{
                    wait(0);            // wait for the child process
                    close(fd[1]);// then do other redirects
                    in = fd[0];
                }
            }
        commandline = "";
    }
    return 0;
}