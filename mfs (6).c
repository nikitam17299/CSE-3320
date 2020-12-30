/*
Student Name: Nikita Ashok Menon
University ID: 1001548454
NET ID: nam8454
Student Name: Tasnia Afroz Mahmud
University ID: 1001121807
NET ID: txm1807
CSE 3320 - OPERATING SYSTEMS
PROFFESOR - TREVOR BAKKER
*/


//Required header files
#define _GNU_SOURCE

#include<stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>


//declaration and initialization of global variables

FILE *fp;
int flag_open =0;
int flag_close =0;
char BS_OEMName[8];
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors =0;
int32_t FirstDataSector =0;
int32_t FirstSectorofCluster;
int root_offset;
int byte_per_cluster;
int FAT_size;

struct __attribute__((__packed__)) DirectoryEntry {
	char DIR_Name[11];
	uint8_t DIR_Attr;
	uint8_t Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t Unused2[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];
char format_name[16][12];
char* format_token;
char* filename;
char *ext;
char temp_name[16][12];
int address;
char read_in[1];

//returns the value of the address for that block of data
//finds that starting address of a block of data given the sector number
int LBAToOffset(int32_t sector)
{
	return (( sector - 2 ) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

//Given a logical block address, look up into the first FAT and return the logical block address of the
//block in the file. If  there is no further blocks then return -1
int16_t NextLB ( uint32_t sector )
{
	uint32_t FATAddress = ( BPB_BytesPerSec * BPB_RsvdSecCnt ) + ( sector * 4 );
	int16_t val;
    fseek( fp, FATAddress, SEEK_SET );
    fread ( &val, 2, 1, fp );
	return val;
}

int new_address=0;
int count_c = 0;
int count_l;

//command functions
void cmd_info();
void cmd_stat(char* filename);
void cmd_ls();


//function for listing what's in directory
void cmd_ls()
{
    int i;
    //going through the 16 different sets of 32 bytes
    printf("\n");
    for(i=0;i<16;i++)
    {
        //filtering so that Only show is attribute is 0x01, 0x10, or 0x20
       if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20  )
       {
            //filtering out the formatted name
           if((format_name[i][0] >='a' && format_name[i][0] <='z') || (format_name[i][0] >= 'A' && format_name[i][0]  <='Z'))
        {
           char formatted[12];
            formatted[11] = '\0';

            memcpy(formatted,dir[i].DIR_Name,11);
            printf("%s\n",formatted);
           }
       }
    }
    printf("\n");
}

//for the command info which prints information about the file system
void cmd_info()
{
    //printing information in decimal
    printf("\nINFO IN BASE 10\n");
    printf("BPB_BytesPerSec: %d\n",BPB_BytesPerSec);
    printf("BPB_SecPerClus: %d\n",BPB_SecPerClus);
    printf("BPB_RsvdSecCnt: %d\n",BPB_RsvdSecCnt);
    printf("BPB_NUMFats: %d\n",BPB_NumFATs);
    printf("BPB_FATSz32: %d\n",BPB_FATSz32);

    //printing information in hexadecimal
    printf("\n\n\nINFO IN HEXADECIMAL\n");
    printf("BPB_BytesPerSec: %x\n", BPB_BytesPerSec);
    printf("BPB_SecPerClus: %x\n",BPB_SecPerClus);
    printf("BPB_RsvdSecCnt: %x\n",BPB_RsvdSecCnt);
    printf("BPB_NUMFats: %x\n",BPB_NumFATs);
    printf("BPB_FATSz32: %x\n\n\n",BPB_FATSz32);

}


int main(int argc ,char **argv)
{
    //FROM PREVIOUS CODE [FIRST ASSIGNMENT MSH]
    //PROFESSORS CODE [TO GET INPUT AND STORE INTO TOKEN ARRAY]
//	int chk = 0;
///	int c_show = 0;
//	char *ChckRtDir = (char*) malloc(12);
//	int leave = 1;

	char * cmd_str = (char*) malloc(255);
	bool h_command = false;

	while(1)
    {

        int chk_cd = 0;
        if(!h_command)
        {
            count_l=0;

            //prompts will all print out mfs.
            printf("mfs> ");
            while(!fgets(cmd_str,255, stdin));
        }

        char* cmd_in = strdup(cmd_str);
        cmd_in[strcspn(cmd_in,"\r\n")]=0;

        char *token[10];
        //user inputs will be stored into token
        int token_count = 0;
        char *arg_ptr;
        char *working_str = strdup(cmd_str);
        char* working_root = working_str;

        while(((arg_ptr = strsep(&working_str," \t\n"))!=NULL)&&(token_count<10))
        {
            token[token_count] = strndup(arg_ptr,255);

            if (strlen(token[token_count])==0)
            {
                token[token_count]=NULL;
            }

            token_count++;
        }

        if(token[0]==NULL)
        {
            continue;
        }

        //if user inputs quit of exit then it will exit out of mfs
        char *command = token[0];
            if(strcmp(command,"quit")==0||strcmp(command,"exit") == 0 )
        {
            free(working_root);
            break;
        }

        //this will open the file
        if(strcmp("open",token[0])==0)
        {
            //flags that make sure the file can be worked with
            if(flag_open == 1)
            {
                printf("File already open\n");
                continue;
            }

            else if(token[1] == NULL)
            {
                printf("Please enter filename\n");
                continue;
            }

            else if((fopen(token[1],"r") == NULL))
            {
                printf("Error: File not found\n");
                continue;
            }

            //if the file is open, then we will go ahead and store important
            //values of the img into variables that will be used later on
            //in order to do things. Like giving us the proper value to
            //seek to.
            else
            {
                fp = fopen(token[1],"r");
                flag_open = 1;
                flag_close =0;
                fseek(fp,3,SEEK_CUR);
                fread(&BS_OEMName,1,8,fp);
                fread(&BPB_BytesPerSec,1,2,fp);
                fread(&BPB_SecPerClus,1,1,fp);
                fread(&BPB_RsvdSecCnt,1,2,fp);
                fread(&BPB_NumFATs,1,1,fp);
                fread(&BPB_RootEntCnt,1,2,fp);
                fseek(fp,36,SEEK_SET);
                fread(&BPB_FATSz32,1,4,fp);
                fseek(fp,44,SEEK_SET);
                fread(&BPB_RootClus,1, 4,fp);
                fseek(fp,71,SEEK_SET);
                fread(&BS_VolLab,1,11,fp);

                root_offset = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt*BPB_BytesPerSec);
                //this is the starting address of the 16 32 bytes

                //moving to where the different files are
                fseek(fp,root_offset,SEEK_SET);
                fread(dir,32*16,1,fp);
                int i;
                for(i=0;i<16;i++)
                {
                    //going through the 16 'files' and figuring out which are valid
                    if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
                    {
                        memcpy(format_name[i],dir[i].DIR_Name,11);
                        format_name[i][11]='\0';
                        memcpy(temp_name[i],format_name[i],12);

                    }

                }
                continue;
            }

        }

        //closing the file after checking that there is one opened
        if(strcmp(command,"close")==0)
        {
            //error checking
            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
            }

            if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
            }

            //closing file and notifying user about it
            else if(fp!=NULL)
            {
                flag_close = 1;
                flag_open = 0 ;
                fclose(fp);
                printf("\nFile has been closed\n\n");
            }
            continue;
        }

