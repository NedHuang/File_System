/*******************************************************************************
* Mingzhe Huang
* Jiatong Ruan
* CSCI-4061, Assignment 3
* 03/14/2017
*******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <libgen.h>
#include "mini_filesystem.c"

/* Test Helper Interface */
void write_into_filesystem(char* input_directory, char *log_filename);
void make_filesystem_summary(char* filename);
void read_images_from_filesystem_and_write_to_output_directory(char* output_directory);
void generate_html_file(char* filename);

const char *get_extension(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	if(!dot || dot == filename)
		return "";
	return dot + 1;
}

char *remove_ext(const char *filename)
{
	size_t len = strlen(filename);
	char *newfilename = malloc(len-3);
	memcpy(newfilename, filename, len-4);
	newfilename[len - 4] = 0;
	return newfilename;
}

/* Global Variable */
char fulloDir[64];

/* Main function */
int main(int argc, char *argv[])
{

	// Command line arguments: <executable-name> <input_dir> <output_dir> <log_filename>
    if (argc != 4){
      printf("Usages:  <executable-name> <input_dir> <output_dir> <log_filename> \n");
      exit(1);
    } else {
      write_into_filesystem(argv[1], argv[3]);
      read_images_from_filesystem_and_write_to_output_directory(argv[2]);
			printf("Count: %d\n", Count);
    }

    return 0;
}

/* Define Helper Functions */
void write_into_filesystem(char* input_directory, char *log_filename){

	DIR *iDir = opendir(input_directory);

	if (iDir == NULL){
		printf("Error : Input directory does not exist (or is not readable).\n");
		exit (1);
	}
	closedir (iDir);

	Initialize_Filesystem(log_filename);

	char find_all_cmd[100];
	strcpy(find_all_cmd, "find ");
	strcat(find_all_cmd, input_directory);
	strcat(find_all_cmd, " -name *.jpg");

	char *out_s = (char*)malloc(sizeof(char)*512*8192);
	char *output_s= (char*)malloc(sizeof(char)*512*8192);
	FILE *cmd_out;
	FILE *inFiles = popen(find_all_cmd, "r");
	char convert_string_cmd[200];
	char image_filename[100];
	int inode_num;

	while (fgets(image_filename, 64, inFiles) != NULL)
	{
		printf("%s\n", image_filename);

		image_filename[strlen(image_filename) -1] = '\0';

		strcpy (convert_string_cmd,"base64 ");
		strcat (convert_string_cmd, image_filename);
		strcat (convert_string_cmd," | tr --delete '\n' ");
		cmd_out = popen(convert_string_cmd, "r");
		fgets(out_s, 4194304, cmd_out);

		if(Create_File(basename(remove_ext(image_filename))) == -1)
		{
			printf("%s already exists in filesystem, skipped this current file.\n", image_filename);
			continue;
		}

		printf("Added %s to mini file system with base64 encoded\n", image_filename);

		inode_num = Open_File(basename(remove_ext(image_filename)));
		if (Write_File(inode_num, 0, out_s) == -1)
		{
			printf("File system full!\n");
			exit (1);
		}
		make_filesystem_summary(basename(image_filename));

		Close_File(inode_num);
	}

}

void make_filesystem_summary(char* filename){
  // Filename​ ,extension​ , ​ Inode No​ , and Size​ .
	FILE* f = fopen("filename", "a");
  int Inode_NO = Search_Directory(remove_ext(filename));
  char* ext = get_extension(filename);
  int filesize = Inode_List[Inode_NO].File_Size;
  fprintf(f, "filename: %s extension: %s Inode_NO: %d File_size: %d\n", filename, ext, Inode_NO, filesize);
	fclose(f);
}

void read_images_from_filesystem_and_write_to_output_directory(char* output_directory){
	struct stat s;
	DIR *oDir;
	char *workingDir = getcwd(NULL, 0);

	strcpy(fulloDir, workingDir);
	strcat(fulloDir, "/");
	strcat(fulloDir, output_directory);
	strcat(fulloDir, "/");

	int err = stat(fulloDir, &s);
	if (-1 == err){
		if (ENOENT == errno){
			mkdir(fulloDir, 0700);
			oDir = opendir(output_directory);
		} else {
			perror("stat");
			exit(1);
		}
	} else {
		if (S_ISDIR(s.st_mode)){
			oDir = opendir(output_directory);
		} else {
			printf("Error: There exists a file with name of the output directory\n");
			exit(1);
		}
	}
	closedir (oDir);

	int i;
	FILE *out_temp;
	char out_name[164];

	char *output_s= (char*)malloc(sizeof(char)*512*8192);
	FILE *html;
	html = fopen("filesystem_content.html", "w");
	fprintf(html, "<html><head><title>Converted Image Thumbnail</title></head><body>\n");
	fclose(html);

	for (i = 0; i < Superblock.next_free_inode; i++)
	{
		strcpy(out_name, fulloDir);
		strcat(out_name, Directory_Structure[i].Filename);
		strcat(out_name, ".temp");

		printf("Created a temperary out file for base64: %s\n", out_name);

		out_temp = fopen(out_name, "w");
		Open_File(Directory_Structure[i].Filename);
		Read_File(i, 0, Inode_List[i].File_Size, output_s);
		fprintf(out_temp, "%s", output_s);
		fclose(out_temp);

		generate_html_file(&Directory_Structure[i].Filename);

	}
	html = fopen("filesystem_content.html", "a");
	fprintf(html, "</body></html>");
  fclose(html);

}

void generate_html_file(char* filename){
	char out_temp_name[164];
	char out_thumb_name[170];

	char decode_cmd[212];
	char convert_cmd[424];

	char out_Jpg_name[164];

	FILE *html;
	html = fopen("filesystem_content.html", "a");

	strcpy(out_temp_name, fulloDir);
	strcpy(out_thumb_name, out_temp_name);
	strcat(out_thumb_name, "/");
	strcat(out_temp_name, filename);
	strcpy(out_Jpg_name, out_temp_name);
	strcat(out_thumb_name, filename);
	strcat(out_temp_name, ".temp");
	strcat(out_Jpg_name, ".jpg");
	strcat(out_thumb_name, "_thumb.jpg");

	printf("Decoded with base64 and removed temp out file: %s\n", out_Jpg_name);

	strcpy(decode_cmd, "/usr/bin/base64 -d ");
	strcat(decode_cmd, out_temp_name);
	strcat(decode_cmd, " >> ");
	strcat(decode_cmd, out_Jpg_name);
	strcat(decode_cmd, " && rm ");
	strcat(decode_cmd, out_temp_name);
	system(decode_cmd);

	printf("Converted image to thumb: %s\n", out_Jpg_name);

	strcpy(convert_cmd, "convert -geometry 200x200 ");
	strcat(convert_cmd, out_Jpg_name);
	strcat(convert_cmd, " ");
	strcat(convert_cmd, out_thumb_name);

	printf("Added image and thumb to html: %s\n", out_Jpg_name);

	system(convert_cmd);
	fprintf(html, "<a href=\"");
	fprintf(html, "%s", out_Jpg_name);
	fprintf(html, "\">\n");

	fprintf(html, "<img src=\"");
	fprintf(html, "%s", out_thumb_name);
	fprintf(html, "\"/></a>\n");

	fclose(html);

}
