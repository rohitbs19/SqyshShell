#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>	
	//the struct to store the background processes	
	typedef struct pidName{
		// holds a pid 
		pid_t pid;
		// and cmdName
		char * cmdName;
	} pidName;
	//global instance of the struct
	pidName * tempVar;
	//counts the number of background jobs present
	int static itr=0,pidNameSize=1;
	// contains the outputFile for redirection
	char *  outputFile;
	// contains the inputFile for redirection
	char *  inputFile;
	int static presentAmp=0;
	int static tempstar=0;

void exitFree(FILE * fp){

	for(int i=0; i<itr; i++){
        	free(tempVar[i].cmdName);
        }
        free(tempVar);
	tempVar = NULL;
	if(fp!=NULL)
	fclose(fp);
	fp=NULL;
        exit(0);
}
void cd_call(char *path){
        char * home;
        // handles the case where an absense of an arg for cd cmd 
        // should take you to the home dir
        if(path==NULL){
                home = getenv("HOME");
		if(home == NULL){
			fprintf(stderr, "getenv: no match found for HOME\n");
		}
        }
        else
        home = path;
        // chdir is a wrapper for system call which changes your current dir
        if(chdir(home)!=0){
                fprintf(stderr,"cd: %s: %s\n", path, strerror(errno));
        }

}

char * readLine(){
	int flag=0;
  	int    numwords = 257;
	char * element1;
	char * temp;
	char * element2;
	// reads the lines and stores them here
	char * buf = (char *)malloc(sizeof(char)* numwords);
	if(buf==NULL){
		fprintf(stderr,"malloc failed");
	}
	//loops and removes the leading white space and and returns the line
 	if(fgets(buf,numwords, stdin)!=NULL){
		element1 = strtok(buf, "\n");
		if(element1!=NULL)
		if(isblank(element1[0]) || isspace(element1[0])){
			flag =1;
			// freeing purpose
			temp = element1;
			// helps remove the leading whitespace
			while(isspace((unsigned char)*element1)) element1++;
			if(element1==NULL){
				flag=0;
			} 
			if(flag!=0)
			element2 = (char *)malloc(sizeof(element1)+1);
			strcpy(element2,element1);
			 element1 = temp;
                        temp = NULL;

		}
	}
	else{
		free(buf);
		exitFree(NULL);
	}
	//frees the buffer if there is an empty command line
	if(element1==NULL){
		free(buf);	
	}
	if(flag==1)
	free(buf);
	if(flag==1)
	return element2;  
	else
	return element1;
}

char * readInputFromFile(FILE * fp){
	int    numwords = 257, flag =0;
        char * element1;
        char * buf = (char *)malloc(sizeof(char)* numwords);
	 if(buf==NULL){
                fprintf(stderr,"malloc failed");
        }

	char * element2;
	char * temp;
	// similar to the above case but takes input from a file mentioned
        if(fgets(buf,numwords, fp)!=NULL){
                element1 = strtok(buf, "\n");
	 if(element1!=NULL)
                if(isblank(element1[0]) || isspace(element1[0])){
                        flag =1;
                     temp = element1;
                        while(isspace((unsigned char)*element1)) element1++;   
			 if(element1==NULL){
                                flag=0;
                        }
                        if(flag!=0)
                        element2 = (char *)malloc(sizeof(element1)+1);
                        if(flag!=0)
                        strcpy(element2,element1);
			 element1 = temp;
                        temp = NULL;

                }
        }
        else{	
		free(buf);
        	exitFree(fp);
	}

        if(element1==NULL){
                free(buf);
        }
        if(flag==1)
        free(buf);
        if(flag==1)
        return element2;
        else
        return element1;
}