        //when user has inputted into
        if(strcmp(command,"info")==0)
        {
            //checking to see there is a file opened to give give info of
            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
                continue;
            }

            if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
                continue;
            }

            if(fp == NULL)
            {
                printf("Empty file\n");
            }

            //going to the command info that will print out all the info
            else if(fp!=NULL)
            {
                cmd_info();
                continue;
            }
            continue;
        }

        //if cd is inputted by the user
        if(strcmp(command,"cd")==0)
        {
        int i=0;

        //going through the 16, and putting them into the right format
        for(i=0;i<16;i++)
        {
            filename = strtok(temp_name[i]," \t");
            ext = strtok(NULL," \t");

            if(ext!=NULL)
            {
                char text[] = ".";
                strcat(text,ext);
                strcat(filename,text);
            }

            if(filename!=NULL)
            {
                strcpy(temp_name[i],filename);
            }

            char capital_name[16];
            strcpy(capital_name,token[1]);
            char *txt = capital_name;

            while(*txt)
            {
                *txt = toupper(*txt);
                txt++;
            }

            //comparing input to the names of file/directory in loop
            if(strcmp(capital_name,temp_name[i])==0)
            {
                int new_address = 0;
                //if it matches then set as og address
                if(dir[i].DIR_FirstClusterLow ==0)
                {
                    new_address = root_offset;
                }

                else
                {
                    new_address = LBAToOffset(dir[i].DIR_FirstClusterLow);
                }

                fseek(fp,new_address,SEEK_SET);


                for(i=0;i<16;i++)
                {
                    fread(&dir[i],sizeof(dir[i]),1,fp);
                }

                //storing the info from directory
                for(i=0;i<16;i++)
                {
                    memcpy(format_name[i],dir[i].DIR_Name,11);
                    format_name[i][11] = '\0';
                    memcpy(temp_name[i], format_name[i],12);
                }

            }


        }

        continue;
    }

    //when stat is inputted by the user
    if(strcmp(command,"stat")==0)
    {
        //checking for any possible file issues
        if(flag_open == 0 )
        {
            printf("Error: File not open\n");
            continue;
        }

        if(flag_close == 1)
        {
            printf("ERROR: FILE ALREADY CLOSED\n");
            continue;
        }

        if (token[1]==NULL)
        {
            printf("ERROR: Missing information.\nMust enter the filename or directory name.\n\n");
        }

        if(fp == NULL)
        {
            printf("Empty file\n");
        }

        else if (fp!= NULL )
        {
            int i=0;
            for(i=0;i<16;i++)
        {

        //filename and temp_name are going to be compared in order to find if
        //user input matches the file
        filename = strtok(temp_name[i]," \t");
        ext = strtok(NULL," \t");

        if(ext!=NULL)
        {
            char text[] = ".";
            strcat(text,ext);
            strcat(filename,text);
        }

        //formatting
        if(filename!=NULL)
        {
            strcpy(temp_name[i],filename);
        }

            char capital_name[16];
            strcpy(capital_name,token[1]);
            char *txt = capital_name;

            //converting to uppercase
            while(*txt)
            {
            *txt = toupper(*txt);
            txt++;
            }

            if(token[1] == NULL)
            {
            printf("Error:File not found");
            }

            //if the user input matches a file name then it will output
            //the attribute and other info.
            if(strcmp(capital_name,temp_name[i])==0)
            {
                printf("\nAttribute: %d\n", dir[i].DIR_Attr);
                printf("Cluster Number :%d\n",dir[i].DIR_FirstClusterLow);
                printf("DIR_Name: %d\n",dir[i].DIR_Name);
                printf("FIle Size: %d\n\n",dir[i].DIR_FileSize);
            }

            }

                continue;
            }
        }

        //when ls is inputted by user, list out what is in directory
        if(strcmp(command,"ls")==0) {
            //checking for any possible file issues
            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
                continue;
            }

            if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
                continue;
            }

            if(fp == NULL)
            {
                printf("Empty file\n");
            }

            //if no issues with file than go ahead to the ls command
            else if(fp!=NULL)
            {
                cmd_ls();
                continue;
            }
        }



        //if user inputs get, then the file entered will be put into directory
        if(strcmp(command,"get")==0)
        {
            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
                continue;
            }

            if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
                continue;
            }

            if(fp == NULL)
            {
                printf("Empty file\n");
            }

            else if (fp!=NULL)
            {
            int found = 0;
            int i=0;

            //going through the 16 'files'
            for(i=0;i<16;i++)
                {
                filename = strtok(temp_name[i]," \t");
                ext = strtok(NULL," \t");

                //getting rid of excess stuff in name
                if(ext!=NULL)
                {
                    char text[] = ".";
                    strcat(text,ext);
                    strcat(filename,text);
                }

            if(filename!=NULL)
            {
                strcpy(temp_name[i],filename);
            }
            //to help with formatting so that it can be compared
            char capital_name[16];
            strcpy(capital_name,token[1]);

            char *txt = capital_name;

            while(*txt)
            {
              *txt= toupper(*txt);
               txt++;
            }

            //comparing the files to what is inputted
            if(strcmp(capital_name,temp_name[i])==0)
            {

            int cluster = dir[i].DIR_FirstClusterLow;
            int size = dir[i].DIR_FileSize;
            int address = LBAToOffset(cluster);
            fseek(fp,address,SEEK_SET);

            FILE *ofp = fopen(capital_name,"w");

            int j;
            for(j=0;j<size;j++)
            {
                fread(&read_in,1,1,fp);
                fprintf(ofp,"%c",read_in[0]);
            }

        fclose(ofp);

        }
        }

//for(i=0;i<16;i++)
//{
//	if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr = 0x10 || dir[i].DIR_Attr == 0x20)
//	{
//		memcpy(format_name[i],dir[i].DIR_Name,11);
//		format_name[i][11] = '\0';
//		memcpy(tem
//	}
	
//}
        printf("\n");
        printf("File successfully retrieved and placed into current working directory");
        printf("\n\n");

        continue;
        }
        }


 if(strcmp(command,"put")==0)
        {
            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
                continue;
            }

            if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
                continue;
            }

            if(fp == NULL)
            {
                printf("Empty file\n");
            }

            else if (fp!=NULL)
            {
            int found = 0;
            int i=0;

            //going through the 16 'files'
            for(i=0;i<16;i++)
                {
                filename = strtok(temp_name[i]," \t");
                ext = strtok(NULL," \t");

                //getting rid of excess stuff in name
                if(ext!=NULL)
                {
                    char text[] = ".";
                    strcat(text,ext);
                    strcat(filename,text);
                }

            if(filename!=NULL)
            {
                strcpy(temp_name[i],filename);
            }
            //to help with formatting so that it can be compared
            char capital_name[16];
            strcpy(capital_name,token[1]);

            char *txt = capital_name;

            while(*txt)
            {
              *txt= toupper(*txt);
               txt++;
            }

//printf("temp_name[i] %s",temp_name[i]);
            //comparing the files to what is inputted
            //if(strcmp(capital_name[0] == )==0)
            
if(dir[i].DIR_Name[0] == (char) 0xE5 || dir[i].DIR_Name[0] == (char) 0x00 || dir[i].DIR_Name[0] == (char) 0x05)
	{
	//	char formatted[12];
	//	formatted[11] = '\0';
//	memcpy(format_name[i],dir[i].DIR_Name,11);
//	format_name[i][11] = '\0';
	
strcpy(dir[i].DIR_Name,capital_name);
//	printf("dir_name strcpy %s",dir[i].DIR_Name);
	   dir[i].DIR_Attr = 1;
	//	dir[i].DIR_Name = capital_name;
	//strcpy(dir[i].DIR_Name = capital_name;
//	printf("Filname %s\n",dir[i].DIR_Name);
            int cluster = dir[i].DIR_FirstClusterLow;
  //         printf("clsuter %d",cluster);
	//	int size = dir[i].DIR_FileSize;
	//	fseek(ofp,0L,SEEK_END);
	//	int size = ftell(ofp);
	//	dir[i].DIR_FileSize = size;
	//	rewind(ofp);
            int address = LBAToOffset(cluster);
            fseek(fp,address,SEEK_SET);
		
            FILE *ofp = fopen(token[1],"r");
//printf("Capital name %s\n",capital_name);
	fseek(ofp,0L,SEEK_END);
        int size = ftell(ofp);
	dir[i].DIR_FileSize = size;  
	rewind(ofp); 
	 int j;
            for(j=0;j<size;j++)
            {
                fread(&read_in,1,1,ofp);
                fprintf(fp,"%c",read_in[0]);
//		printf("%c",read_in[0]);
            }
	break;
        fclose(ofp);

        }
        }

for(i=0;i<16;i++)
{

if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)

{

memcpy(format_name[i],dir[i].DIR_Name,11);
format_name[i][11]='\0';
} 
}
        printf("\n");
       printf("\nFile retrieved successfully and placed into FAT32 image.\n\n");

        printf("\n\n");

        continue;
        }
        }

        //if user inputs put then we will be putting file in directory
        /*if(strcmp(command,"put")==0)
        {
            //checking to see if file is opened and can be worked with
            if (token[1]==NULL)
            {
                printf("ERROR: Missing information.\nMust enter the filename.\n\n");
            }

            //file checking, insuring there are no issues
            if(fp == NULL)
            {
                printf("Error: File not opened");
            }
            char temp[12];
            strcpy(temp,token[1]);

            //opening a new file
            FILE *ofp = fopen(temp,"r");
            //checking the new file
            if(ofp == NULL)
            {
                printf("Error: File empty");
            }

            //seeking to the new address
            fseek(fp,new_address,SEEK_SET);
            fread(&dir,16,32,fp);

            char formatname[12];
            memset(formatname,' ',12);
            char *tok = strtok(token[1],".");
            strncpy(formatname,tok,strlen(tok));
            tok = strtok(NULL,".");

            //name without all formatting corrections
            if(tok)
            {
                strncpy((char*) (formatname+8),tok,strlen(tok));
            }

            formatname[11] = '\0';
            int i;

            //formatting to uppercase, to aid in comparing
            for(i=0;i<11;i++)
            {
                formatname[i] = toupper(formatname[i]);
            }
            for(i=0;i<16;i++)
            {
                //this is where we are putting it into the fat32 image file.
                if(dir[i].DIR_Name[0] == (char) 0xE5 || dir[i].DIR_Name[0] == (char) 0x00 || dir[i].DIR_Name[0] == (char) 0x05)
                {
                    strcpy(dir[i].DIR_Name,formatname);
                    dir[i].DIR_Attr = 1;
                    int cluster = dir[i].DIR_FirstClusterLow;

                    char *buffer;
                    fseek(ofp,0L,SEEK_END);
                    long size = ftell(ofp);
                    dir[i].DIR_FileSize = size;
                    rewind(ofp);
                    fseek(fp,new_address,SEEK_SET);
                    fwrite(&dir,16,32,fp);
                    buffer = (char*) calloc(1,512);
                    if(!buffer)
                    {
                        fclose(ofp);
                    }
                    //working on copying the file into fat32 image
                    fread(buffer,512,1,ofp);
                    int addr = LBAToOffset(cluster);
                    fseek(fp,addr,SEEK_SET);
                    fwrite(buffer,512,1,fp);
                    size = size -512;

                    while(size>0)
                    {
                        cluster = NextLB(cluster);
                        addr = LBAToOffset(cluster);
                        fseek(fp,addr,SEEK_SET);
                        fread(buffer,512,1,ofp);
                        fwrite(buffer,512,1,fp);
                        size = size -512;
                    }
                    //after we are done placing the file, we may close it
                    fclose(ofp);
                    free(buffer);
                    break;
                }
            }

            ///	for(i=0;i<16;i++)
            ///		filename = strtok(temp_name[i]," \t");
            ///fseek(fp,address,SEEK_SET);

            printf("\nFile retrieved and placed into your FAT32 image.\n\n");

            continue;
        }*/

        //if read is inputted by the user then it
        if(strcmp(command,"read")==0)
        {

            if(flag_open == 0 )
            {
                printf("Error: File not open\n");
                continue;
            }

            else if(flag_close == 1)
            {
                printf("ERROR: FILE ALREADY CLOSED\n");
                continue;
            }

            else if(token[1] == NULL||token[2]==NULL||token[3]==NULL)
            {
                printf("\nERROR: Missing information!\n");
                printf("Make sure you have inputted filename, position & number of bytes.\n\n");
            }

            else if(fp == NULL)
            {
                printf("Empty file\n");
            }

            else if (fp!=NULL)
            {
            int i;

            //going through the 16 clusters of 32 bytes
            for(i=0;i<16;i++)
            {

            //will be used to compare the filenames to what is inputted
            filename = strtok(temp_name[i]," \t");
            ext = strtok(NULL," \t");

            //formatting the files, getting rid of excess stuff
            if(ext!=NULL)
            {
                char text[] = ".";
                strcat(text,ext);
                strcat(filename,text);
            }

            if(filename!=NULL)
            {
                strcpy(temp_name[i],filename);
            }

            //converting to uppercase in order to compare
            char capital_name[16];
            strcpy(capital_name,token[1]);
            char *txt = capital_name;

            while(*txt)
            {
                *txt = toupper(*txt);
                txt++;
            }

            //if the filename that the user inputted is the same
            if (strcmp(capital_name,temp_name[i])==0)
            {
                address = LBAToOffset(dir[i].DIR_FirstClusterLow);

                if(token[2]!=NULL)
                {
                    //seeking to the location specified
                    fseek(fp,address+atoi(token[2]),SEEK_SET);
                }

                int i;
                //prints out the number of bytes requested at the specific position
                for(i=0;i<atoi(token[3]);i++)
                {
                    fread(&read_in,1,1,fp);
                    printf("%d\n",read_in[0]);
                }

            }
        }
        }

        continue;
        }

        free(working_root);
        return 0;
    }
}
























