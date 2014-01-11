#ifndef FS
#define FS
#include <stdio.h>	
#define DEBUG
#ifdef DEBUG
	#define log_msg(a) printf(">> line: %d, mess: %s\n", line, a);
#else
	#define log_msg(a)
#endif

#define size 4096// блок данных

typedef struct ifolder_s {
	char names[10][32];
	unsigned long nodes[10];
}ifolder;

typedef struct ifile_s {
	int used_count;
	int total_size;
	unsigned long data[49];	
}ifile;

typedef struct inode_s * inode;
struct inode_s {  // inodes
	int type;
	unsigned long next;
	union {
		ifolder is_folder;
		ifile is_file;
	};
};

typedef struct file_node_s * file_node;
struct file_node_s
{
	char data[size];
	int size;
};

typedef struct node_s * node;
struct node_s {
	unsigned long index;
	inode inode;
	node parent;
	node next;
	node childs[10];
	char name[32];
};  // in op.memry

struct fs_info_s {
	int inode_start;
	int inode_size;
	int data_node_size;
	unsigned long dev_size;
	unsigned long data_start;// format 
}fs_info;

extern char* filesys;
node fs_cash;


#endif
