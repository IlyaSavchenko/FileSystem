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

char** split(char* path)
{
    char** array; // возвращаемое значение. 
 
    if (strlen(path) > 1) // возм. - "если это не корневая папка" - если длинна пути больше единицы
    {   
        int count = 0;  
        int i = 0;  
        while(path[i] != 0)
        {
            if (path[i] == '/') count++; // определяем "глубину" переданного  пути
            i++;
        }
 /* в результате мы посчитали "глубину залегания" последнего элемента - узла, указанного в пути */
 
        array = (char**)malloc(sizeof(char*)*(count + 2)); //  выделяем память для  (count + 2) указателей на массивы символов
        array[count + 1] = 0; // в конец строки - отделяющию null символ
 
        array[0] = "/"; // в начало добавлям разделяющий слэш
        int n = 1; // индекс элемента массива указателей на массивы символов - сейчас поставлен на указатель для первого массива символов.
        i = 1; // текущий индекс символа (парвый=0) в строке, содежащей переданный в функцию путь
 
 
        while(path[i] != 0)// перебираем все символы строки , содержащей путь со второго до предпоследнего включительно
        {
            int c = 1;  // число символом в имени одного из узлов пути (можно сказать - имени папки) - пока только один 
            int j = i;  // вспомогательный  индекс - далее мы считываем имя узла  - пока не встретим слыш или строка не закончится - приравниваем его к i
         
            while((path[j] != '/') && (path[j] != 0)) // определяем длинну имени узла -перемещаемся по фрагмента строки пути до следующего слэша
            {
                c++; // фиксируем то , что нами обнаружен очередной символ имени узла
        j++; // накручиваем счётчик 
        i++; // фиксируем тот факт, что мы считали очередной символ переданной в функцию строки, содержащей путь.
            }
            if (path[i] != 0) i++; /* если мы вышли из предыдущего цикла только по причине того, что встретили слэш - то пропускаем его - 
                              чтобы при следующей итерации цикла while(path[i] != 0) снова начать читать символы */
 
            array[n] = (char*)malloc(sizeof(char)*c);// выделяем память по числу букв "c" в имени текущего узла 
            array[n][c - 1] = 0;  // в конец фрагмента - имени узла - отделяющию null символ
            n++;    
        }
 
 
        n = 1; // номер указателя в массие фрагментов пути -  имён директорий
        i = 1;
        while(path[i] != 0)
        {           
            int j = i;
            int tmp = i;            
            while((path[j] != '/') && (path[j] != 0)) // заполняем массив именами фрагментов пути - именами узлов (директорий)
            {
                array[n][j - tmp] = path[j];
                j++;i++;
            }
            if (path[i] != 0) i++;
            n++;    
        }
    }
 else // если всё-таки длина пути равна единице - то есть это путь к корню относительно корня.
    {
        array = (char**)malloc(sizeof(char*)*2); // выделяем память для массива без значащего содержимого
        array[1] = 0;
        array[0] = "/";
    }
    return array;
}
 
/* ищет в иерархихи узел , путь к которому задан как (* path) -
возвращает ПОЛСЕДНИЙ ОБНАРУЖЕННЫЙ узел пути - самую "младший" во ввложенности обнаруженный файл*/

inode readInode(unsigned long index) {
    if (index == NULL) {
        log_msg("WARNING! index = NULL");
        return NULL;
    }
    FILE * sys = fileSystem();
    fseek(sys, index, SEEK_SET);
    inode in = malloc(sizeof(struct inode_s));
    fread(in, sizeof(struct inode_s), 1, sys);
    if (in->type == 0) {
        log_msg("WARNING! type of inode -> NULL");
        return NULL;
    }
    fclose(sys);
    return in;
}

void copyName(char* dest, char* source) {
    printf("copy name\n");
    int len = strlen(source);
    printf("source %s\n", source);
    len = len > 31 ? 31 : len;
    int i = 0;
    for (; i < len; i++)
        dest[i] = source[i];
    dest[i] = NULL;
    printf("dest %s\n", dest);
}

node readTree(unsigned long index, node parent, char* name) {
    if (index == NULL) return NULL;
    inode in = readInode(index);
    if (in == NULL) {
        log_msg("!!readed node is NULL");
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
    printf("size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
    printf("data_start %lu, dev_size %lu\n", fs_info.data_start, fs_info.dev_size);
    log_msg("");
    fs_cash = readTree(fs_info.inode_start, NULL, "/");
    fclose(sys);
}

node readNode(unsigned long index) {
    
}

void showNode(node nd) {
    printf("Node\n");
    printf("~index: %lu\n", nd->index);
    printf("~name: %s\n", nd->name);
    if (nd->inode->type == 1) {
        printf("~This is directory\n");
        for (int i = 0; i < 10; i++) {
            printf("~child [%d] is NULL %d\n", i, nd->childs[i] == NULL);
            printf("~child of inode [%d] is NULL %d\n", i, nd->inode->is_folder.nodes[i] == NULL);
            if (nd->inode->is_folder.nodes[i] != NULL) 
                printf("~child's name [%d] %s\n", i, nd->inode->is_folder.names[i]);
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

node searchChild(char* name, node parent) {
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
        printf("Path: %d : %s\n", i-1, pathDecomp[i-1]);
        count++;
    }
    printf("Count %d\n", count);
    if (count == 1) {
        if (strcmp(path, "/") == 0) {
            return fs_cash;
            printf("This is root, inode_start %d\n", fs_info.inode_start);
        }
        else {
            printf("ERROR! Not root\n");
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
        printf("Path: %d : %s\n", i-1, pathDecomp[i-1]);
        count++;
    }
    if (count == 2) {
        if (strcmp(pathDecomp[0], "/") == 0) {
            printf("This is root, inode_start %d\n", fs_info.inode_start);
            return fs_cash;
        }
        else {
            printf("ERROR! Not root\n");
            return NULL;
        }
    } else {
        printf("Search \n");
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
        printf("try %d \n", pos);
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
    for (int i = 0; i < size; i++) 
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
        printf("try %d \n", pos);
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
    log_msg("Formated");
    fseek(sys, 0, SEEK_SET);
    fs_info.inode_size = sizeof(struct inode_s);
    fs_info.inode_start = sizeof(struct fs_info_s);
    fs_info.dev_size = size;
    fs_info.data_node_size = sizeof(struct file_node_s);
    fs_info.data_start = (unsigned long)(size * 0.05);
    printf("Size (written) %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
    fwrite(&fs_info, sizeof(struct fs_info_s), 1, sys);
    log_msg("Written info about file system");

    float last = 0, old = 0;
    for (unsigned long i = fs_info.inode_start; i < fs_info.data_start; i+=fs_info.inode_size) {
        fseek(sys,i,SEEK_SET);  
        fwrite(&zero, 4, 1, sys);
        float cur = ((float)i / fs_info.data_start) * 100;
        printf("Formating inode: %f%%\n", cur);
    }
    last = 0;
    for (unsigned long i = fs_info.data_start; i < fs_info.dev_size; i+=fs_info.data_node_size) {
        fseek(sys,i,SEEK_SET);
        fwrite(&zero, 4, 1, sys);
        float cur = ((float)i / fs_info.dev_size) * 100;
        printf("Formating data: %f%%\n", cur);
    }

    inode ind = emptyInode(1);
    log_msg("");

    node n = createNodeEmptyInode(ind, fs_info.inode_start);
    log_msg("");
    saveNode(n);
    log_msg("");

    fclose(sys);
    log_msg("Formating successful");
}



