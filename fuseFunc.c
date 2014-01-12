#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "fileSystem.h"

void open_root() {
	
}

static int my_getattr(const char *path, struct stat *stbuf) {
	node nd = searchByName(path);
	if (nd == NULL) {
		log_msg("File not found");
		return -ENOENT;
	} else {
		log_msg(path);
		printf("This is NULL inode %d\n", nd->inode == NULL);
		if (nd->inode->type == 1) {
			log_msg("DIR");
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 3;
			return 0;
		} else {
			printf("FILE\n");
			stbuf->st_mode = S_IFREG | 0666;
			stbuf->st_nlink = 1;
			stbuf->st_size = nd->inode->is_ifle.total_size;
			return 0;
		}
	}
}

static int my_open(const char *path, struct fuse_file_info *finfo) {  //?????????????????????????1
	node nd = searchByName(path);
	if (nd == NULL) 
		return -ENOENT;
	if (nd->inode->type != 2)
		return -ENOENT;
	return 0;
}

static int my_unlink(const char *path) {
	log_msg("Unlink");
	node nd = searchByName(path);
	log_msg("SUCCESS! Node is finded!");
	showNode(nd);
	delete(nd);
	log_msg("Unlink complete");
	return 0;
}

static int my_mkdir(const char* path, mode_t mode) {
	int position = searchFreeInode();
	log_msg(path);
	char** names = split(path);
	log_msg(path);
	int i = 0, count = 0;
	while(names[i++] != NULL) {
		log_msg(names[count]);
		count++;
	}
	char* name = names[count - 1];
	log_msg("");
	log_msg(path);
	node parent = searchParent(path);
	inode ind = emptyInode(1);
	log_msg("");
	node nd = createNodeEmptyInode(ind, position);
	log_msg("");
	copyName(nd->name, name);
	log_msg("");
	add(parent, nd);
	log_msg("");
	return 0;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *finfo) {
	node nd = searchByName(path);
	printf("Read DIR %s\n", path);
	if (nd == NULL) {
		printf("File is can't open %s\n", path);
		return -ENOENT;
	} else { 
		if (nd->inode->type == 1) {
			filler(buf, ".", NULL, 0);	
			filler(buf, "..", NULL, 0);
			node n1 = nd;
			do {
				for (int i = 0; i < 10; i++) {
					if (nd->childs[i] != NULL)
						filler(buf, nd->childs[i]->name, NULL, 0);
				}
				nd = nd->next;
			} while (nd != NULL);
			return 0;
		}
		return -ENOENT;
	}
}

static int my_opendir(const char *path, struct fuse_file_info *finfo) {
	node nd = searchByName(path);
	if (nd == NULL) 
		return -ENOENT;
	if (nd->inode->type != 1)
		return -ENOENT;
	return 0;
}

static int my_rmdir(const char *path) {
	log_msg("rmdir");
	node nd = searchByName(path);
	log_msg("Node is finded");
	showNode(nd);
	delete(nd);
	log_msg("rmdir SUCCESS");
	return 0;
}

int my_truncate(const char * path, off_t offset) {
	return 0;
}


static struct fuse_operations operations = {
	.getattr	= my_getattr,
	.readdir 	= my_readdir,
	.opendir 	= my_opendir,
	.mkdir 		= my_mkdir,
	.rmdir      = my_rmdir,
	.open       = my_open,
	.unlink 	= my_unlink, 
	.truncate 	= my_truncate
};

int main(int argc, char *argv[]) {

	printf("Sizes:\n");
	printf("ifolder %d\n", sizeof(struct ifolder_s));
	printf("ifile %d\n", sizeof(struct ifile_s));
	printf("inode %d\n", sizeof(struct inode_s));


	char result = 0;
	char** my_argv = malloc(sizeof(char*));
	int my_argc = 1;
	my_argv[0] = argv[0];
	while ((result = getopt(argc,argv,"fn:du:")) != -1) {
		switch (result) {
			case 'f':
				formatForFS();
				break;
			case 'd':
				log_msg("");
				my_argc++;
				log_msg("");
				my_argv = realloc(my_argv, sizeof(char*) * my_argc);
				log_msg("");
				my_argv[my_argc - 1] = "-d";
				log_msg("");
				break;
			case 'n':
			log_msg("");
				my_argc++;
				log_msg("");
				my_argv = realloc(my_argv, sizeof(char*) * my_argc);
				log_msg("");
				my_argv[my_argc - 1] = optarg;
				log_msg("");
				break;
			case 'u':
				fileSystem = optarg;
		}
	}
	loadFileSystem();
	searchFreeInode();
	return fuse_main(new_argc, new_argv, &operations, NULL);
}