char ** parse(char * line){
        //this tokenizes uptil the first occurence of the space " " and uses this as the
          char * word;
	if(line!=NULL)
	word  = strtok(line," " );
        //for holding the command at the 0 element and subsequent args
	int numWords =257;
        char **wordArray;
	
	wordArray =  (char **)calloc(numWords,sizeof(char*));
	 if(wordArray==NULL){
                fprintf(stderr,"calloc failed");
        }
        // holds the command to execute
	if(wordArray!=NULL)
	wordArray[0] = word;
	++tempstar;
	// iterating var
        int tempInt=0, tempInt2=0, imp1=0, imp2=0,j=1;
while(word!=NULL){
	
		word = strtok(NULL," " );
                if(word!=NULL){
                	if((strcmp(word,"<")!=0 && strcmp(word, ">")!=0 && strcmp(word, "&")!=0) &&( tempInt!=1 && tempInt2!=1)){
					wordArray[tempstar] = word;
					++tempstar;
                	}	
                        if(strcmp(word,">")==0 || tempInt==1 ){
                                // takes the name of the ouput file
                                if(tempInt==1 && imp2!=2){
                                        outputFile = word;
                                        imp2=2;
                                }
                                 if(strcmp(word, ">")==0){
                                        tempInt=1;
					 				
				}
                        }
                        // similar case but takes the input file instead of an outputfile
                        if(strcmp(word,"<")==0 || tempInt2==1 ){
                                if(tempInt2==1 && imp1!=2){

                                        inputFile = word;
                                        imp1=2;
                                }
                                if(strcmp(word, "<")==0 ){
					
						                                      
					 tempInt2=1;
                                }
                        }// background process
                        if(strcmp(word,"&")==0){
				
                                presentAmp=1;
				
                        }
                        j++;
                }
        }
	//returns the commands and the args
	return wordArray;   

}

void cmdExcec(char ** word,  int pidNameSize){
	// as file descriptor
	int file2=0;
	//for the pid and fork call
	pid_t pid;
	//wait call
        int status;
        pid = fork();
	// see the presence of a background task and updates the pidNameSize accordingly
	if(presentAmp){
	if(itr>=pidNameSize)	
		pidNameSize = itr;
	++pidNameSize;	
	tempVar =(pidName *)realloc(tempVar,sizeof(pidName)*pidNameSize);
	if(tempVar==NULL){
		fprintf(stderr, "realloc failed\n");
	}
	}
	//for an edge case
	if(word==NULL){
	return;
	}
        if(pid==0){
		// to make the process be in the background itself
                if(presentAmp){
                        setpgid(0,0);
                }
		//input redirection 
                if(inputFile!=NULL)
                        file2 = open(inputFile, O_RDONLY);
                        dup2(file2,0);
			close(file2);
                if(outputFile!=NULL){
                        int file = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR);
                        dup2(file,1);
			 close(file);
                }
		
                if(execvp(word[0], word )==-1){
                        fprintf(stderr, "%s: %s\n", word[0], strerror(errno));
                        exit(1);
                }
        }
	else if(pid<0){
		//if fork fails
		perror("fork failed : ");
	}
        else{
                if(presentAmp!=1){
                        do {
                                 waitpid(pid, &status, WUNTRACED);
                        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                }
                else{
			tempVar[itr].pid = pid;
	                tempVar[itr].cmdName = (char *)malloc(sizeof(char)* strlen(word[0])+1);	
			if( tempVar[itr].cmdName==NULL){
				fprintf(stderr,"malloc failed\n");	
			}	
			strcpy(tempVar[itr].cmdName, word[0]);
			++itr;
                }
        }


}

