/*******************************************************************************
* Mingzhe Huang
* Jiatong Ruan
* CSCI-4061, Assignment 3
* 03/14/2017
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "mini_filesystem.h"

/*******************************************************************************
Declaration of the lower level functions that not exposerd to user;
These functions are used to modifu the filesystem structure.
*******************************************************************************/
int Search_Directory(char* filename);
/*Search through the directory structure for the given filename, return Inode
number of the file, if the file is found and error (-1) if it is not.*/
int Add_to_Directory(char* filename, int inode_number);
/*Add an entry to the directory structure with the ​ filename and ​inode_number
provided as input. Return the ​status indicating whether it was able to add the
entry to the directory successfully or not.*/
Inode Inode_Read(int inode_number);
/*For the given ​ inode_number​ , if the file exists, return the ​ Inode structure
 or-1 if the file doesn’t exist.*/
int Inode_Write(int inode_number, Inode input_inode);
/*Copy the contents of Inode structure ​ input_inode to the Inode present at the
inode_number​ .*/
int Block_Read(int block_number, int num_bytes, char* to_read);
/*Read the given ​ num_bytes from the ​ block_number and write it to the provided
character String ​ to_read​ ; return the number of bytes read.*/
int Block_Write(int block_number, int num_bytes, char* to_write);
/* Given the ​ block_number​ , write the contents of the string ​
to_write to the block and return the number of bytes written.*/
Super_block Superblock_read();
// Return the superblock structure.
int Superblock_Write(Super_block input_superblock);
// Copy the contents of ​ input_superblock​ to the superblock structure.

int directory_structure_index;

/*******************************************************************************
helper functions
*******************************************************************************/
char* getMicrotime(){
	time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	return asctime(timeinfo);

}


/*******************************************************************************
lower level functions
*******************************************************************************/
int Search_Directory(char* filename){
	int i;
	for (i = 0; i < MAXFILES; i ++){
    // if the file is found in the directory, return Inode_Number;
    // otherwise return -1;
		if (strcmp(Directory_Structure[i].Filename, filename) == 0){
			Count ++;
			return Directory_Structure[i].Inode_Number;
		}
	}
	Count ++;
	return -1;
}

int Add_to_Directory(char* filename, int inode_number){
	Directory directory;
	if (strlen(filename) > 20 || inode_number >= MAXFILES){
		Count ++;
		return -1;
	}
	strcpy(directory.Filename, filename);
	directory.Inode_Number = inode_number;
	if (directory_structure_index >= MAXFILES){
		Count ++;
		return -1;
	}

	Directory_Structure[directory_structure_index] = directory;
	directory_structure_index++;
	Count++;
	return 0;
}

Inode Inode_Read(int inode_number){
	if( inode_number >= MAXFILES){
		Inode err;
		err.Inode_Number = -1;
		return err;
	}
	FILE *log_f = fopen(Log_Filename, "a");
	fprintf("%s\n", getMicrotime());
	fprintf(log_f,"%s \t atcion: Inode Read\n", getMicrotime());
	fclose(log_f);

	Count ++;
	return Inode_List[inode_number];
}


int Inode_Write(int inode_number, Inode input_inode){
	if (inode_number >= MAXFILES){
		Count ++;
		return -1;
	}
	Inode_List[inode_number] = input_inode;
	FILE *log_f = fopen(Log_Filename, "a");
	fprintf(log_f,"%s \t atcion: Inode Write\n", getMicrotime());
	fclose(log_f);
	Count ++;
	return 0;
}

int Block_Read(int block_number, int num_bytes, char* to_read){
	if (num_bytes > BLOCKSIZE){
		Count ++;
		return -1;
	}
	int i;
	for (i = 0; i < num_bytes; i++){
		to_read[i] = Disk_Blocks[block_number][i];
	}
	to_read[num_bytes] = '\0';
	FILE *log_f = fopen(Log_Filename, "a");
	fprintf(log_f,"%s \t atcion: Block Read\n", getMicrotime());
	fclose(log_f);
	Count ++;
	return num_bytes;
}

int Block_Write(int block_number, int num_bytes, char* to_write){
	int to_write_length = strlen(to_write);
	if (to_write_length > BLOCKSIZE){
		to_write_length = BLOCKSIZE;
	}
	if(block_number > MAXBLOCKS){
		Count ++;
		return -1;
	}
	char* temp = strndup(to_write, to_write_length);
	Disk_Blocks[block_number] = temp;
	FILE *log_f = fopen(Log_Filename, "a");
	fprintf(log_f,"%s \t atcion: Block Write\n", getMicrotime());
//	printf("%s Inode Write\n", getMicrotime());
	fclose(log_f);
	Count ++;
	return to_write;
}


