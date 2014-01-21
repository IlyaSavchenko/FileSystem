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
		if (nd->inode->type == 2) {
			log_msg("FILE");
			stbuf->st_mode = S_IFREG | 0666;
			stbuf->st_nlink = 1;
			stbuf->st_size = nd->inode->is_file.total_size;
			return 0;
		} else
			printf("DIR\n");
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 3;
			return 0;
		}
	}


static int my_open(const char *path, struct fuse_file_info *fi) {  //?????????????????????????1
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

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
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

static int my_opendir(const char *path, struct fuse_file_info *fi) {
	node nd = searchByName(path);
	if (nd == NULL) 
		return -ENOENT;
	if (nd->inode->type != 1)
		return -ENOENT;
	return 0;
}

static int my_rmdir(const char *path) {
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

static int my_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	int pos = searchFreeInode();
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
	inode in = emptyInode(2);
	log_msg("");
	node n = createNodeEmptyInode(in, pos);
	log_msg("");
	copyName(n->name, name);
	log_msg("");
	add(parent, n);
	log_msg("");
	return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	log_msg(">>Start of reading");
	printf(">>Size %d, offset %d \n", size, offset);
	node file = searchByName(path);
	if (file->inode->type != 2) {
		log_msg(">>Is not file!");
		return -ENOENT;
	}

	int block_num, node_num;
	int node_capacity = SIZE * 49;
	node_num = offset / node_capacity;
	offset -= node_num * node_capacity;

	block_num = offset / SIZE;
	offset -= block_num * SIZE;

	int nind = node_num;

	while (nind > 0) 
		if (file->next == NULL) return 0;
		file = file->next;
		nind--;
	}
	if (file->inode->is_file.data[block_num] == NULL) {
		log_msg("Main is not exsist!");
		return 0;
	}
	file_node nd = dataOfNode(file->inode->is_file.data[block_num]);
	if (nd == NULL) {
		log_msg(">>ERROR in reading!");
		return 0;
	}
	if (nd->size == 0) {
		log_msg(">>Data is empty!");
		return 0;
	}

	log_msg(">>Success!");
	size_t read_size = size;
	size_t readed_size = 0;
	size_t len = nd->size;
	if (offset < len) {
		do {
			if (offset + size > len)
				read_size = len - offset;
			memcpy(buf + readed_size, nd->data + offset, read_size);
			size -= read_size;
			readed_size += read_size;
			if (len < size - 1) return readed_size;
			printf(">>Size need to read! %d\n", size);
			if ((int)size > 0) {
				if (block_num < 48) {
					log_msg(">>Next Data Block");
					block_num++;
				} else {
					log_msg(">>Next node");
					file = file->next;
					block_num = 0;
					if (file == NULL)
						return 0;
				}
				offset = 0;
				if (file->inode->is_file.data[block_num] == NULL) {
					log_msg(">>Next data block is not exist!");
					return readed_size;
				}
				nd = dataOfNode(file->inode->is_file.data[block_num]);
				if (nd == NULL) {
					log_msg(">>ERROR in reading!");
					return readed_size;
				}
				if (nd->size == 0) {
					log_msg(">>Data is empty");
					return readed_size;
				}
			}
		} while ((int)size > 0);
	} else
		readed_size = 0;
	log_msg(">>End of reading");
	return readed_size;
}