int main(int argc, char** argv){
	
	int status, pid;

	int filePresent=0,prompt=0,flag=0;

	
	int flagnul=0;

	// checks the command line arguments and isatty function to decide 
	// to take input from and print a prompt or not
	if(argc==1 && isatty(0)){
		prompt =1;
	}	
	else if(argc==1 && !isatty(0)){
		prompt =0;
	}
	else if(argc==2){
		filePresent =1;
	}
	else {
		fprintf(stderr,"Too many commandLine Args\n");
		exit(1);
	}
	if(filePresent==0){
	
	while(1){


		 tempstar =0;
		 char * cmdname;
       	 	 char * line=NULL;
        	 char ** words=NULL;
		 inputFile = NULL;
		 outputFile = NULL;
		 flagnul = 1;
		 flag=0;
		 presentAmp=0;

			pid =   waitpid(-1,&status, WNOHANG);
			while(pid>0){
				for(int i=0; i<itr; i++){
					if(tempVar[i].pid == pid){
						cmdname = tempVar[i].cmdName;
					}
				}
                                 int exitStatus = WEXITSTATUS(status);
                                 fprintf(stderr, "[%s (%d) completed with status %d]\n",cmdname, pid, exitStatus);
				pid = waitpid(-1,&status, WNOHANG);
                        }
			if(prompt)
				printf("sqysh$ ");
			
			
			line = readLine();	
			if(line==NULL ){
		        	flagnul=0;
			}
			
			if(flagnul) 
				words = parse(line);
		 	
		if(flagnul){	
			if(words[0]!=NULL && strcmp(words[0],"cd")==0){			
				
				flag =1;
				
				if(words[2]!=NULL){
					fprintf(stderr, "cd: too many arguments\n");
				}	
				cd_call(words[1]);
			}
			
			 else if(flagnul && words[0]!=NULL && strcmp(words[0],"exit")==0){
                                flag=1;
				for(int i=0; i<itr; i++){
				  	free(tempVar[i].cmdName);
				}
				for(int i =0; i<tempstar -1 ; i++){
                                	free(words[i]);
                                }
	    			free(tempVar);
				free(words);
				free(line);
                                exit(0);
	                }
			else if (flagnul && words[0]!=NULL && strcmp(words[0],"pwd")==0){
				flag=1;
   				 char cwd[1024]; 
				if (getcwd(cwd, sizeof(cwd)) != NULL)
       				 fprintf(stdout, "%s\n", cwd);
   				 else
       				 perror("getcwd() error");
				
			}
	
			if(words[0]!=NULL && flag!=1 && flagnul){
				cmdExcec(words, pidNameSize);
			}
		}	
			if(flagnul)
				free(words[0]);
				
			if(flagnul)
				free(words);
		}
		
	}
	else if(filePresent==1){
		FILE * fp;
		fp = fopen(argv[1],"r");
		
		if(fp==NULL){
			fprintf(stderr, "Invalid file\n");
			exit(1);
		}
		while(1){
			presentAmp=0;
			tempstar =0;
			char * line1;
			char ** words1;
			char * cmdname;
			int flag1=0;
			
			pid =   waitpid(-1,&status, WNOHANG);


                        while(pid>0){
					
                        	for(int i=0; i<itr; i++){
			                  if(tempVar[i].pid == pid){
                                          	cmdname = tempVar[i].cmdName;
                                          }
                                }
				int exitStatus = WEXITSTATUS(status);
                                fprintf(stderr, "[%s (%d) completed with status %d]\n",cmdname, pid, exitStatus);
				 pid =   waitpid(-1,&status, WNOHANG);

			}
			
			line1 =	readInputFromFile(fp);
			int flagnul1=1;
			
			if(line1==NULL){
				flagnul1 = 0;
			}	
			if(flagnul1)
			words1 = parse(line1);
			if(flagnul1) 	
			if(words1!=NULL ){
			if(words1[0]!=NULL && strcmp(words1[0],"cd")==0){
                                flag1 =1;
                        
                                if(words1[2]!=NULL){
                                        fprintf(stderr, "cd: too many arguments\n");
                                }
                        
                                        cd_call(words1[1]);
                  	}	

                	else if((flagnul1 && words1[0]!=NULL && strcmp(words1[0],"exit")==0)){
                                flag1=1;
                        	for(int i=0; i<itr; i++){
                                        free(tempVar[i].cmdName);
                                }
                                for(int i =0; i<tempstar -1 ; i++){
                                          free(words1[i]);
                                }
                                free(tempVar);
                                free(words1);
                                free(line1); 
				fclose(fp);
			        exit(0);
                 	}	
			 else if (flagnul1 && words1[0]!=NULL && strcmp(words1[0],"pwd")==0){
                                flag1=1;
                                char cwd[1024]; 
				if (getcwd(cwd, sizeof(cwd)) != NULL)
                                 fprintf(stdout, "%s\n", cwd);
                                 else   
                                 perror("getcwd() error");

                        }
                 	if(flagnul1 && words1[0]!=NULL && flag1!=1){
                                cmdExcec(words1,pidNameSize);
                  	}	
			if(flagnul1)
				free(words1[0]);
			if(flagnul1)
				free(words1);
			}
			

		}
	
	}


}
