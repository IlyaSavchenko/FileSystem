#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fileSystem.h"

char* fs = "./device";

FILE* fileSystem() {
    FILE* sys = fopen(fs, "rb+");
    fseek(sys, 0, SEEK_SET);
    return sys;
}

char** split(char* path) {
    char* buf = malloc((strlen(path) + 1) * sizeof(char));
    forSplit(buf, path);
    char** res;
    if (strlen(buf) > 1) {
        int count = 0;
        int i = 0;
        while(buf[i] != 0)
            if (buf[i++] == '/') 
                count++;
        res = malloc(sizeof(char*)*(count + 2));
        res[count + 1] = 0;
        res[0] = "/";
        char* pointer = strtok(buf, "/");
        i = 1;
        while(pointer) {
            res[i++] = pointer;
            pointer = strtok(NULL, "/");
        }
    } else {
        res = (char**)malloc(sizeof(char*)*2);
        res[1] = 0;
        res[0] = "/";
    }
    return res;
}
void forSplit(char* dest, char* source) {
    printf("copy name\n");
    int length = strlen(source);
    printf("source %s\n", source);
    int i = 0;
    for (; i < length; i++)
        dest[i] = source[i];
     dest[i] = NULL;
    //printf("dest %s\n", dest);
}

void copyName(char* dest, char* source) {
    int length = strlen(source);
    printf(">>Source %s\n", source);
    length = length > 31 ? 31 : length;
    int i = 0;
    for (; i < length; i++)
        dest[i] = source[i];
    dest[i] = NULL;
    //printf(">>dest %s\n", dest);
}

inode readInode(unsigned long index) {
    if (index == NULL) {
        log_msg(">>WARNING! index = NULL");
        return NULL;
    }
    FILE * sys = fileSystem();
    fseek(sys, index, SEEK_SET);
    inode in = malloc(sizeof(struct inode_s));
    fread(in, sizeof(struct inode_s), 1, sys);
    if (in->type == 0) {
        log_msg(">>WARNING! type of inode -> NULL");
        return NULL;
    }
    fclose(sys);
    return in;
}

node readTree(unsigned long index, node parent, char* name) {
    if (index == NULL) return NULL;
    inode in = readInode(index);
    if (in == NULL) {
        log_msg(">>Readed node is NULL");
        return NULL;    
    } 
    inode p = in;
    node head = malloc(sizeof(struct node_s));
    node h = head;
    h->parent = parent;
    h->index = index;
    h->inode = p;
    copyName(h->name, name);
    h->next = readTree(p->next, parent, name);
    if (p->type == 1) {
        for (int i = 0; i < 10; i++) {
            if (p->is_folder.nodes[i] != NULL) {
                h->childs[i] = readTree(p->is_folder.nodes[i], head, p->is_folder.names[i]);
            } else {
                h->childs[i] = NULL;
            }
        }
    }
    return h;
}