static int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	log_msg(">>Start of writing");
	printf(">>Size %d, offset %d \n", size, offset);
	node file = searchByName(path);
	node root = file;

	printf(">>     Base offset: %d\n", offset);

	int block_num, node_num;
	int node_capacity = SIZE * 49;

	printf(">>     Node length %d\n", node_capacity);

	node_num = offset / node_capacity;
	offset -= node_num * node_capacity;

	printf(">>     Node_num %d\n", node_num);
	printf(">>     New offset %d\n", offset);

	block_num = offset / SIZE;
	offset -= block_num * SIZE;

	printf(">>     Block_num %d\n", block_num);
	printf(">>     New offset %d\n", offset);

	int nind = node_num;
	while (nind > 0) {
		log_msg(">>Next node:")
		if (file->next == NULL) {
			log_msg(">> Next inode!");
                        inode in = emptyInode(2);
                        log_msg("");
                        unsigned long pos = searchFreeInode();
                        if (pos < 0) return 0;
                        log_msg("");
                        node new_node = createNodeEmptyInode(in, pos);
                        printf("type %d\n", new_node->inode->type);
                        log_msg("");
                        file->next = new_node;
                        log_msg("");
                        printf("type %d\n", file->next->inode->type);
                        file->inode->next = pos;
                        log_msg("");
                        save_node(new_node);
                        log_msg("");
                        save_node(file);
                        log_msg("");
                        block_num = 0;
                        log_msg("");
                } 	
		} 
		file = file->next;
		nind--;
	}
	if (file->inode->type != 2) {
		log_msg(">>Is not file!");
		return -ENOENT;
	}
	file_node n;
	if (file->inode->is_file.data[block_num] == NULL) {
		log_msg(">>New file:");
		n = dataOfEmptyNode();
		log_msg("");
		file->inode->is_file.data[block_num] = searchFreeDataNode();
		log_msg("");
		file->inode->is_file.used_count++;
		log_msg("");
		printf(">>Index %d \n", file->inode->is_file.data[block_num]);
		log_msg(">>Free space finded!");
	}
	else {
		log_msg("LOAD NODE");
		n = dataOfNode(file->inode->is_file.data[block_num]);
	}
	if (n == NULL) {
		log_msg(">>ERROR in loading!");
		return 0;
	}
	size_t write_size = size;
	size_t writted_size = 0;
	size_t len = n->size;
	if (offset <= len) {
		do {
			//log_msg("+++++++++++++++++++++++++++++++++++++WRITE ITERATION++++++++++++++++++")
			log_msg(">>Write:")
			if (size > SIZE)
				write_size = SIZE - offset;
			printf(">>Offcet %d, size %d, size %d, writte size %d, writted size %d\n", offset, size, size, write_size, writted_size);
			memcpy(n->data + offset, buf + writted_size, write_size);
			size -= write_size;
			writted_size += write_size;
			printf("Offcet %d, size %d, dsize %d, writte size %d, writted size %d\n", offset, size, size, write_size, writted_size);
			n->size += write_size;
			len = n->size;
			printf("len %d", len);
			saveNode(file);
			saveData(n, file->inode->is_file.data[block_num]);
			if (len < SIZE - 1) {
				root->inode->is_file.total_size += writted_size;
				saveNode(root);
				return writted_size;
			}
			printf(">>Size need to write! %d\n", size);
			if ((int)size > 0) {
				if (block_num < 48) {
					log_msg(">>Next Data Block");
					block_num++;
				} else {
					log_msg(">>Next inode");
					inode in = emptyInode(2);
					log_msg("");
					unsigned long pos = searchFreeInode();
					node new_node = createNodeEmptyInode(in, pos);
					file->next = new_node;
					file->inode->next = pos;
					saveNode(new_node);
					saveNode(file);
					file = new_node;
					block_num = 0;
				}
				offset = 0;
				n = dataOfEmptyNode();
				file->inode->is_file.data[block_num] = searchFreeDataNode();
				if (file->inode->is_file.data[block_num] < 0) return writted_size;
				file->inode->is_file.used_count++;
				
			}
		} while ((int)size > 0);
	} else
		writted_size = 0;
	log_msg(">>Parent saved");
	log_msg(">End of writing");
	root->inode->is_file.total_size += writted_size;
	saveNode(root);
	return writted_size;
}

static struct fuse_operations operations = {
	.getattr	= my_getattr,
	.readdir 	= my_readdir,
	.opendir 	= my_opendir,
	.mkdir 		= my_mkdir,
	.rmdir      = my_rmdir,
	.open       = my_open,
	.create     = my_create,
	.read 		= my_read,
	.write 		= my_write,
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
		}
	}
	loadFileSystem();
	searchFreeInode();
	return fuse_main(my_argc, my_argv, &operations, NULL);
}