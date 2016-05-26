#include <iostream>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <cstdlib>
#include <time.h> 

using namespace std;

#define user 7002040

void student_running(); //checks if student is running the program by check the real UID of process against student
void obtain_creds();//obtains permission and validates it
void sniff_check(); //checks for file in directory and makes sure the student has access to the file and etc
                    //also checks if sniff was edited a minute ago
void ownership_change(); //changes the owenership of the file

int main(void){

  student_running();
  //control flow on loggedin
  obtain_creds();
  sniff_check();
  ownership_change();
  cout << "runpriv ended " << endl;
  return(1);
}

void student_running(){
  pid_t process = getpid();
  int uid = getuid();
  string username  = getlogin();
  if(uid != user){
      cerr << "user is not logged in" << endl;
      exit(EXIT_FAILURE);
  } else {
      return;
  }
}//end of student running


void obtain_creds(){
//  cout << "obtain creds" << endl;
  int status;
  pid_t pid;
  char* av[] = {"kinit", NULL};
  char* ep[] = {"Path= ...", "SHELL=/bin/sh", "SHELL=/bin/bash", "BASH=/bin/bash","IFS=\t\n", NULL};
  switch(pid = fork()){
    case -1:
      cerr << "forking a process failed" << endl;
      exit(EXIT_FAILURE);
    case 0: //in the child process
    //  cout << "From the child" << endl;
      status = execve("/usr/bin/kinit", av, ep);
      exit(status); // only happens if execve(2) fails
    default: //in parent
       if(waitpid(pid, &status,0) < 0){
          cerr << "Process failed" << endl;
          exit(EXIT_FAILURE);
      } else {
          //check the status
            //if status == 0 => properly executed
            if(!status){
              sniff_check();
            } else {
                cerr << "failed here" << endl;
                exit(EXIT_FAILURE);
            }
            //else it did not
      }

  }


}//obtain_creds() end

void sniff_check(){
  //cout << "sniff check" << endl;
  struct stat file;
  int status;
  int uid = getuid();
  time_t now = time(NULL);
  struct tm * current = localtime(&now);
  int last_min = current->tm_min;   
  status = stat("sniff", &file);
  int last_mod = (localtime(&file.st_mtime))->tm_min;
  int last_day = current->tm_mday; 
  int last_mod_day = (localtime(&file.st_mtime))->tm_mday; 
  //cout << "last_mod" << last_mod << endl; 
  //cout << "last_min" << last_min << endl;
  //cout << "last change " << (last_mod - last_min)  << endl; 
  //cout << "last change w/o /6 " << (last_mod - last_min) << endl;
  if(status == -1){
      cerr << "file does not exsist" << endl;
      exit(EXIT_FAILURE);
  } else {
    //  file.st_mode & S_IRUSR  user has read permission
    //  file.st_mode & S_IXUSR  user has execute permission
      //not owned by the student && owner cannot execute
      if((file.st_mode & S_IXUSR) || (int(file.st_uid) != int(uid) )){
          cerr << "user does not own file or user cannot execute the file" << endl;;
          exit(EXIT_FAILURE);
      }//users access if
      //anyone else can own or do anything else themselves
      else if( ( (file.st_mode & S_IRWXO) || (file.st_mode & S_IWOTH) || (file.st_mode & S_IXOTH) || (file.st_mode & S_IROTH) ) ){
        cerr << "others have access to this file" << endl;
        exit(EXIT_FAILURE);

      } 
      //check edits in the last minute
      else if ( ((last_min - last_mod) == 1) || ((last_min - last_mod) == 0) || ((last_min - last_mod) == -1) ){
		if(last_mod_day == last_day){
			cerr << "file was modified a minute ago" << endl; 
			exit(EXIT_FAILURE); 
		} 
      } 	
	//check last time modified
     }//end of others if


  //}//checking stat s


}//sniff_check() end


//change ownership of sniff
void ownership_change(){
  pid_t pid; 
  int status; 	
  char* ep[] = {"Path= ...", "SHELL=/bin/sh", "SHELL=/bin/bash", "BASH=/bin/bash","IFS=\t\n", NULL};
  char* av[] = {"chown","root:95","sniff", NULL}; //change to proj before turning in
  switch(pid = fork()){
    case -1:
      cerr << "forking chown process failed" << endl;
      exit(EXIT_FAILURE);
    case 0: //in the child process
      status = execve("/usr/bin/chown", av, ep);
      //status = 0;
      exit(status); // only happens if execve(2) fails
    default: //in parent
       if(waitpid(pid, &status,0) < 0){
          cerr << "Process failed" << endl;
          exit(EXIT_FAILURE);
      } else {
          //check the status
           //if status == 0 => properly executed
          if(!status){
		int error = chmod("sniff", 04550);
		if(error){
			cerr << "could not change permissions for root" << endl;
			exit(EXIT_FAILURE); 
		}  else {
			cout << "sniff owner and permissions changed" << endl;
            	}
      
  	} else {
			cerr << "chown did not work" << endl;
			exit(EXIT_FAILURE); 
		
	} // end of status check
	
    } //end of wait 

  }//end of fork

}//end of ownership 