void loadFileSystem() {
    FILE * sys = fileSystem();
    fread(&fs_info, sizeof(struct fs_info_s), 1, sys);
    printf(">>size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
    printf(">>start %lu, size %lu\n", fs_info.data_start, fs_info.dev_size);
    log_msg("");
    fs_cash = readTree(fs_info.inode_start, NULL, "/");
    fclose(sys);
}

node readNode(unsigned long index) {
    
}

void showNode(node nd) {
    printf(">>Node\n");
    printf(">>~Index: %lu\n", nd->index);
    if (nd->inode->type == 1) {
        printf(">>~This is directory\n");
        for (int i = 0; i < 10; i++) {
            printf(">>~Child [%d] is NULL %d\n", i, nd->childs[i] == NULL);
            printf(">>~Child of inode [%d] is NULL %d\n", i, nd->inode->is_folder.nodes[i] == NULL);
            if (nd->inode->is_folder.nodes[i] != NULL) 
                printf(">>~Child's name [%d] %s\n", i, nd->inode->is_folder.names[i]);
        }
    }
}

void saveNode(node nd) {
    FILE * sys = fileSystem();
    fseek(sys, nd->index, SEEK_SET);
    if (nd->inode->type != 0)
        fwrite(nd->inode, sizeof(struct inode_s), 1, sys);
    else {
        int stat = NULL;
        fwrite(&stat, 4, 1, sys);
    }
    fclose(sys);
}

void add(node parent, node child) {
    int i = 0;
    while (i < 10 && parent->childs[i] != NULL) i++;
    if (i < 10) {
        parent->childs[i] = child;
        child->parent = parent;
        parent->inode->is_folder.nodes[i] = child->index;
        copyName(parent->inode->is_folder.names[i], child->name);
        saveNode(child);
        saveNode(parent);
    }
}

void clearChild(node parent, node child) {
    if (parent == NULL) return;
    node n = parent;
    if (parent->inode->type != 1) return;
    do {
        for (int i = 0; i < 10; i++) {
            if (n->inode->is_folder.nodes[i] == child->index) {
                n->inode->is_folder.nodes[i] = NULL;
                n->childs[i] = NULL;
                saveNode(n);
                return;
            }
        }
        n = n->next;
    } while(n != NULL);
}

void delete(node nd) {
    clearChild(nd->parent, nd);
    freeNode(nd);
}

node searchChildByName(char* name, node parent) {
    node n = parent;
    log_msg("");
    log_msg(parent->name);
    log_msg(name);
    do {
        for (int i = 0; i < 10; i++) {
            if (n->childs[i] != NULL) {
                if (strcmp(n->childs[i]->name, name) == 0)
                    return n->childs[i];
            }
        }
        n = n->next;
    } while (n != NULL);
    return NULL;
}

node searchByName(char* path) {
    char** pathDecomp = split(path);
    int i = 0, count = 0;
    while(pathDecomp[i++] != NULL) {
        printf(">>Path: %d : %s\n", i-1, pathDecomp[i-1]);
        count++;
    }
    printf(">>Count %d\n", count);
    if (count == 1) {
        if (strcmp(path, "/") == 0) {
            return fs_cash;
            printf(">>This is root, inode_start %d\n", fs_info.inode_start);
        }
        else {
            printf(">>ERROR! Not root\n");
            return NULL;
        }
    } else {
        node folder = searchParent(path);
        if (folder == NULL) return NULL;
        return searchChildByName(pathDecomp[count - 1], folder);
    }
}

node searchParent(char* path) {
    char** pathDecomp = split(path);
    int i = 0, count = 0;
    while(pathDecomp[i++] != NULL) {
        printf(">>Path: %d : %s\n", i-1, pathDecomp[i-1]);
        count++;
    }
    if (count == 2) {
        if (strcmp(pathDecomp[0], "/") == 0) {
            printf(">>This is root, inode_start %d\n", fs_info.inode_start);
            return fs_cash;
        }
        else {
            printf(">>ERROR! Not root\n");
            return NULL;
        }
    } else {
        printf(">>Search \n");
        node nd = fs_cash;
        i = 1;
        while (pathDecomp[i+1] != NULL) {
            nd = searchChildByName(pathDecomp[i], nd);
            if (nd == NULL) return NULL;
            i++;
        }
        return nd;
    }
}

file_node dataOfNode(unsigned long index) {
    FILE * sys = fileSystem();
    file_node n = malloc(sizeof(struct file_node_s));
    fseek(sys, index, SEEK_SET);
    int stat;
    fread(&stat, 4, 1, sys);
    if (stat == NULL) return NULL;
    fseek(sys, index, SEEK_SET);
    fread(n, sizeof(struct file_node_s), 1, sys);
    fclose(sys);
    return n;
}

unsigned long searchFreeDataNode() {
    FILE * sys = fileSystem();
    fseek(sys, fs_info.data_start, SEEK_SET);
    unsigned long pos = fs_info.data_start;
    int stat;
    do {
        printf(">>Try %d \n", pos);
        fread(&stat, 4, 1, sys);
        pos += fs_info.data_node_size;
        fseek(sys, pos, SEEK_SET);
    } while (stat != NULL);
    fclose(sys);
    return pos - fs_info.data_node_size;
}

void freeNode(node nd) {
    do {
        if (nd->inode->type == 1) {
            for (int i = 0; i < 0; i++) {
                if (nd->childs[i] != NULL) {
                    freeNode(nd->childs[i]);
                }
            }
        }
        nd->inode->type = 0;
        saveNode(nd);
        free(nd->inode);
        node next = nd->next;
        free(nd);
        nd = next;
    } while (nd != NULL);
}

file_node dataOfEmptyNode() {
    file_node fn = malloc(sizeof(struct file_node_s));
    for (int i = 0; i < SIZE; i++) 
        fn->data[i] = NULL;
    fn->size = 0;
}

void saveData(file_node nd, unsigned long index) {
    FILE * sys = fileSystem();
    fseek(sys, index, SEEK_SET);
    fwrite(nd, sizeof(struct file_node_s), 1, sys);
    fclose(sys);
}

node createNodeEmptyInode(inode ind, unsigned long index) {
    node head = malloc(sizeof(struct node_s));
    head->index = index;
    head->inode = ind;
    head->parent = NULL;
    head->next = NULL;
    for (int i = 0; i < 10; i++) {
        head->childs[i] = NULL;
    }
    return head;
}

inode emptyInode(int type) {
    inode in = malloc(sizeof(struct inode_s));
    in->type = type;
    in->next = NULL;
    if (type == 1) {
        for (int i = 0; i < 10; i++) {
            in->is_folder.nodes[i] = NULL;
        }
    } else if (type == 2) {
        for (int i = 0; i < 49; i++) {
            in->is_file.data[i] = NULL;
        }
        in->is_file.used_count = 0;
        in->is_file.total_size = 0;
    } else {
        return NULL;
    }
    return in;
}

unsigned long searchFreeInode() {
    FILE * sys = fileSystem();
    fseek(sys, fs_info.inode_start, SEEK_SET);
    unsigned long pos = fs_info.inode_start;
    int stat;
    do {
        printf(">>Try %d \n", pos);
        fread(&stat, 4, 1, sys);
        pos += fs_info.inode_size;
        fseek(sys, pos, SEEK_SET);
    } while (stat != NULL);
    fclose(sys);
    return pos - fs_info.inode_size;
}

void formatForFS() {
    FILE * sys = fileSystem();
    log_msg("");
    int zero = NULL;
    fpos_t pos;
    fgetpos(sys, &pos);

    fseek(sys,0,SEEK_END);   
    unsigned long size = ftell(sys); 
    log_msg(">>Formated");
    fseek(sys, 0, SEEK_SET);
    fs_info.inode_size = sizeof(struct inode_s);
    fs_info.inode_start = sizeof(struct fs_info_s);
    fs_info.dev_size = size;
    fs_info.data_node_size = sizeof(struct file_node_s);
    fs_info.data_start = (unsigned long)(size * 0.05);
    printf(">>Size (written) %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
    fwrite(&fs_info, sizeof(struct fs_info_s), 1, sys);
    log_msg(">>Written info about file system");

    float last = 0, old = 0;
    for (unsigned long i = fs_info.inode_start; i < fs_info.data_start; i+=fs_info.inode_size) {
        fseek(sys,i,SEEK_SET);  
        fwrite(&zero, 4, 1, sys);
        float cur = ((float)i / fs_info.data_start) * 100;
        printf(">>Formating inode: %f%%\n", cur);
    }
    last = 0;
    for (unsigned long i = fs_info.data_start; i < fs_info.dev_size; i+=fs_info.data_node_size) {
        fseek(sys,i,SEEK_SET);
        fwrite(&zero, 4, 1, sys);
        float cur = ((float)i / fs_info.dev_size) * 100;
        printf(">>Formating data: %f%%\n", cur);
    }

    inode ind = emptyInode(1);
    log_msg("");

    node n = createNodeEmptyInode(ind, fs_info.inode_start);
    log_msg("");
    saveNode(n);
    log_msg("");

    fclose(sys);
    log_msg(">>Formating successful");
}



