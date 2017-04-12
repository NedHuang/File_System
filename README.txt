CSCI-4061 Spring 2017 Project 3

Jiatong Ruan 5052178
Mingzhe Huang 4944090

CSELab machine: LIND40-03

We set up the file system mainly based off homework description. Extra functions are
added for get/remove extension of filenames and get current time.

To search through the directory, we utilized system call. To be specific, in order to
deal with '\0' problem in c, we use base64 to get around it.

Log file, summary file and html file are all written in working directory.