Super_block Superblock_Read(){
	Count ++;
	return Superblock;
}

int Superblock_Write(Super_block input_superblock){
	if(input_superblock.next_free_inode >= MAXFILES || input_superblock.next_free_block >= MAXBLOCKS){
		Count ++;
		return -1;
	}
	Superblock = input_superblock;
	Count ++;
	return 0;
}

/*******************************************************************************
File system Interface functions
These functions are declared as part of the API
*******************************************************************************/
/*
return 0 if the filesystem is initialized successfully;
set Inode_Number to -1 and everthing else to 0/
*/
int Initialize_Filesystem(char* log_filename){
	Log_Filename = log_filename;
	FILE *fp = fopen(log_filename, "w");
	if (fp == NULL){
		perror("Error: fopen error\n");
		return -1;
	}
	fclose(fp);
	Superblock.next_free_inode = 0;
	Superblock.next_free_block = 0;
	int i;
	for (i = 0; i < MAXFILES; i++){
		Directory_Structure[i].Inode_Number = -1;
	}
	directory_structure_index = 0;
	Count = 0;
	return 0;
}

/*
get next free inode and create an entry if it is availabe.
otherwise return error
*/
int Create_File(char* filename){
	if (Search_Directory(filename) != -1){
		return -1;
	}
	Inode current_inode;
	current_inode.Inode_Number = Superblock.next_free_inode;
	current_inode.User_Id = getuid();
	current_inode.Group_Id = getgid();
	current_inode.File_Size = 0;
	current_inode.Flag = 0;

	Inode_Write(current_inode.Inode_Number, current_inode);

	if (Add_to_Directory(filename, Superblock.next_free_inode) == -1){
		return -1;
	}
	Superblock.next_free_inode++;
	Superblock.next_free_block++;
	return 0;
}


int Open_File(char* filename){
	int inode_number = Search_Directory(filename);
	if (inode_number < 0){
		return -1;
	}
	Inode_List[inode_number].Flag = 1;
	return inode_number;
}

int Read_File(int inode_number, int offset, int count, char* to_read){
	Inode inode = Inode_Read(inode_number);
	if (inode.Inode_Number < 0){
		return -1;
	}
	if (inode.Flag == 0){
		return -1;
	}

	int char_size = count + offset;
	int j = char_size / BLOCKSIZE;
	int i, k;
	char temp_str[BLOCKSIZE];
	for (i = 0; i < j; i ++){
		Block_Read(inode.Start_Block + i, BLOCKSIZE, temp_str);
		for (k = 0; k < BLOCKSIZE; k ++){
			to_read[i*BLOCKSIZE + k] = temp_str[k];
		}
	}
	char temp_last[char_size % BLOCKSIZE];
	Block_Read(inode.Start_Block + j, char_size % BLOCKSIZE, temp_last);
	for (k = 0; k < char_size %  BLOCKSIZE; k ++){
		to_read[j*BLOCKSIZE + k] = temp_last[k];
	}
	to_read = strndup(to_read+offset, count);
	return 0;
}

int Write_File(int inode_number, int offset, char* to_write){
	Inode inode = Inode_Read(inode_number);

	if (inode.Inode_Number == -1){
		return -1;
	}
	if (inode.Flag == 0){
		return -1;
	}

	int str_len = strlen(to_write);
	char temp_str[BLOCKSIZE];
	int j = str_len / BLOCKSIZE;
	int i;
	char *remain_string = strndup(to_write+offset, str_len );

	int remain_length = str_len;

	Inode_List[inode_number].File_Size = str_len;
	Inode_List[inode_number].Start_Block = Superblock.next_free_block;
	Inode_List[inode_number].End_Block = Superblock.next_free_block + j;
	Superblock.next_free_block = Superblock.next_free_block + j + 1;

	for(i = 0; i < j; i++){
		remain_string = strndup(to_write+BLOCKSIZE * i, remain_length );
		strncpy(temp_str, remain_string, BLOCKSIZE);
		Block_Write(Inode_List[inode_number].Start_Block + i, BLOCKSIZE, remain_string);
		remain_length -= BLOCKSIZE;
	}

	remain_string = strndup(to_write+BLOCKSIZE * j, remain_length);
	Block_Write(Inode_List[inode_number].Start_Block + i, remain_length, remain_string);
	return 0;
}

int Close_File(int inode_number){
	if (inode_number < 0 || inode_number >= MAXFILES){
		return -1;
	}
	if (Inode_List[inode_number].Inode_Number == -1){
		return -1;
	}
	Inode_List[inode_number].Flag = 0;
	return 0;
}
